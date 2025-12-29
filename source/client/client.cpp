#include <exception>
#include <unistd.h>
#include <algorithm>
#include <cmd.hpp>
#include <poll.h>
#include <signal.h>
#include <socket.hpp>
#include <rsa.hpp>

void getcmd(std::string line,std::string& cmd,std::string& msg){
    std::transform(line.begin(), line.end(), line.begin(),[](unsigned char c){return std::toupper(c);});
    size_t i = line.find(" ");
    if(i == std::string::npos){
        //then the entire line is a command. send the command
        cmd.append(line);
    }else{
        //from 0 to i-1 is a command, next is a string
        cmd.append(line.substr(0,i));
        msg.append(line.substr(i+1,line.size()-i));        
    }
}

int main(){
    signal(SIGPIPE, SIG_IGN);
    std::string addr, server_addr;
    std::cout<<"Address: ";
    std::getline(std::cin,addr);
    
    std::cout<<"Connect to: ";
    std::getline(std::cin,server_addr);
    
    Client client = Client(addr);
    if(!client.connect_to(server_addr)){
        throw std::exception();
    }
    pollfd p = {client.get_fd(),POLLIN,0};

    while(true){
        std::string line;
        std::getline(std::cin,line);
        std::string cmd,msg;
        getcmd(line,cmd,msg);

        if(cmd == REQUEST_KEYS_STR){
            Socket::write(client.get_sock(),cmd);
            poll(&p,1,-1);
            uint64_t mod = std::stoull(Socket::read(client.get_sock()));
            poll(&p,1,-1);
            uint64_t exp = std::stoull(Socket::read(client.get_sock()));
            std::cout<<"Received: "<<mod<<" "<<exp<<std::endl;
            client.attach_object<uint64_t>("public_mod",std::make_shared<uint64_t>(mod));
            client.attach_object<uint64_t>("public_exp",std::make_shared<uint64_t>(exp));
        }else if(cmd == STOP_SERVER_MSG_STR){
            Socket::write(client.get_sock(),cmd);
            poll(&p,1,-1);
            std::string response = Socket::read(client.get_sock());
            std::cout << response << std::endl;
            if(response == SHUTDOWN_STR){
                std::cout<<"Exiting..."<<std::endl;
                break;
            }else{
                std::cout<<"Exiting failed..."<<std::endl;
                continue;
            }
        }else if(cmd == SEND_ENCRYPTED_MSG_STR && !msg.empty()){
            std::shared_ptr<uint64_t> public_mod_p = client.get_object<uint64_t>("public_mod"); 
            std::shared_ptr<uint64_t> public_exp_p = client.get_object<uint64_t>("public_exp");
            if(public_mod_p == nullptr || public_exp_p == nullptr){
                std::cout<<"No key saved was found. Request keys using REQK"<<std::endl;
                continue;
            }
            std::string encrypted_msg = RSA_Container::encrypt(*public_mod_p,*public_exp_p,msg);
            std::cout<<encrypted_msg<<std::endl;
            c_print(encrypted_msg);
            std::cout<<encrypted_msg.size()<<std::endl;  
            Socket::write(client.get_sock(),cmd);
            Socket::write(client.get_sock(),encrypted_msg);
            poll(&p,1,-1);
            std::string response = Socket::read(client.get_sock());
            std::cout << response << std::endl;

        }else if(cmd == SEND_NONENCRYPTED_MSG_STR && !msg.empty()){
            Socket::write(client.get_sock(),cmd);
            Socket::write(client.get_sock(),msg);
            poll(&p,1,-1);
            std::string response = Socket::read(client.get_sock());
            std::cout << response << std::endl;

        }else if(cmd == INVALIDATE_KEYS_STR){
            Socket::write(client.get_sock(),cmd);
            poll(&p,1,-1);
            std::string response = Socket::read(client.get_sock());
            std::cout << response << std::endl;
        }else{
            std::cout << "Command ignored..."<< std::endl;
        }
    }   
}    