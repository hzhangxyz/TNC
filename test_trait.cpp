#include <iostream>
#include <array>

template <std::size_t rank>
struct DefaultLeg{
  static const std::array<int, rank> value;
};

template <>
const std::array<int, 2> DefaultLeg<2>::value = {1,2};

template <>
const std::array<int, 1> DefaultLeg<1>::value = {3};

/////////
template <class T>
struct MyTrait{
  static const int N;

};

template <>
const int MyTrait<char>::N = 2;

template <>
const int MyTrait<int>::N = 1;

///////
template <class T>
struct MyClass{
  std::array<int, MyTrait<T>::N> p = DefaultLeg<MyTrait<T>::N>::value;
};


int main(){
  std::cout << MyTrait<int>::N << MyTrait<char>::N <<  "\n";
  MyClass<int> a;
  std::cout << a.p[0] << "\n";
  MyClass<char> b;
  std::cout << b.p[0] << "\n";
}
