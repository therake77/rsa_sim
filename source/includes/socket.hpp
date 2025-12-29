#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#include <unordered_map>
#include <memory>

class Socket{
private:
    const std::string addr;
    int socket_fd;
    struct sockaddr_un socket_addr;
    bool start();
public:
    Socket(std::string);
    Socket(int, struct sockaddr_un);
    ~Socket();
    static std::string read(const Socket&);
    static void write(const Socket&,std::string);
    int get_fd();
    bool operator==(const Socket&);
    
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    bool connect_to(std::string addr);
    bool sock_listen(int);
};

class Agent{
protected:    
    Socket sock;
    std::unordered_map<std::string,std::shared_ptr<void>>customObjects;
public:    
    Agent(std::string);
    int get_fd();
    const Socket& get_sock() const;
    template<typename T>
    
    std::shared_ptr<T> get_object(std::string key){
        auto obj = this->customObjects.find(key);
        if(obj == this->customObjects.end()){
            return {};
        } 
        return std::static_pointer_cast<T>(obj->second);
    }

    template<typename T>
    void attach_object(std::string obj_key, std::shared_ptr<T> obj_ptr){
        this->customObjects.insert({obj_key,obj_ptr});
        return;
    }
};

class Client : public Agent{
public:
    Client(std::string);
    bool connect_to(std::string);
};

class Server : public Agent{
private:
    void (*op)(Server *,const Socket&);
    std::unordered_map<int,std::unique_ptr<Socket>> conn_pool;
    bool running = false;
    bool add_conn(int,sockaddr_un);
    bool remove_conn(int);

public:
    Server(std::string);
    bool wait_for_comms(int);
    void set_operation(void (*)(Server*,const Socket&));
    void up();
    void stop();
};
