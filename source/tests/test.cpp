#include <rsa.hpp>
#include <cmd.hpp>
#include <iostream>
#include <list>
#include <algorithm>

int main(){
    std::string s;
    std::getline(std::cin,s);
    std::vector<std::string> blocks = toblocks(s,7);
    for(std::string b: blocks){
        std::cout<<b<<std::endl;
        c_print(b);
    }
}