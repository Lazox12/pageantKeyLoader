#include <filesystem>
#include <iostream>
#include <ranges>
#include <cstring>
#include <fstream>
#include <SQLiteCpp/SQLiteCpp.h>
#include <sha256.h>

namespace fs = std::filesystem;

SQLite::Database db("../db.sqlite3",SQLite::OPEN_READWRITE);

std::string hash(const std::string &path) {
    std::ifstream fs("keys/"+path+".ppk");
    if (!fs.is_open()) {
        throw std::runtime_error("Could not open file");
    }
    std::stringstream buffer;
    buffer << fs.rdbuf();
    fs.close();
    return sha256(buffer.str());
}


int addKey(const std::string &path,const std::string& name, const bool update = true,bool load = true,const std::string &comment = "") {
    try {
        std::cout<<"Adding key "<<path<<" to "<<name<<std::endl;
        std::string item;
        std::stringstream ss(path);
        while (getline (ss, item, '/')) {
        }
        SQLite::Statement query(db,"SELECT origin FROM keys WHERE name == ?");
        query.bind(1,name);
        if (query.executeStep()) {
            std::cout<<"name "<<name<<" already in use"<<std::endl;
            return 1;
        }
        fs::copy(path, "keys/"+name+".ppk", fs::copy_options::overwrite_existing);
        const std::string h = hash(name);
        std::cout<<h<<std::endl;
        db.exec("INSERT INTO keys VALUES (NULL,'"+name+"','"+path+"','"+h+"',"+(update?"1":"0")+","+(load?"1":"0")+",'"+comment+"')");
        return 0;
    }catch (fs::filesystem_error &e) {
        std::cout <<"error adding keys"+static_cast<std::string>(e.what())<<std::endl;
        return -1;
    }catch (SQLite::Exception &e) {
        std::cout <<"error adding keys"+static_cast<std::string>(e.what())<<std::endl;
        std::cout << db.getErrorMsg();
        return -2;
    }
}
int removeKey(const std::string &name) {
    std::cout << name << " removed"<<std::endl;
    try {
        SQLite::Statement query(db,"SELECT origin FROM keys WHERE name == ?");
        query.bind(1,name);
        if (!query.executeStep()) {
            std::cout<<"name "<<name<<" does not exist"<<std::endl;
            return 1;
        }
        fs::remove("keys/"+name+".ppk");
        db.exec("DELETE FROM keys WHERE name == '"+name+"'");
        return 0;
    }catch (fs::filesystem_error &e) {
        std::cout <<"error removing keys"+static_cast<std::string>(e.what())<<std::endl;
        return -1;
    }
}

void init() {
    std::cout<<"init"<<std::endl;
    if (!fs::exists("keys")) {
        fs::create_directory("keys");
    }if (!fs::exists("tmp")) {
        fs::create_directory("tmp");
    }
    if (!db.tableExists("keys")) {
        std::cout<<"keys not found"<<std::endl;
        db.exec(R"V0G0N(
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

void load_keys(const std::string& process,const std::string& folder) {
    std::string args;

    for (const auto& file:fs::directory_iterator(folder)) {
        if (file.is_regular_file() and file.path().extension() ==".ppk") {
            args += " "+file.path().string();
        }
    }

    std::cout<<process+args<<std::endl;
    system((process+args).c_str());
}
void editComment(const std::string &path,const std::string &comment) {
    fs::remove("tmp/"+path);
    std::ifstream ifs("keys/"+path);
    std::ofstream ofs("tmp/"+path);

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
    fs::remove("keys/"+path);
    fs::copy("tmp/"+path,"keys/"+path);
}

void normal() {
    SQLite::Statement query(db,"SELECT * FROM keys");
}

bool getBoolFromUser(const std::string& name)
{
    std::cout << name<<" (Y=1/N=0) : " << std::endl;

    bool setValue;
    if (std::cin >> setValue) return setValue;  // Boolean read correctly

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
        std::cout<<"enter command"<<std::endl<<"a : addKey"<<std::endl<<"r : removeKey"<<std::endl <<"l : loadKeys"<<std::endl<<"e : editComment"<<std::endl<<"q : quit"<<std::endl;
        std::cin>>command;
        if (strcmp("a",command.c_str())==0) {
            std::string path,name;
            bool update,load;
            std::cout<<"enter path to the key"<<std::endl;
            std::cin>>path;
            std::cout<<"enter name to the key"<<std::endl;
            std::cin>>name;
            load = getBoolFromUser("do you whant to auto load this key on startup");
            update = getBoolFromUser("do you whant to automaticaly update this key");
            if (addKey(path,name,update,load,"")==0) {
                std::cout<<"key added successfully"<<std::endl;
            }
        }
        if (strcmp("r",command.c_str())==0) {
            std::string name;
            std::cout<<"enter name to the key"<<std::endl;
            std::cin>>name;
        }
        if (strcmp("l",command.c_str())==0) {
            std::cout<<"Loading all keys"<<std::endl;
            load_keys("","keys");
        }
        if (strcmp("e",command.c_str())==0) {
            std::string name,newComment;
            std::cout<<"enter name of the key"<<std::endl;
            std::cin>>name;
            std::cout<<"enter new comment for the key"<<std::endl;
            std::cin>>newComment;
            editComment(name,newComment);
        }
        if (strcmp("q",command.c_str())==0) {
            std::cout<<"Quitting"<<std::endl;
            break;
        }
    }
}

int main(int argc, char** argv) {
    init();

    if (argc>2 && strcmp(argv[1],"-a") == 0) {
        normal();
    }else {
        user();
    }
    //addKey("C:/users/tomik/.ssh/test.ppk","aaaa");
    //removeKey("aaaa");
    //editComment("aaaa.ppk","test.ppk");
    return 0;
}
