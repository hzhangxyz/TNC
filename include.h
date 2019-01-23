#ifndef INCLUDE_H
#define INCLUDE_H
#include <iostream>
#include <array>
#include <vector>
#include <map>
#include <string>

// 初始化Leg，并重载ostream
enum Leg {Left,Right,Up,Down,Phy,Left1,Right1,Up1,Down1,Phy1,Left2,Right2,Up2,Down2,Phy2,Left3,Right3,Up3,Down3,Phy3,Left4,Right4,Up4,Down4,Phy4};
std::ostream& operator<<(std::ostream& out, const Leg value){
  static std::map<Leg, std::string> strings;
  if (strings.size() == 0){
#define IncEnum(p) strings[p] = #p
#define IncGroup(x) IncEnum(Left##x);IncEnum(Right##x);IncEnum(Up##x);IncEnum(Down##x);IncEnum(Phy##x);
IncGroup() IncGroup(1) IncGroup(2) IncGroup(3) IncGroup(4)
#undef IncGroup
#undef IncEnum
  }
  return out << strings[value];
}

// 设置默认Leg
template<std::size_t rank>
struct DefaultLeg{
    static const std::array<Leg, rank> value;
};
template <>
const std::array<Leg, 4> DefaultLeg<4>::value = {Phy1,Phy2,Phy3,Phy4};
template <std::size_t other>
const std::array<Leg, other> DefaultLeg<other>::value = {};

// 使用plugin并载入Eigen，注意Eigen内部实际上也是做了一些变化的
#define EIGEN_TENSOR_PLUGIN "eigen_tensor_plugin.h"
#define EIGEN_USE_MKL_ALL
//#define EIGEN_USE_MKL_VML
//#define EIGEN_USE_LAPACKE
//#define EIGEN_USE_BLAS
#include <Eigen/CXX11/Tensor>

// check Tensor的一个macro
template<class SomeTensor>
void __debug_tensor(const SomeTensor& x, const char* name, std::ostream& os){
    os << " " << name << "= { rank=" << x.NumDimensions << " dims=[";
    for(auto i=0;i<x.NumDimensions;i++){
        os << "(" << x.dimension(i) << "|" << x.leg_info[i] << "), ";
    }
    os << "], size=" << x.size();
    if(x.size()<500){os << ", data=\n" << x << " }\n";}
    else{os << "}";}
}
#define debug_tensor(x) __debug_tensor(x, #x, std::clog)

#endif