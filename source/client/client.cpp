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
    
    //first: we should read the keys our server send to communicate
    poll(&p,1,-1);
    std::string keystring = Socket::read(client.get_sock());
    
    uint64_t n;
    uint64_t e;
    bool key_st = parse_keystring(keystring,n,e); 
    if(!key_st){
        std::cout<<"Bad keystring\n";
        throw std::exception();
    }
    std::cout<<"Keys received: \n"<<"n: "<<n<<" e: "<<e<<std::endl;
    //By now we should have the keys
    //Every command is going to be encrypted from this point
    int control = true;
    while(control){
        
        //First: get the command from stdin
        std::string cmd;
        std::getline(std::cin,cmd);
        if(cmd.size() == 0){
            continue;
        }
        std::string encrypted_msg = RSA_Container::encrypt(n,e,cmd);
        std::cout<<"Encrypted msg: "<<encrypted_msg<<std::endl;
        std::cout<<"Encrypted message (ASCII): ";
        c_print(encrypted_msg);
        Socket::write(client.get_sock(),encrypted_msg);
        //The server will return a response
        poll(&p,1,-1);
        std::string response = Socket::read(client.get_sock());
        //Parse the response (response codes defined in cmd.hpp)
        std::pair<int,std::string> parsed_response = parse_response(response);
        switch(parsed_response.first){
            case BAD:{
                std::cout<<"Bad command\n";
                break;
            }
            case MSG:{
                std::cout<<"Peer says: "<<parsed_response.second<<std::endl;
                if(parsed_response.second == SHUTDOWN_STR){
                    control = false;
                    break;
                }
                break;
            }
            case KEY:{
                parse_keystring(parsed_response.second,n,e);
                break;
            }case ID:{
                std::string proof = RSA_Container::decrypt(n,e,parsed_response.second);
                std::cout<<"Proof of identity is: "<<proof<<std::endl;
                break;
            }
            default:
                std::cout<<"Something bad has happened...\n";
                break;
        }
        
    }
}    