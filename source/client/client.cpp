#include "socket.hpp"
#include <exception>
#include <unistd.h>

void streamprint(std::string s){
    std::cout<<s;
}

int main(){
    std::string addr, server_addr;
    std::cout<<"Address: ";
    std::getline(std::cin,addr);
    
    std::cout<<"Connect to: ";
    std::getline(std::cin,server_addr);
    
    ClientSocket client = ClientSocket(addr);
    client.connectTo(server_addr);

    while(true){
        std::string msg;
        std::getline(std::cin,msg);
        Socket::write(client,msg);
        std::cout<<Socket::read(client);
    }   
}    