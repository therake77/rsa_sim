#include <cmd.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <string.h>
#include <list>

int parsecmd(std::string cmd){
    if(cmd.size() > 4){
        return BAD;
    }
    if(cmd == REQUEST_KEYS_STR){
        return REQUEST_KEYS;
    }
    if(cmd == SEND_NONENCRYPTED_MSG_STR){
        return SEND_NONENCRYPTED_MSG;
    }
    if(cmd == SEND_ENCRYPTED_MSG_STR){
        return SEND_ENCRYPTED_MSG;
    }
    if(cmd == STOP_SERVER_MSG_STR){
        return STOP;
    }
    if(cmd == INVALIDATE_KEYS_STR){
        return INVALIDATE_KEYS;
    }
    return BAD;
}

std::vector<std::string> toblocks(std::string msg,int blocksize){
    std::vector<std::string> result;
    int rsize = msg.size()%blocksize;
    if(rsize != 0){
        msg.append(std::string(blocksize-rsize,' '));
    }
    for(int i = 0 ; i < msg.size(); i+=blocksize){
        result.push_back(msg.substr(i,blocksize));
    }
    return result;
}

uint64_t stouint64(std::string s){
    //considering our padding scheme, this is necessary
    size_t size = strlen(s.data());

    if(size > 8){
        return -1;
    }
    uint64_t result = 0;
    for(size_t j = 0; j < size; j++){
        uint64_t n_c = ((unsigned char) s[j])*pow(256,size-1-j);
        result+= n_c;
    }
    return result;
}

std::string uint64tos(uint64_t num){
    std::string s;
    while(num > 0){
        u_char c = num & 0xFF;
        s.push_back(c);
        num >>= 8;
    }
    std::reverse(s.begin(),s.end());
    return s;
}

void c_print(std::string s){
    for(unsigned char c : s){
        std::cout<< (uint) c <<" ";
    }
    std::cout<<std::endl;
    return;
}