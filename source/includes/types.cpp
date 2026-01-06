#include <types.hpp>

template<typename T>
Value<T>::Value(T v): value(std::move(v)){}