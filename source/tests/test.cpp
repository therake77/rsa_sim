#include <rsa.hpp>
#include <cmd.hpp>
#include <iostream>
#include <list>
#include <algorithm>

int main(){
    /*std::string msg = "helloaer";
    uint64_t num= 0;
    for(int j = msg.size()-1; j >=0 ; j--){
        uint32_t ascii = ((unsigned char) msg[msg.size()-1 - j]);
        std::cout<<ascii<<std::endl;
        num +=(ascii*pow(256,j));
    }
    std::cout<<num<<std::endl;
    std::string s;
    while(num > 0){
        u_char c = (num & 0xFF00000000000000)>>56;
        if(c == 0){
            std::cout<<num<<std::endl;
            num <<= 8;
            continue;
        }
        std::cout<<c<<std::endl;
        s.push_back(c);
        num <<= 8;
        std::cout<<num<<std::endl;
    }
    std::cout << s<<std::endl;
    */
    unsigned char arr[] = {83, 244, 223, 140, 82, 216, 134, 104};
    std::string s = std::string(reinterpret_cast<char*>(arr), 8);
    std::string s2 = uint64tos(stouint64(s)) ;
    std::cout << (s==s2) <<std::endl;
    c_print(s);
    c_print(s2);
}