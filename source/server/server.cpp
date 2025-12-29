#include <iostream>
#include <socket.hpp>
#include <rsa.hpp>
#include <cmd.hpp>
#include <algorithm>
#include <cctype>

void f(Server* s, const Socket& peer){
    std::string msg = Socket::read(peer);
    std::transform(msg.begin(),msg.end(),msg.begin(),[](unsigned char c){return std::toupper(c);});
    std::cout<<"Received message: "<<msg<<std::endl;
    int i = parsecmd(msg);
    RSA_Container& r = *(s->get_object<RSA_Container>("rsa_obj"));
    switch(i){
        case REQUEST_KEYS:
            if(!r.hasValidKeys){
                while(!r.hasValidKeys){
                    try{
                        r.generateKeys();
                    }catch(std::exception e){
                        std::cout<<"Invalid keys event"<<std::endl;
                    }
                }
            }
            Socket::write(peer,std::to_string(r.n));
            Socket::write(peer,std::to_string(r.e));
            break;
        case STOP:
            s->stop();
            Socket::write(peer,SHUTDOWN_STR);
            break;
        case SEND_ENCRYPTED_MSG:{
            if(!r.hasValidKeys){
                Socket::read(peer);
                Socket::write(peer,BAD_CMD_STR);
                break;
            }
            std::string encrypted_msg = Socket::read(peer);
            std::cout<<encrypted_msg<<std::endl;
            std::string decrypted = RSA_Container::decrypt(r.n,r.d,encrypted_msg);
            std::cout<<decrypted<<std::endl;
            Socket::write(peer,OK_STR);
            break;
        }
        case SEND_NONENCRYPTED_MSG:{
            std::string non_encrp_msg = Socket::read(peer);
            std::cout<<non_encrp_msg<<std::endl;
            Socket::write(peer,OK_STR);
            break;
        }
        case INVALIDATE_KEYS:{
            r.invalidateKeys();
            Socket::write(peer,OK_STR);
            break;
        }
        case BAD:{
            Socket::write(peer,BAD_CMD_STR);
            break;
        }
    }

}

int main(){
    
    std::string addr;
    std::cout<<"Address: ";
    std::getline(std::cin,addr);

    Server server = Server(addr);
    RSA_Container r = RSA_Container();
    
    try{
        r.generateKeys();
    }catch(std::exception e){
        std::cout << "An error happened generating keys..."<<std::endl;
        return 0;
    }

    server.attach_object<RSA_Container>("rsa_obj",std::make_shared<RSA_Container>(r));
    server.set_operation(&f);
    server.wait_for_comms(1);
    server.up();
    return 0;
}