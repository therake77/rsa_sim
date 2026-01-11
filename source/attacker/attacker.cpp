#include <iostream>
#include <socket.hpp>
#include <rsa.hpp>
#include <cmd.hpp>
#include <poll.h>

void welcome(Server* s, const Socket& peer){
    uint64_t n = s->get_object<uint64_t>("n");
    uint64_t e = s->get_object<uint64_t>("e");
    Socket::write(peer,std::to_string(n)+" "+std::to_string(e));
}

void operation(Server* s, const Socket& peer){
    //get our important constants:
    uint64_t n = s->get_object<uint64_t>("n");
    uint64_t e = s->get_object<uint64_t>("e");
    uint64_t d = s->get_object<uint64_t>("d");
    Client& attacker = *(s->get_object<std::shared_ptr<Client>>("conn_entity"));
    //read the encripted message:
    std::string encrypted_msg = Socket::read(peer);
    //send it to the server
    Socket::write(attacker.get_sock(),encrypted_msg);
    //and also decrypt it here:
    std::string decrypted_msg = RSA_Container::decrypt(n,d,encrypted_msg);
    //print it
    std::cout<<"Message hacked: \n";
    std::cout<<decrypted_msg<<std::endl;
    //now wait for the server response
    pollfd stat = {attacker.get_fd(),POLLIN,0};
    poll(&stat,1,-1);
    std::string response = Socket::read(attacker.get_sock());
    std::cout<<"Response received:"<<response<<std::endl;
    Socket::write(peer,response);
}

int main(){
    std::string addr,public_addr;
    std::string to_attack_addr;
    //the attacker needs a direction
    std::cout<<"\'Personal\' address: ";
    std::cin>>addr;
    std::cout<<"\'Public\' address: ";
    std::cin>>public_addr;
    std::cout<<"Address to attack: ";
    std::cin>>to_attack_addr;

    Client attacker = Client(addr);
    Server false_server = Server(public_addr);

    //FIRST: Connect to the real server to get its keys
    attacker.connect_to(to_attack_addr);
    //At connection, we receive the keys. Extract them
    pollfd stat = {attacker.get_fd(),POLLIN,0};
    uint64_t n;
    uint64_t e;
    poll(&stat,1,-1);
    parse_keystring(Socket::read(attacker.get_sock()),n,e);
    //now we have the keys. Show them
    std::cout<<"Received keys: n: "<<n<<", e: "<<e<<std::endl;
    //obtain the private key via pollard factorization
    uint64_t p,q,d;
    p = fact_n(n);
    //know obtain the rest of the keys
    q = n/p;
    d = inverse_mod(e,(p-1)*(q-1));
    std::cout<<"Attack performed:\np: "<<p<<" q: "<<q<<" d: "<<d<<std::endl;
    

//From here, we have already the keys. Now our job is to "sustitute" the original server
    false_server.attach_object("n",std::make_unique<std::any>(n));
    false_server.attach_object("e",std::make_unique<std::any>(e));
    false_server.attach_object("d",std::make_unique<std::any>(d));
    false_server.attach_object("conn_entity",std::make_unique<std::any>(
            std::make_shared<Client>(std::move(attacker))
        )
    );

    false_server.set_welcome_op(&welcome);
    false_server.set_operation(&operation);
    false_server.wait_for_comms(5);
    
    false_server.up();
}