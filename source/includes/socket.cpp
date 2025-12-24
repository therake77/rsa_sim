#include "socket.hpp"
#include <unistd.h>
#include <exception>
#include <fcntl.h>
#include <poll.h>

Socket::Socket(const std::string addr): addr(addr){
    if(!start()){
        throw std::exception();
    }
}
Socket::~Socket(){
    close(this->socket_fd);
}

bool Socket::start(){
    unlink(addr.c_str());
    socket_fd = socket(PF_FILE,SOCK_STREAM,0);
    memset(&socket_addr,0,sizeof(socket_addr));
    socket_addr.sun_family = AF_FILE;
    strcpy(socket_addr.sun_path,addr.c_str());
    if(bind(socket_fd,(struct sockaddr *) &socket_addr,sizeof(socket_addr)) < 0){
        return false;
    }
    return true;
}

std::string Socket::read(Socket s){
    char buff[2048];
    memset(&buff,0,sizeof(buff));
    char c;
    int i = 0;
    size_t read_len = 0;
    while(read_len < 2048){
        int n = recv(s.socket_fd,&c,1,0);
        if(n == 1){
            if(c == '\0'){
                break;
            }
            buff[i++] = c;
            read_len++;
        }
        if((n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) || n == 0){
            break;
        }
    }
    return std::string(buff,read_len);
}


void Socket::write(Socket s, std::string msg){
    const char* buff = msg.c_str();
    size_t len = msg.size()+1;
    size_t written = 0;
    while(written < len){
        int n = send(s.socket_fd,buff + written,len-written,0);
        if(n < 0){
            if(errno == EINTR){
                continue;
            }
            break;
        }
        written+=n;
    }
    return;
}

ClientSocket::ClientSocket(std::string addr) : Socket(addr){}
ClientSocket::ClientSocket(int fd, sockaddr_un addr) : Socket(addr.sun_path){
    this->socket_fd = fd;
    this->socket_addr = addr;
}

bool ClientSocket::connectTo(std::string addr){
    struct sockaddr_un dest_addr;
    memset(&dest_addr,0,sizeof(dest_addr));
    dest_addr.sun_family = AF_FILE;
    strcpy(dest_addr.sun_path,addr.c_str());

    if(connect(this->socket_fd,(struct sockaddr *)&dest_addr,sizeof(dest_addr)) < 0){
        return false;
    }
    return true;
}

int ClientSocket::getid(){
    return this->socket_fd;
}


ServerSocket::ServerSocket(std::string addr) : Socket(addr){}

bool ServerSocket::waitForConnections(int n_conn){
    if(listen(this->socket_fd,n_conn)){
        return false;
    }
    int flags = fcntl(this->socket_fd,F_GETFL);
    if(fcntl(this->socket_fd,F_SETFL,flags | O_NONBLOCK) < 0){
        return false;
    }
    return true;
}

void ServerSocket::up(){
    this->active = true;
    pollfd server_pollfd = {this->socket_fd,POLLIN,0};
    std::vector<pollfd> to_watch;
    to_watch.push_back(server_pollfd);

    while(active){
        //check for events
        int poll_st = poll(to_watch.data(),to_watch.size(),100);
        
        if(poll_st < 0){
            throw std::exception();
        }

        //check events
        if(to_watch[0].revents & POLLIN){  //new connection event
            struct sockaddr_un client_addr;
            socklen_t len = sizeof(client_addr);
            memset(&client_addr,0,sizeof(client_addr));
            int client_fd = accept(this->socket_fd,(struct sockaddr *)&client_addr,&len);
            to_watch.push_back({client_fd,0,0});
            this->addConnection(ClientSocket(client_fd,client_addr));
        }
        
        for(auto p = to_watch.begin(); p!=to_watch.end();){
            if((*p).revents & (POLLERR | POLLHUP )){//disconnection event
                std::cout<<"Removing: "<<(*p).fd<<std::endl;
                this->removeConnection((*p).fd);
                close((*p).fd);
                p = to_watch.erase(p);   
            }else{
                p++;
            }
        }
        
        //execute user function
        f(this);
    }
}

void ServerSocket::stop(){
    this->active = false;
    close(this->socket_fd);
}

void ServerSocket::removeConnection(int fd){
    for(auto it = this->conn_pool.begin(); it!=this->conn_pool.end(); it++){
        if((*it).getid() == fd){
            this->conn_pool.erase(it);
            return;
        }
    }
    return;
}

int ServerSocket::getid(){
    return this->socket_fd;
}

    /*Returns a copy of the client pool*/
std::vector<ClientSocket> ServerSocket::getCurrentClients(){
    return std::vector(this->conn_pool);
}

void ServerSocket::setOperation(void (*f)(ServerSocket *)){
    this->f = f;
}

void ServerSocket::addConnection(ClientSocket s){
    this->conn_pool.push_back(s);
}