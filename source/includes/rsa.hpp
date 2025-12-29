#include <cstdint>
#include <random>

const uint64_t bases[] = {2ULL,7ULL,61ULL};

struct gcdinfo{
    __int128_t x = 0;
    __int128_t y = 0;
    uint64_t gcd = 0ULL;
};

uint64_t fast_mod_exp(uint64_t, uint64_t, uint64_t);
bool isPrime(uint64_t n);
uint64_t gcd(uint64_t a, uint64_t b);
gcdinfo extended_euclidean_algorithm(uint64_t a, uint64_t b);

class RSA_Container{
private:
    std::random_device rd;
    std::mt19937_64 gen;
    std::uniform_int_distribution<uint32_t> keyspace;
    uint32_t generatePrime();
public:
    uint64_t n;
    uint32_t p;
    uint32_t q;
    uint64_t d;
    uint64_t e;
    bool hasValidKeys = false;
    static std::string encrypt(uint64_t, u_int64_t , std::string );
    static std::string decrypt(uint64_t, u_int64_t , std::string );
    RSA_Container();
    RSA_Container(const RSA_Container &r);
    void generateKeys();
    void invalidateKeys();
};