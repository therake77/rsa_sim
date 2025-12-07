#include <socket.hpp>
#include <iostream>
void f(ServerSocket* s){
    std::vector<ClientSocket> clients = s->getCurrentClients();
    for(auto c: clients){
        std::string s = Socket::read(c);
        if(s != ""){
            std::cout<<s<<std::endl;
        }
    }
}

int main(){
    ServerSocket server = ServerSocket("mylocalhost:4000");
    server.setOperation(&f);
    server.waitForConnections(1);
    server.up();
}