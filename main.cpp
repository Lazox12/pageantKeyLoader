#include <filesystem>
#include <iostream>
#include <ranges>
#include <cstring>
#include <fstream>
#include <SQLiteCpp/SQLiteCpp.h>
#include <sha256.h>
#include <vector>

#include <string>

#include "sshKey.cpp"

#define pageant_process "pageant.exe"

namespace fs = std::filesystem;


SQLite::Database* db;

std::string hash(const std::string &path) {
    std::ifstream fs(path);
    if (!fs.is_open()) {
        throw std::runtime_error("Could not open file");
    }
    std::stringstream buffer;
    buffer << fs.rdbuf();
    fs.close();
    return sha256(buffer.str());
}

void killPageant() {
    std::string command = "taskkill /F /IM ",
    system(command+pageant_process);
}


int addKey(const std::string &path,const std::string& name, const bool update = true,bool load = true,const std::string &comment = "") {
    try {
        std::cout<<"Adding key "<<path<<" to "<<name<<std::endl;
        std::string item;
        std::stringstream ss(path);
        while (getline (ss, item, '/')) {
        }
        SQLite::Statement query(*db,"SELECT origin FROM keys WHERE name == ?");
        query.bind(1,name);
        if (query.executeStep()) {
            std::cout<<"name "<<name<<" already in use"<<std::endl;
            return 1;
        }
        fs::copy(path, "keys/"+name+".ppk", fs::copy_options::overwrite_existing);
        const std::string h = hash("keys/"+name+".ppk");
        (*db).exec("INSERT INTO keys VALUES (NULL,'"+name+"','"+path+"','"+h+"',"+(update?"1":"0")+","+(load?"1":"0")+",'"+comment+"')");
        return 0;
    }catch (fs::filesystem_error &e) {
        std::cout <<"error adding keys"+static_cast<std::string>(e.what())<<std::endl;
        return -1;
    }catch (SQLite::Exception &e) {
        std::cout <<"error adding keys"+static_cast<std::string>(e.what())<<std::endl;
        std::cout << (*db).getErrorMsg();
        return -2;
    }
}
int addKeysRecursively(const std::string &path,const std::string& name, const bool update = true,bool load = true,const std::string &comment = "") {
    for (const auto& file:fs::directory_iterator(path)) {
        if (file.is_regular_file() and file.path().extension() ==".ppk") {
            std::string keyName = name;
            const size_t start_pos = keyName.find("%name%");
            if(start_pos != std::string::npos) keyName.replace(start_pos, 6, file.path().stem().string());

            std::string keyComment = comment;
            const size_t start1_pos = keyComment.find("%name%");
            if(start_pos != std::string::npos) keyComment.replace(start1_pos, 6, file.path().stem().string());
            std::cout<<"adding key "<<keyName<<" to "<<keyComment<<"from"<<file.path()<<std::endl;
            int ret = addKey(file.path().string(),keyName,update,load,comment);
            if (ret != 0) {
                return ret;
            }

        }
    }
    return 0;
}
int removeKey(const std::string &name) {
    try {
        SQLite::Statement query(*db,"SELECT origin FROM keys WHERE name == ?");
        query.bind(1,name);
        if (!query.executeStep()) {
            std::cout<<"key "<<name<<" does not exist"<<std::endl;
            return 1;
        }
        fs::remove("keys/"+name+".ppk");
        (*db).exec("DELETE FROM keys WHERE name == '"+name+"'");
        std::cout << name << " removed"<<std::endl;
        return 0;
    }catch (fs::filesystem_error &e) {
        std::cout <<"error removing keys"+static_cast<std::string>(e.what())<<std::endl;
        return -1;
    }
}
void removeAllKeys() {
    std::cout << "removing all keys"<<std::endl;
    SQLite::Statement query(*db,"SELECT name FROM keys");
    while (query.executeStep()) {
        removeKey(query.getColumn(0));
    }
}

