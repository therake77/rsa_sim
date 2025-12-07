#include "socket.hpp"
#include <exception>
#include <unistd.h>
int main(){
    ClientSocket client = ClientSocket("myclienthost:0001");
    client.connectTo("mylocalhost:4000");
    while(true){
        std::string msg;
        std::getline(std::cin,msg);
        Socket::write(client,msg);
    }   
}    