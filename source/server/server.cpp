#include <iostream>
#include <socket.hpp>
#include <rsa.hpp>
#include <cmd.hpp>
#include <algorithm>
#include <cctype>

void welcome(Server* s, const Socket& peer){
    
    std::cout<<"Client with address "<<peer.get_sock_addr()<<" has connected"<<std::endl;
    RSA_Container& r = s->get_object<RSA_Container>("rsa_obj");
    std::string keystring = std::to_string(r.n)+" "+std::to_string(r.e);
    std::cout<<"Keys sent"<<std::endl;
    Socket::write(peer,keystring);
    
}

void f(Server* s, const Socket& peer){
    
    RSA_Container& r = (s->get_object<RSA_Container>("rsa_obj"));
    std::string encrypted_msg = Socket::read(peer);
    std::cout<<"Encrypted message: "<<encrypted_msg<<" Length: "<<encrypted_msg.length()<<std::endl;
    std::cout<<"Encrypted message (ASCII): ";
    c_print(encrypted_msg);
    std::string decrypted_msg = RSA_Container::decrypt(r.n,r.d,encrypted_msg);
    std::cout<<"Decrypted message: "<<decrypted_msg<<" Length: "<<decrypted_msg.length()<<std::endl;
    std::cout<<"Decrypted message (ASCII): ";
    c_print(decrypted_msg);
    int cmd_code;
    std::string args;
    bool cmd_st = parse_cmd(decrypted_msg,cmd_code,args);
    switch(cmd_code){
        case REQUEST_KEYS: {
            std::cout<<"Peer requested new key generation..."<<std::endl;
            r.generateKeys();
            std::cout<<"New key generation:\np: "<<r.p<<" q:"<<r.q<<"\nn: "<<r.n<<" e: "<<r.e<<" d: "<<r.d<<std::endl;
            Socket::write(peer,build_response(KEY,std::to_string(r.n)+" "+std::to_string(r.e)));
            break;
        }
        
        case SEND_MSG: {
            std::cout<<"Peer sent a message: ";
            std::cout<<args<<std::endl;
            Socket::write(peer,build_response(MSG,OK_STR));
            break;
        }

        case STOP:{
            std::cout<<"Peer request shutdown \n";
            Socket::write(peer,build_response(MSG,SHUTDOWN_STR));
            s->stop();
            std::cout<<"Shutdown...\n";
            break;  
        }
        
        case SEND_PROOF_OF_ID:{
            std::cout<<"Peer requested proof of identity \n";
            std::string proof = RSA_Container::encrypt(r.n,r.d,"SERVER");
            Socket::write(peer,build_response(ID,proof));
            break;
        }
        
        default:
            std::cout<<"Command couldn't be parsed \n";
            Socket::write(peer,build_response(MSG,BAD_CMD_STR));
            break;
    }
}

int main(){
    
    std::string addr;
    std::cout<<"Address: ";
    std::getline(std::cin,addr);

    Server server = Server(addr);
    RSA_Container r = RSA_Container();
    std::cout<<"Keys generated:\n";
    std::cout<<"p: "<<r.p<<" q: "<<r.q<<" n: "<<r.n<<std::endl;
    std::cout<<"e: "<<r.e<<std::endl;
    std::cout<<"d: "<<r.d<<std::endl;
    server.attach_object("rsa_obj",std::make_unique<std::any>(std::make_any<RSA_Container>(r)));
    server.set_operation(&f);
    server.set_welcome_op(&welcome);
    server.wait_for_comms(5);
    server.up();
    std::cout<<"Exited"<<std::endl;
    return 0;
}