void init() {
    if (!fs::exists("db.sqlite3")) {
        std::cout<<"db.sqlite3 missing"<<std::endl;
        std::ofstream ("db.sqlite3");
    }
    db = new SQLite::Database("db.sqlite3",SQLite::OPEN_READWRITE);
    if (!fs::exists("keys")) {
        fs::create_directory("keys");
    }if (!fs::exists("tmp")) {
        fs::create_directory("tmp");
    }
    if (!(*db).tableExists("keys")) {
        std::cout<<"keys not found"<<std::endl;
        (*db).exec(R"V0G0N(
create table keys
(
    id         integer
        constraint keys_pk
            primary key,
    name       TEXT,
    origin     TEXT,
    originHash TEXT,
    AutoUpdate integer,
    AutoLoad integer,
    comment    TEXT
);)V0G0N");
    }

}

void load_keys(std::vector<std::string>& files) {
    std::string args;
    for (auto &i : files) {
        args += " "+i;
    }
    std::cout<<pageant_process+args<<std::endl;
    system((pageant_process+args).c_str());
}
void load_all_keys() {
    std::vector<std::string> keys;
    std::string path = fs::current_path().string();
    for (const auto& file:fs::directory_iterator("keys\\")) {
        if (file.is_regular_file() and file.path().extension() ==".ppk") {
            keys.push_back(" "+path+file.path().string());
        }
    }
    load_keys(keys);
}

void editComment(const std::string& name,const std::string& comment) {
    SQLite::Statement query((*db),"UPDATE keys SET comment = ? WHERE name = ?");
    query.bind(2,name);
    query.bind(1,comment);
    query.exec();
}
void updateComment(const std::string &name) {
    SQLite::Statement query((*db),"SELECT comment FROM keys WHERE name = ?");
    query.bind(1,name);
    if (!query.executeStep()) {
        throw std::runtime_error("update comment failed");
    }
    std::string comment = query.getColumn(0).getString();
    if (comment.empty()) {
        return;
    }
    fs::remove("tmp/"+name+".ppk");
    std::ifstream ifs("keys/"+name+".ppk");
    std::ofstream ofs("tmp/"+name+".ppk");

    std::string line;
    while (getline(ifs,line)) {
        if (std::string::size_type substr = line.find("Comment"); substr != std::string::npos) {
            ofs<<"Comment: "<<comment<<std::endl;
        }else {
            ofs<<line<<std::endl;
        }
    }
    ifs.close();
    ofs.close();
    fs::remove("keys/"+name+".ppk");
    fs::copy("tmp/"+name+".ppk","keys/"+name+".ppk");
}
void updateKey(const std::string& name,const std::string& origin) {
    fs::remove("keys/"+name+".ppk");
    fs::copy(origin,"keys/"+name+".ppk");
    updateComment(name);
}
void updateAllComments() {
    SQLite::Statement query((*db),"SELECT name FROM keys");
    while (query.executeStep()) {
        updateComment(query.getColumn(0).getString());
    }
}
void normal() {
    SQLite::Statement query(*db,"SELECT name,origin,originHash,AutoUpdate FROM keys WHERE AutoLoad == 1");
    std::vector<std::string> keys;
    std::string path = fs::current_path().string()+"\\keys\\";
    while (query.executeStep()) {
        if (query.getColumn(3).getInt() == 1) {
            if (query.getColumn(2).getString() != hash(query.getColumn(1).getString())) {
                std::cout<<"updating key "<<query.getColumn(0).getString()<<std::endl;
                updateKey(query.getColumn(0).getString(),query.getColumn(1).getString());
            }
            keys.push_back(path+query.getColumn(0).getString()+".ppk");
        }
    }
    load_keys(keys);
}

bool getBoolFromUser(const std::string& name)
{
    std::cout << name<<" (Y=1/N=0) : " << std::endl;

    bool setValue;
    if (std::cin >> setValue) {
        std::cin.clear();
        std::cin.ignore(1000000, '\n');
        return setValue;  // Boolean read correctly
    }

    // Badly formed input: failed to read a bool
    std::cout << "Wrong value. Only 1 or 0 is accepted." << std::endl;
    std::cin.clear();                // Clear the failed state of the stream
    std::cin.ignore(1000000, '\n');  // Extract and discard the bad input

    return getBoolFromUser(name);  // Try again
}

