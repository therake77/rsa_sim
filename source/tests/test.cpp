#include <rsa.hpp>
#include <iostream>

int main(){
    RSA_Factory r = RSA_Factory();
    r.generateKeys();
    std::cout<<"n: "<<r.n<<" e,d: "<<r.d<<" "<<r.e<<std::endl;
    //gcdinfo result = extended_euclidean_algorithm(39,8);
}