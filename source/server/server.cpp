#include <iostream>
#include <socket.hpp>
#include <rsa.hpp>
#include <cmd.hpp>
#include <algorithm>
#include <cctype>

void f(ServerSocket* s){
    std::vector<ClientSocket> clients = s->getCurrentClients();
    for(auto c: clients){
        std::string msg = Socket::read(c);
        std::transform(msg.begin(),msg.end(),msg.begin(),[](unsigned char c){return std::toupper(c);});
        int i = parsecmd(msg);
        RSA_Container r = *(s->getObject<RSA_Container>("rsa_obj"));
        switch(i){
            case CREATE_KEYS:
                r.generateKeys();
                Socket::write(c,std::to_string(r.n)+","+std::to_string(r.e));
                break;
            case START_COMMS:
                if(!r.hasValidKeys)
                    r.generateKeys();
                Socket::write(c,std::to_string(r.n)+","+std::to_string(r.e));
                break;
            case STOP:
                s->stop();
                break;
            case SENDMSG:
                std::vector<std::string> blocks = toblocks(msg,4);
                for(auto i : blocks){
                    std::cout<<i<<" ";
                }
                Socket::write(c,"Message readed\n");
        }
    }
}

int main(){
    
    std::string addr;
    std::cout<<"Address: ";
    std::getline(std::cin,addr);

    ServerSocket server = ServerSocket(addr);
    RSA_Container r = RSA_Container();
    server.attachObject<RSA_Container>("rsa_obj",std::make_shared<RSA_Container>(r));
    server.setOperation(&f);
    server.waitForConnections(1);
    server.up();
}