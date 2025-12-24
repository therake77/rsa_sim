#include <cmd.hpp>
#include <algorithm>
#include <cctype>

int parsecmd(std::string cmd){
    if(cmd.size() > 4){
        return SENDMSG;
    }
    if(cmd == START_COMMS_STR){
        return START_COMMS;
    }
    if(cmd == CREATE_KEYS_STR){
        return CREATE_KEYS;
    }
    if(cmd == STOP_STR){
        return STOP;
    }
    return SENDMSG;
}

std::vector<std::string> toblocks(std::string msg,int blocksize){
    std::vector<std::string> result;
    int rsize = msg.size()%blocksize;
    if(rsize != 0){
        msg.append(std::string(" ",rsize));
    }
    for(int i = 0 ; i < msg.size(); i+=blocksize){
        result.push_back(msg.substr(i,blocksize));
    }
    return result;
}

void upperInPlace(std::string s){

}