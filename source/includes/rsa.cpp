#include <rsa.hpp>
#include <iostream>
#include <exception>
#include <cmd.hpp>

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
        if(x == n-1ULL){
            return true;
        }
        if(x == 1ULL){
            return false;
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
        blocks[i] = uint64tos(fast_mod_exp(stouint64(blocks[i]),public_exp,mod));
        blocks[i].resize(8,'/0');
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
        blocks[i] = uint64tos(fast_mod_exp(stouint64(blocks[i]),private_exp,mod));
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
    try{
        this->generateKeys();
    }catch(std::exception e){
        std::cout<<"Error generating keys\n";
        throw e;
    }
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
    uint64_t totient = (uint64_t)(((uint64_t)p-1ULL) *((uint64_t)q-1ULL));
    //generate e
    uint64_t temp;
    std::uniform_int_distribution<uint64_t> e_space(2,totient-1);
    
    do{
        temp = e_space(gen);
    }while(gcd(totient,temp) != 1ULL);
    this->e = temp;

    //generate d
    this->d = inverse_mod(e,totient);

    if((uint64_t)(((__uint128_t)e*(__uint128_t)d)%(__uint128_t)totient)!=1ULL){
        throw std::exception();
    }

    this->hasValidKeys = true;
    return;
}

void RSA_Container::invalidateKeys(){
    this->hasValidKeys = false;
}

uint64_t pollard_rho_fact(uint64_t n,uint64_t x_0, uint64_t c){
    auto f = [n,c](const uint64_t u){
        return (uint64_t)((fast_mod_exp(u,2,n) + c) % n);
    };

    uint64_t d = 1ULL;
    uint64_t x = x_0,y = x;
    while(d == 1){
        x = f(x);
        y = f(f(y));
        if(x > y){
            d = gcd(x-y,n);
        }else{
            d = gcd(y-x,n);
        }
    }
    if(d == n){
        return 0;
    }
    return d;
}

uint64_t fact_n(uint64_t n){
    if(n ==0 || n == 1) return 1;
    bool control = true;
    
    std::uniform_int_distribution<uint32_t> x_0_space = std::uniform_int_distribution<uint32_t>(3,n-2);
    std::uniform_int_distribution<uint32_t> c_space = std::uniform_int_distribution<uint32_t>(2,n-1);
    std::random_device gen;
    size_t iter_num = 0;
    uint64_t x_0 = 2, c = 1;
    uint64_t p;
    while(control){
        p = pollard_rho_fact(n,x_0,c);
        iter_num++;
        if(p != 0 || iter_num == (0x01<<16)){
            control = false;
        }
        x_0 = x_0_space(gen);
        c = c_space(gen);
        
    }
    return p;
}