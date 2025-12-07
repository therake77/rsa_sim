#include <socket.hpp>
#include <rsa.hpp>
#include <iostream>
void f(ServerSocket* s){
    std::vector<ClientSocket> clients = s->getCurrentClients();
    for(auto c: clients){
        std::string msg = Socket::read(c);
        if(msg != ""){
            std::cout<<msg<<std::endl;
        }
        if(msg == std::string("stop")){
            std::cout<<"stopping..."<<std::endl;
            s->stop();
        }
    }
}

int main(){
    ServerSocket server = ServerSocket("mylocalhost:4000");
    server.setOperation(&f);
    server.waitForConnections(1);
    server.up();
}