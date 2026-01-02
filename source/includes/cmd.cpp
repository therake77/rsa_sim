#include <cmd.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <string.h>
#include <list>

std::string trim(std::string s){
    bool flag = true;
    int k = 0;
    while(flag){
        if(s.at(k) != ' '){
            flag = false;
            continue;
        }
        k++;
    }
    s = s.substr(k,s.size()-k);
    flag = true;
    k = s.size()-1;
    while(flag){
        if(s.at(k) != ' '){
            flag = false;
            continue;
        }
        k--;
    }
    s = s.substr(0,k+1);
    return s;
}


bool parse_cmd(std::string cmd,int& __cmd,std::string& payload){
    cmd = trim(cmd);
    std::cout<<"command trimmed: "<<cmd<<std::endl;
    int i = cmd.find(' ');
    std::string _cmd = "";
    std::string _msg = "";
    if(i != std::string::npos){
        _cmd = cmd.substr(0,i);
        _msg = cmd.substr(i+1,cmd.size()-i-1);
    }else{
        _cmd = cmd;
    }
    std::transform(_cmd.begin(),_cmd.end(),_cmd.begin(),[](const char c){return std::toupper(c);});
    //now starts the comparisons
    std::cout<<"command: "<<_cmd<<std::endl;
    if(_cmd == REQUEST_KEYS_STR){
        __cmd = REQUEST_KEYS;
        return true;
    }else if( _cmd == SEND_MSG_STR){
        __cmd = SEND_MSG;
        if(_msg.size() != 0){
            payload = _msg;
            return true;
        }
        return false;
    }else if( _cmd == STOP_SERVER_STR){
        __cmd = STOP;
        return true;
    }else if( _cmd == SEND_PROOF_OF_ID_STR){
        __cmd = SEND_PROOF_OF_ID;
        return true;
    }else{
        __cmd = BAD;
        return false;
    }
}

bool parse_keystring(std::string keystring,uint64_t& mod, uint64_t& exp){
    int i = keystring.find(" ");
    if(i == std::string::npos){
        return false;
    }
    try{
        mod = std::stoull(keystring.substr(0,i));
        exp = std::stoull(keystring.substr(i+1,keystring.size()-i+1));
    }catch(std::exception e){
        return false;
    }
    return true;
}

std::string build_response(int __res_code, std::string args){
    switch(__res_code){
        case MSG:{
            return std::string(MSG_STR)+":"+args;
        }
        case KEY:{
            return std::string(KEY_STR)+":"+args;
        }
        case ID:{
            return std::string(ID_STR)+":"+args;
        }
        default:
            return std::string(MSG_STR)+":"+"Bad response";
    }
}

std::pair<int,std::string> parse_response(std::string msg){
    int i = msg.find(":");
    if(i == std::string::npos){
        return {BAD,""};
    }
    std::string _key = msg.substr(0,i);
    std::string _args = msg.substr(i+1,msg.length()-i-1);
    //now get the message
    if(_key == MSG_STR){
        return {MSG,_args};
    }
    if(_key == KEY_STR){
        return {KEY,_args};
    }
    if(_key == ID_STR){
        return {ID,_args};
    }
    return {BAD,""};
}

std::vector<std::string> toblocks(std::string msg,int blocksize){
    std::vector<std::string> result;
    for(int i = 0 ; i < msg.size(); i+=blocksize){
        if(i+blocksize>msg.size()){
            result.push_back(msg.substr(i,msg.size()-i+1));
            continue;
        }
        result.push_back(msg.substr(i,blocksize));
    }
    result[result.size()-1] = std::string(blocksize-result[result.size()-1].length(),'\0') + result[result.size()-1];
    return result;
}

uint64_t stouint64(std::string s){
    if(s.size() > 8){
        return -1;
    }
    uint64_t result = 0;
    for(size_t j = 0; j < s.size(); j++){
        uint64_t n_c = ((unsigned char) s[j])*pow(256,s.size()-1-j);
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