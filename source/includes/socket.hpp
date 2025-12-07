#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

class Socket{
protected:
    const std::string addr;
    int socket_fd;
    struct sockaddr_un socket_addr;
    bool start();
public:
    Socket(std::string);
    static std::string read(Socket);
    static void write(Socket,std::string);
    bool operator=(const Socket& s) const{
        return this->socket_fd == s.socket_fd;
    }
};

class ClientSocket : public Socket{
public:
    ClientSocket(const std::string);
    ClientSocket(int , sockaddr_un);
    bool connectTo(std::string);
    int getid();
};

class ServerSocket : public Socket{
public:
    ServerSocket(const std::string);
    bool waitForConnections(int);
    void setOperation(void (*)(ServerSocket *));
    void up();
    int getid();
    std::vector<ClientSocket> getCurrentClients();
private:
    void (*f)(ServerSocket *);
    std::vector<ClientSocket> conn_pool;
    bool active = false;
    void addConnection(ClientSocket);
    void removeConnection(int);
};   