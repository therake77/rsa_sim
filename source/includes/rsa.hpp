
typedef unsigned long int ksize_t;


class PrimeGenerator{
    PrimeGenerator();
public:
    ksize_t generatePrime(){

    }
};

class RSA_Factory{
private:

    ksize_t n;
    ksize_t p;
    ksize_t q;
public:
    RSA_Factory();
    void generateKeys();

};