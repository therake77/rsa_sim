#include <rsa.hpp>
#include <iostream>
#include <exception>
#include <cmd.hpp>

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
    uint64_t result = 1ULL;
    while(exp > 1){
        if (exp & 1){
            result = (uint64_t)((__uint128_t)((__uint128_t)result * (__uint128_t)base) % (__uint128_t)n);
            exp -= 1;
        }
        base = (uint64_t)(((__uint128_t)base * (__uint128_t)base) % (__uint128_t)n);
        exp >>= 1;
    }
    return (uint64_t)(((__uint128_t) base * (__uint128_t)result) % (__uint128_t)n);
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
    if(x == 1ULL || x == n-1){
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
        i.y = temp.x - ((__int128_t)(a / b)*(__int128_t)temp.y);
        i.gcd = temp.gcd;
        return i;
    }
}

uint64_t inverse_mod(uint64_t a, uint64_t n){
    gcdinfo g = extended_euclidean_algorithm(a,n);
    if(g.gcd == 1){
        int64_t x = (int64_t)((__int128_t)g.x % (__int128_t)n);
        if(x < 0){
            x+=n;
        }
        return (uint64_t) x;
    }
    return 0ULL;
}

std::string RSA_Container::encrypt(uint64_t mod,uint64_t public_exp,std::string msg){
    std::vector<std::string> blocks = toblocks(msg,7);

    for(int i = 0; i < blocks.size(); i++){
        std::cout<<"Block: "<<blocks[i]<<std::endl;
        uint64_t umsg = stouint64(blocks[i]);
        std::cout<<"Block number: "<<umsg<<std::endl;
        uint64_t umsge = fast_mod_exp(umsg,public_exp,mod);
        std::cout<<"Encrypted block number: "<<umsge<<std::endl;
        blocks[i] = uint64tos(umsge);
        std::cout<<"Encrypted block before padding: ";
        c_print(blocks[i]);
        blocks[i].resize(8,'/0');
        std::cout<<"Encrypted block after padding: ";
        c_print(blocks[i]);
    }

    std::string encrypted_msg;
    encrypted_msg.reserve(blocks.size()*7);
    for(int i = 0 ; i < blocks.size(); i++){
        encrypted_msg.append(blocks[i]);
    }
    return encrypted_msg;
}

std::string RSA_Container::decrypt(uint64_t mod,uint64_t private_exp,std::string msg){
    //first, transform the messagess to blocks
    std::vector<std::string> blocks = toblocks(msg,8);
    for(int i = 0; i < blocks.size(); i++){
        c_print(blocks[i]);
        uint64_t umsg = stouint64(blocks[i]);
        std::cout<<"Block number: "<<umsg<<std::endl;
        uint64_t umsg_d = fast_mod_exp(umsg,private_exp,mod);
        std::cout<<"Decrypted block number: "<<umsg_d<<std::endl;
        blocks[i] = uint64tos(umsg_d);

    }
    std::string decrypted_msg;
    decrypted_msg.reserve(blocks.size()*7);
    for(int i = 0 ; i < blocks.size(); i++){
        decrypted_msg.append(blocks[i]);
    }
    return decrypted_msg;
}

RSA_Container::RSA_Container()
{
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
    std::cout<<"p : "<<p<<" q: "<<q<<" n: "<<n<<std::endl;
    uint64_t totient = (uint64_t)(((uint64_t)p-1ULL) *((uint64_t)q-1ULL));
    std::cout<<"generated totient: "<<totient<<std::endl;
    //generate e
    uint64_t temp;
    std::uniform_int_distribution<uint64_t> e_space(2,totient-1);
    
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


