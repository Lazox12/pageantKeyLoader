#include <fstream>
#include <stdint.h>
#include <base64.h>
#include <hmac_sha256.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <Argon2.h>

#define SHA256_HASH_SIZE 32

static uint8_t* PUT_32BIT_MSB_FIRST(uint32_t value)
{
    auto *p = (uint8_t*)malloc(4);
    p[3] = (uint8_t)(value);
    p[2] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)(value >> 16);
    p[0] = (uint8_t)(value >> 24);
    return p;
}

struct Buffer {
    uint8_t* Buff = nullptr;
    int Size = 0;
    uint8_t* curr = nullptr;

    bool innit(int size) {
        this->Size = size;
        this->Buff = (uint8_t*)malloc(sizeof(uint8_t) * size);
        if (this->Buff == nullptr) {
            return false;
        }
        this->curr = this->Buff;
        return true;
    }
    bool append(const void* data, int size) {
        if (this->curr + size > this->Buff + this->Size) {
            return false;
        }
        if (data == nullptr) {
            return false;
        }
        memcpy(this->curr, data, size);
        this->curr += size;
        return true;
    }
    int getSize() {
        return this->curr- this->Buff;
    }

};

class sshKey{
    public:
        std::string alg;
        std::string encription;
        std::string comment;
        std::string publicKey;
        std::string privateKey;
        std::string mac;
        std::string Derivation;
        std::string Argon2Memory;
        std::string Argon2Passes;
        std::string Argon2Parallelism;
        std::string Argon2Salt;

        void parseKey(std::ifstream & ifs){
            std::string line;
            while (std::getline(ifs,line)) {
                if(line.find("PuTTY-User-Key-File-3") != std::string::npos) {
                    this->alg = line.substr(line.find(": ")+2,line.length());
                    continue;
                }
                if(line.find("Encryption") != std::string::npos) {
                    this->encription = line.substr(line.find(": ")+2,line.length());
                    continue;
                }
                if(line.find("Comment") != std::string::npos) {
                    this->comment = line.substr(line.find(": ")+2,line.length());
                    continue;
                }
                if(line.find("Public-Lines") != std::string::npos) {
                    int cnt = atoi(line.substr(line.find(": ")+2,line.length()).c_str());
                    for(int i=0;i<cnt;i++){
                      std::getline(ifs,line);
                      this->publicKey.append(line);
                    }
                    continue;
                }
                if(line.find("Private-Lines") != std::string::npos) {
                    int cnt = atoi(line.substr(line.find(": ")+2,line.length()).c_str());
                    for(int i=0;i<cnt;i++){
                        std::getline(ifs,line);
                        this->privateKey.append(line);
                    }
                    continue;
                }
                if(line.find("Private-MAC") != std::string::npos) {
                    this->mac = line.substr(line.find(": ")+2,line.length());
                    continue;
                }
                if(this->encription != "none") {
                    if(line.find("Key-Derivation") != std::string::npos) {
                        this->Derivation = line.substr(line.find(": ")+2,line.length());
                        continue;
                    }
                    if(line.find("Argon2-Memory") != std::string::npos) {
                        this->Argon2Memory = line.substr(line.find(": ")+2,line.length());
                        continue;
                    }
                    if(line.find("Argon2-Passes") != std::string::npos) {
                        this->Argon2Passes = line.substr(line.find(": ")+2,line.length());
                        continue;
                    }
                    if(line.find("Argon2-Parallelism") != std::string::npos) {
                        this->Argon2Parallelism = line.substr(line.find(": ")+2,line.length());
                        continue;
                    }
                    if(line.find("Argon2-Salt") != std::string::npos) {
                        this->Argon2Salt = line.substr(line.find(": ")+2,line.length());
                    }
                }
            }
            if (this->mac != generateMac()) {
                throw std::invalid_argument("invalid key macError");
            }
        }
        void decodeData(const std::string& password = "") {
            this->publicKey = base64_decode(this->publicKey);
            this->privateKey = base64_decode(this->privateKey);
        }
        std::string generateMac() {

            buff.innit(2000);
            uint8_t zero[3] = {0,0,0};
            std::string* input[] = {&this->alg,&this->encription,&this->comment,&this->publicKey,&this->privateKey};

            for (auto i:input) {
                uint32_t size = i->size();
                buff.append(PUT_32BIT_MSB_FIRST(size),sizeof(uint32_t));
                buff.append((*i).c_str(),size);
            }
            std::vector<uint8_t> out(SHA256_HASH_SIZE);
            std::stringstream ss_result;
            hmac_sha256("",1,buff.Buff,buff.getSize(),out.data(),out.size());

            for (uint8_t x : out) {
                ss_result << std::hex << std::setfill('0') << std::setw(2) << (int)x;
            }
            return ss_result.str();


        }
    private:
        Buffer buff;

};