void user() {
    std::cout<<"entering cli mode"<<std::endl;
    std::cout<<"to run automaticaly run with -a"<<std::endl;
    std::string command;
    while (true) {
        std::cout<<"enter command"<<std::endl<<"a : addKey"<<std::endl<<"f : recursively add all keys is a folder "<<std::endl<<"r : removeKey"<<std::endl <<"l : loadAllKeys"<<std::endl<<"e : editComment"<<std::endl<<"d : reload all keys and restart pageant"<<std::endl<<"c : remove all keys"<<std::endl<<"q : quit"<<std::endl;
        std::getline(std::cin,command);
        if (strcmp("a",command.c_str())==0) {
            std::string path,name;
            std::cout<<"enter path to the key"<<std::endl;
            std::getline(std::cin,path);
            std::cout<<"enter name to the key"<<std::endl;
            std::getline(std::cin,name);
            bool load = getBoolFromUser("do you whant to auto load this key on startup");
            bool update = getBoolFromUser("do you whant to automaticaly update this key");
            std::string comment;
            if (!getBoolFromUser("do you whant to preserve comment for this key?")) {
                comment = name+" imported from keyLoader";
            }
            if (addKey(path,name,update,load,comment)==0) {
                std::cout<<"key added successfully"<<std::endl;
                updateComment(name);
            }else {
                std::cout<<"adding key failed"<<std::endl;
            }
            continue;
        }
        if (strcmp("f",command.c_str())==0) {
            std::string path,name;
            std::cout<<"enter path to the folder"<<std::endl;
            std::getline(std::cin,path);
            std::cout<<"enter name convencion for the keys %name% is the current key name for example %name% in test.ppk would be test"<<std::endl;
            std::getline(std::cin,name);
            bool load = getBoolFromUser("do you whant to auto load these keys on startup");
            bool update = getBoolFromUser("do you whant to automaticaly update these keys");
            std::string comment;
            if (!getBoolFromUser("do you whant to preserve comments for these keys? the only visible property in pageant \n if not it will automaticaly set the comment to the key name")) {
                comment = "%name% imported from keyLoader";
            }
            if (addKeysRecursively(path,name,update,load,comment)==0) {
                std::cout<<"key added successfully"<<std::endl;
                updateAllComments();
            }else {
                std::cout<<"adding key failed"<<std::endl;
            }
            continue;
        }
        if (strcmp("r",command.c_str())==0) {
            std::string name;
            std::cout<<"enter name to the key"<<std::endl;
            std::getline(std::cin,name);
            std::cout<<"removing key"<<std::endl;
            removeKey(name);
            continue;
        }
        if (strcmp("l",command.c_str())==0) {
            std::cout<<"Loading all keys"<<std::endl;
            load_all_keys();
            continue;
        }
        if (strcmp("e",command.c_str())==0) {
            std::string name,newComment;
            std::cout<<"enter name of the key"<<std::endl;
            std::getline(std::cin,name);
            std::cout<<"enter new comment for the key"<<std::endl;
            std::getline(std::cin,newComment);
            editComment(name,newComment);
            updateComment(name);
            continue;
        }
        if (strcmp("d",command.c_str())==0) {
            killPageant();
            normal();
            continue;
        }
        if (strcmp("c",command.c_str())==0) {
            bool remove=getBoolFromUser("do you whant to remove all keys");
            if (remove) {
                removeAllKeys();
            }
            continue;

        }
        if (strcmp("q",command.c_str())==0) {
            std::cout<<"Quitting"<<std::endl;
            break;
        }
        std::cout<<"invalid option"<<std::endl;
    }
}

int main(int argc, char** argv) {
    std::ifstream ifs("../test.ppk");
    sshKey key;
    key.parseKey(ifs);
    key.decodeData();
    key.generateMac();


    return 0;
    init();
    if (argc>1 && strcmp(argv[1],"-a") == 0) {
        normal();
    }else {
        user();
    }

    //addKey("C:/users/tomik/.ssh/test.ppk","aaaa");
    //removeKey("aaaa");
    //editComment("aaaa.ppk","test.ppk");
    return 0;
}
