#include <filesystem>
#include <iostream>
#include <ranges>
#include <cstring>
#include <fstream>
#include <openssl/sha.h>
#include <SQLiteCpp/SQLiteCpp.h>

namespace fs = std::filesystem;

SQLite::Database db("../db.sqlite3",SQLite::OPEN_READWRITE);

std::string hash(const std::string inputStr) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    const unsigned char* data = (const unsigned char*)inputStr.c_str();
    SHA256(data, inputStr.size(), hash);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
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
        db.exec("INSERT INTO keys VALUES (nullptr,'"+name+"','"+path+"',nullptr,"+(update?"1":"0")+","+(load?"1":"0")+",'"+comment+"')");
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

int main(int argc, char** argv) {
    init();

    if (argc>2 && strcmp(argv[1],"-a") == 0) {
        normal();
    }
    std::string pw1 = "aaaa",hashed;
    hashed = hash(pw1);
    std::cout<< hashed<<std::endl;
    //addKey("C:/users/tomik/.ssh/test.ppk","aaaa");
    //removeKey("aaaa");
    //editComment("aaaa.ppk","test.ppk");
    return 0;
}
