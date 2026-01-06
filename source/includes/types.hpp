
struct Erasable{
    virtual ~Erasable() = default;
};

template<typename T> 
class Value: Erasable{
    T value;
public:
    explicit Value(T v);
};