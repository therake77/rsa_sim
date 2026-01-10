#include <iostream>
#include <socket.hpp>
#include <rsa.hpp>
#include <cmd.hpp>
#include <algorithm>
#include <cctype>

void welcome(Server* s, const Socket& peer){
    RSA_Container& r = s->get_object<RSA_Container>("rsa_obj");
    std::string keystring = std::to_string(r.n)+" "+std::to_string(r.e);
    Socket::write(peer,keystring);
}

void f(Server* s, const Socket& peer){
    RSA_Container& r = (s->get_object<RSA_Container>("rsa_obj"));
    std::string encrypted_msg = Socket::read(peer);
    std::cout<<"Encrypted message: "<<encrypted_msg<<" Length: "<<encrypted_msg.length()<<std::endl;
    c_print(encrypted_msg);
    std::string decrypted_msg = RSA_Container::decrypt(r.n,r.d,encrypted_msg);
    std::cout<<"Decrypted message: "<<decrypted_msg<<" Length: "<<decrypted_msg.length()<<std::endl;
    c_print(decrypted_msg);
    int cmd_code;
    std::string args;
    bool cmd_st = parse_cmd(decrypted_msg,cmd_code,args);
    std::cout<<"code: "<<cmd_code<<" args: "<<args<<std::endl;
    switch(cmd_code){
        case REQUEST_KEYS: {
            r.generateKeys();
            Socket::write(peer,build_response(KEY,std::to_string(r.n)+" "+std::to_string(r.e)));
            break;
        }
        
        case SEND_MSG: {
            std::cout<<args<<std::endl;
            Socket::write(peer,build_response(MSG,OK_STR));
            break;
        }

        case STOP:{
            Socket::write(peer,build_response(MSG,SHUTDOWN_STR));
            s->stop();
            break;
        }
        
        case SEND_PROOF_OF_ID:{
            std::string proof = RSA_Container::encrypt(r.n,r.d,"SERVER");
            Socket::write(peer,build_response(ID,proof));
            break;
        }
        
        default:
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
    std::any any_r = std::make_any<RSA_Container>(r);
    server.attach_object("rsa_obj",std::unique_ptr<std::any>(&any_r));
    server.set_operation(&f);
    server.set_welcome_op(&welcome);
    server.wait_for_comms(5);
    server.up();
    return 0;
}