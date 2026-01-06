#include <rsa.hpp>
#include <cmd.hpp>
#include <iostream>
#include <list>
#include <algorithm>
#include <cctype>

int main(){
    uint64_t num;
    std::cin>>num;
    std::cout<<num<<std::endl;
    uint64_t p = pollard_rho_fact(num,2,1);
    if(p == 0){
        std::cout<<"failed"<<std::endl;
    }
    if(num % p != 0){
        std::cout<<"invalid factor"<<std::endl;
    }
    uint64_t q = num/p;
    std::cout<<p<<std::endl;
    std::cout<<q<<std::endl;
    std::cout<<((uint64_t)p*q == num ? "true" : "false" )<<std::endl;
    return 0;
}