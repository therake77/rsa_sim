#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#include <unordered_map>
#include <memory>

class Socket{
protected:
    const std::string addr;
    int socket_fd;
    struct sockaddr_un socket_addr;
    bool start();
public:
    /*Constructor of Socket class. Expects an address for UNIX socket*/
    Socket(std::string);

    /*Allows to read from an specified Socket in a safe way. 
    Expects the null character to work as delimiter between messages*/
    static std::string read(Socket);

    /*
    Allows writing directly over an specified Socket. 
    We strongly recommend to null-terminate your message, or the program would stall
    */
    static void write(Socket,std::string);

    /*
    Equal sockets are the ones with the same file descriptor
    */
    bool operator=(const Socket& s) const{
        return this->socket_fd == s.socket_fd;
    }
    ~Socket();
};

class ClientSocket : public Socket{
public:
    /*
    Creates a socket with address specified
    */
    ClientSocket(const std::string);
    /*allows creating a socket object to manipulate an existing socket*/
    ClientSocket(int , sockaddr_un);

    /*Set a connection to another socket, specified by address*/
    bool connectTo(std::string);
    /*get the file descriptor of the socket*/
    int getid();
};

class ServerSocket : public Socket{
public:
    ServerSocket(const std::string);

    /*Prepares the socket to start listening to new connections in a non-blocking mode*/
    bool waitForConnections(int);

    /*Sets an user-specified operation that the socket would execute at the main loop.
    A pointer to the current socket would be pass as the argument. 
    Be aware that before this function is called, the socket would clear all connections that were previously closed.
    */
    void setOperation(void (*)(ServerSocket *));
    
    /*Starts the main loop in a blocking way*/
    void up();
    /*Used to stop the main loop. We recommend to use this in your user-specified function*/
    void stop();
    /*Returns the file descriptor of the socket*/
    int getid();
    /*Get the current clients connected as ClientSocket objects*/
    std::vector<ClientSocket> getCurrentClients();

    /*Get a saved object from the pool of saved objects*/
    template<typename T>
    std::shared_ptr<T> getObject(std::string key){
        auto obj = this->customObjects.find(key);
        if(obj == this->customObjects.end()){
            return {};
        } 
        return std::static_pointer_cast<T>(obj->second);
    }
    
    /*Save a new object in the pool of custom objects*/
    template<typename T>
    void attachObject(std::string obj_key, std::shared_ptr<T> obj_ptr){
        this->customObjects.insert({obj_key,obj_ptr});
        return;
    }

private:
    void (*f)(ServerSocket *);
    std::vector<ClientSocket> conn_pool;
    std::unordered_map<std::string,std::shared_ptr<void>>customObjects;
    bool active = false;
    void addConnection(ClientSocket);
    void removeConnection(int);
};   