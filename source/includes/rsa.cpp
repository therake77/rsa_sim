#include <rsa.hpp>
#include <iostream>
#include <exception>

//note: it will be removed in future (uint128 is only used as intermediate objects in arithmetic operations)
std::ostream& operator<<(std::ostream& os, __int128 value) {
    if (value == 0) {
        os << "0";
        return os;
    }

    bool neg = false;
    if (value < 0) {
        neg = true;
        value = -value;
    }

    char buffer[50];
    int i = 0;

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (neg)
        os << '-';

    while (i--)
        os << buffer[i];

    return os;
}

RSA_Container::RSA_Container(const RSA_Container &r){
    this->gen = r.gen;
    this->keyspace = r.keyspace;
    this->n = r.n;
    this->p = r.p;
    this->q = r.q;
    this->d = r.d;
    this->e = r.e;
    this->hasValidKeys = r.hasValidKeys;
}

uint64_t fast_mod_exp(uint64_t base, uint64_t exp, uint64_t n){
    if(exp == 0){
        return 1;
    }
    uint64_t result = 1;
    while(exp > 1){
        if (exp & 1){
            result = ((__uint128_t)(result * base) % n);
            exp -= 1;
        }
        base = (__uint128_t)(base * base) % n;
        exp >>= 1;
    }
    return ((__uint128_t) base * result) % n;
}

uint64_t* remove_two_powers(uint64_t n){
    uint64_t* sd = new uint64_t[2];
    sd[0] = 0LL;
    sd[1] = 0LL; 
    while(!n & 1){
        n >>= 1;
        sd[0]+=1;
    }
    sd[1] = n;
    return sd;
}

bool millerTest(uint64_t a,uint64_t n, uint64_t d, uint64_t s){
    uint64_t x = fast_mod_exp(a,d,n);
    if(x == 1 || x == n-1){
        return true;
    }
    for(uint64_t i = 0 ; i < s; i++){
        x = fast_mod_exp(x,2,n);
        if(x == n-1){
            return true;
        }
    }
    return false;
}

bool isPrime(uint64_t n){
    if(!n & 1){
        return false;
    }
    
    uint64_t* sd = remove_two_powers(n-1);
    uint64_t s = sd[0];
    uint64_t d = sd[1];
    for(uint64_t base : bases){
        if(!millerTest(base,n,d,s)){
            return false;
        }
    }
    delete[] sd;
    return true;
}



uint64_t gcd(uint64_t a, uint64_t b){
    if(a < b){
        uint64_t temp = a;
        b = a;
        a = temp;
    }
    while(b != 0){
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

gcdinfo extended_euclidean_algorithm(uint64_t a, uint64_t b){
    gcdinfo i;
    if(b == 0){
        return {1,0,a};
    }else{
        gcdinfo temp = extended_euclidean_algorithm(b,a%b);
        i.x = temp.y;
        i.y = temp.x - (__int128_t)(a / b)*temp.y;
        i.gcd = temp.gcd;
        return i;
    }
}

uint64_t inverse_mod(uint64_t a, uint64_t n){
    gcdinfo g = extended_euclidean_algorithm(a,n);
    if(g.gcd == 1){
        return g.x % n;
    }
    return 0ULL;
}

RSA_Container::RSA_Container(){
    gen = std::mt19937_64(rd());
    keyspace = std::uniform_int_distribution<uint32_t>(0x80000000u,0xFFFFFFFFu);
}

uint32_t RSA_Container::generatePrime(){
    while (true)
    {
        uint32_t prime = keyspace(gen);
        if(isPrime(prime)){
            return prime;
        }
    }
}

void RSA_Container::generateKeys(){
    //first, generate the 
    this->p = generatePrime();
    this->q = generatePrime();
    this-> n = ((uint64_t)p*(uint64_t)q);
    uint64_t totient = (uint64_t)(p-1) * (uint64_t)(q-1);

    //generate e
    uint64_t temp;
    std::uniform_int_distribution<uint64_t> e_space(2,totient-1);
    std::cout<<"generated totient: "<<totient<<std::endl;

    do{
        temp = e_space(gen);
    }while(gcd(totient,temp) != 1ULL);
    this->e = temp;
    std::cout<<"generated e: "<<temp<<std::endl;

    //generate d
    this->d = inverse_mod(e,totient);
    std::cout<<"generated d: "<<this->d<<std::endl;

    if((uint64_t)(((__uint128_t)e*(__uint128_t)d)%(__uint128_t)totient)!=1ULL){
        throw std::exception();
    }

    this->hasValidKeys = true;
    return;
}

void RSA_Container::invalidateKeys(){
    this->hasValidKeys = false;
}

std::string RSA_Container::encript(uint64_t public_key,std::string msg){
    return std::string("");
}