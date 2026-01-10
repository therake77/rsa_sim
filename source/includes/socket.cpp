#include <socket.hpp>
#include <unistd.h>
#include <arpa/inet.h>
#include <exception>
#include <fcntl.h>
#include <poll.h>
#include <algorithm>
#include <stdio.h>
#include <cstdint>
/*-------------------------------------Start of Socket definitions-------------------------------------*/

/*Constructor of Socket class. Expects an address for UNIX socket*/
Socket::Socket(const std::string addr): addr(addr){
    if(!start()){
        throw std::exception();
    }
}

Socket::Socket(int __fd, struct sockaddr_un _sock_addr) : addr(_sock_addr.sun_path){
    this->socket_fd = __fd;
    this->socket_addr = _sock_addr;
}

bool Socket::operator==(const Socket& s){
    return this->socket_fd == s.socket_fd;
} 

/*Move operator and constructor*/
Socket::Socket(Socket&& sock) noexcept : addr(sock.addr){
    this->socket_fd = sock.socket_fd;
    this->socket_addr = sock.socket_addr;
    /*Remove ownership of first socket*/
    sock.socket_fd = -1;
}

Socket& Socket::operator=(Socket&& sock) noexcept{
    if(this != &sock){
        this->socket_fd = sock.socket_fd;
        this->socket_addr = sock.socket_addr;
        /*Remove ownership of first socket*/
        sock.socket_fd = -1;
    }
}


/*Destructor that closes the file descriptor*/
Socket::~Socket(){
    if(this->socket_fd >=0 ){
        close(this->socket_fd);
    }
}

bool Socket::start(){
    unlink(addr.c_str());
    socket_fd = socket(AF_UNIX,SOCK_STREAM,0);
    memset(&socket_addr,0,sizeof(socket_addr));
    socket_addr.sun_family = AF_FILE;
    strcpy(socket_addr.sun_path,addr.c_str());
    if(bind(socket_fd,(struct sockaddr *) &socket_addr,sizeof(socket_addr)) < 0){
        return false;
    }
    return true;
}

/*Allows to read from an specified Socket in a safe way. 
Expects the null character to work as delimiter between messages*/
std::string Socket::read(const Socket& s){
    //first we need to read the header:
    uint8_t serialized_header[4];
    size_t read_so_far = 0;
    while(read_so_far < sizeof(uint32_t)){
        int n = recv(s.socket_fd,serialized_header+read_so_far,sizeof(uint32_t) - read_so_far,0);
        if(n == 0){
            //peer closed connection
            return "";
        }
        if(n < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                continue;
            }
            perror("Error at reading");
            return "";
        }
        read_so_far+=n;
    }
    
    //by now we should have the header.
    uint32_t msg_lenght = 0UL;
    memcpy(&msg_lenght,serialized_header,sizeof(serialized_header));
    msg_lenght = ntohl(msg_lenght);
    
    //now read the message
    read_so_far = 0;
    unsigned char buff[msg_lenght];
    memset(buff,0,sizeof(buff));

    while(read_so_far < msg_lenght){
        int n = recv(s.socket_fd,buff + read_so_far,sizeof(buff) - read_so_far, 0);
        if (n == 0){
            //Peer closed connection
            return "";
        }
        if(n < 0 ){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                continue;
            }
            perror("negative n");
            return "";
        }
        read_so_far += n;
    }
    return std::string(reinterpret_cast<char*>(buff),msg_lenght);
}

/*
Allows writing directly over an specified Socket. 
We strongly recommend to null-terminate your message, or the program would stall
*/

void Socket::write(const Socket& s, std::string msg){
    const char* buff = msg.c_str();
    size_t len = msg.size();
    if(len > (uint32_t) (-1)){
        return;
    }
    size_t written = 0;
    //first: write the length of the message to receive:
    uint32_t header = htonl(msg.size()); 
    uint8_t serialized_header[4];
    memcpy(serialized_header,&header,sizeof(uint32_t));

    while(written < sizeof(header)){
        int n = send(s.socket_fd, serialized_header + written, sizeof(header) - written,0);
        if(n < 0){
            if(errno == EINTR){
                continue;
            }
            perror("Write failed");
            return;
        }
        written+=n;
    }
    //now the header was sent. send the message
    written = 0;
    while(written < len){
        int n = send(s.socket_fd,buff + written,len-written,0);
        if(n < 0){
            if(errno == EINTR){
                continue;
            }
            perror("negative n");
            break;
        }
        written+=n;
    }
    return;
}
/*Returns the file dscriptor of a given socket*/
int Socket::get_fd(){
    return this->socket_fd;
}



/*Allows connecting to another socket, with address addr*/
bool Socket::connect_to(std::string addr){
    struct sockaddr_un dest_addr;
    memset(&dest_addr,0,sizeof(dest_addr));
    dest_addr.sun_family = AF_FILE;
    strcpy(dest_addr.sun_path,addr.c_str());

    //start connection
    int conn_st = connect(this->socket_fd,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
    
    //an error occurred that is not EINPROGRESS
    if(conn_st < 0 && errno != EINPROGRESS ){
        perror("Error connecting to server");
        return false;
    }
    
    //wait for the socket to be writtable
    pollfd p = {this->socket_fd, POLLHUP|POLLERR|POLLOUT,0};
    
    //if some error happens at poll
    if(poll(&p,1,-1) < 0){
        perror("Poll");
        return false;
    }
    
    //check connection status
    int err;
    socklen_t len = sizeof(err);
    if (getsockopt(this->socket_fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        perror("getsockopt");
        return false;
    }

    if (err != 0) {
        errno = err;
        perror("connect failed");
        return false;
    }
    return true;
}

/*starts the socket on listening for connections*/
bool Socket::sock_listen(int max_conn){
    if(listen(this->socket_fd,max_conn)){
        return false;
    }
    //set the socket in a non-blocking mode
    int flags = fcntl(this->socket_fd,F_GETFL);
    if(fcntl(this->socket_fd,F_SETFL,flags | O_NONBLOCK) < 0){
        return false;
    }
    return true;
}

/*-------------------------------------End of Socket definitions-------------------------------------*/
/*-------------------------------------Start of Agent definitions-------------------------------------*/

Agent::Agent(std::string addr) : sock(addr){}

/*Returns the file descriptor of the socket*/
int Agent::get_fd(){
    return this->sock.get_fd();
}
/*Returns a unique pointer encapsulating the socket object*/
const Socket& Agent::get_sock() const{
    return this->sock;
}

bool Agent::attach_object(std::string obj_key, std::unique_ptr<std::any> obj_ptr){
    auto [it, emplaced] = this->customObjects.emplace(obj_key,std::move(obj_ptr));
    return emplaced;
}

/*-------------------------------------End of Agent definitions-------------------------------------*/
/*-------------------------------------Start of Client definitions-------------------------------------*/

Client::Client(std::string addr) : Agent(addr){}

/*Allows the client to connect to an specified agent via an address*/
bool Client::connect_to(std::string addr){
    if(!(this->sock).connect_to(addr)){
        return false;
    }
    return true;
}

/*-------------------------------------End of Client definitions-------------------------------------*/
/*-------------------------------------Start of Server definitions-------------------------------------*/

auto Server::add_conn(int _fd, sockaddr_un sock_addr){
    auto r = this->conn_pool.insert_or_assign(_fd,std::make_unique<Socket>(_fd,sock_addr));
    return r;
}

bool Server::remove_conn(int _fd){
    auto it = this->conn_pool.find(_fd);
    if(it != this->conn_pool.end()){
        this->conn_pool.erase(it);
        return true;
    }
    return false;
}

Server::Server(std::string addr) : Agent(addr){}

bool Server::wait_for_comms(int n_conns){
    if(!this->sock.sock_listen(n_conns)){
        return false;
    }
    return true;
}

void Server::set_operation(void (*f)(Server*,const Socket&)){
    this->op = f;
}

void Server::set_welcome_op(void (*f)(Server*,const Socket&)){
    this->welcome_op = f;
}

void Server::up(){
    this->running = true;
    pollfd server_pollfd = {this->sock.get_fd(),POLLIN,0};
    std::vector<pollfd> to_watch;
    to_watch.push_back(server_pollfd);

    while(running){
        //check for events
        int poll_st = poll(to_watch.data(),to_watch.size(),-1);
        if(poll_st == 0){ //No event has happened
            continue;
        }
        if(poll_st < 0){ //An error ocurred
            throw std::exception();
        }

        //first, we're going to check if server socket has crashed:
        if(to_watch[0].revents & (POLLERR | POLLHUP | POLLNVAL)){
            std::cout<<"Server socket error. Exiting..."<<std::endl;
            //Delete all sockets
            this->conn_pool.clear();
            break;
        }
        
        //Now, check if new connections have happened
        if(to_watch[0].revents & POLLIN){  //new connection event
            struct sockaddr_un client_addr;
            socklen_t len = sizeof(client_addr);
            memset(&client_addr,0,sizeof(client_addr));
            int client_fd = accept(this->sock.get_fd(),(struct sockaddr *)&client_addr,&len);
            
            //wait for the socket to be ready to write
            pollfd temp = {client_fd,POLLOUT,0};
            poll(&temp,1,-1);
            if(temp.revents & (POLLERR | POLLHUP | POLLRDHUP)){
                continue;    
            }
            auto t = this->add_conn(client_fd,client_addr);
            to_watch.push_back({client_fd,POLLIN,0});
            this->welcome_op(this,*(t.first->second));
        }
        
        //Now, check for event in peers sockets
        for(auto p = to_watch.begin()+1; p!=to_watch.end();){
            if((*p).revents & (POLLERR | POLLHUP | POLLRDHUP)){//disconnection event
                std::cout<<"Removing: "<<(*p).fd<<std::endl;
                this->remove_conn((*p).fd);
                p = to_watch.erase(p);   
            }else if((*p).revents & (POLLIN)){
                //execute user function. Note that is only going to be executed when the client has sent some message
                //identify the client that is ready to perform operation
                try{
                    Socket& s = *(this->conn_pool.at(p->fd));
                    this->op(this,s);
                }catch(std::exception e){
                    std::cout<<e.what()<<std::endl;
                    return;
                }
                p++;
            }
            else{
                p++;
            }
        }

        

    }
}

void Server::stop(){
    this->running = false;
}