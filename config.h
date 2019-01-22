#ifndef CONFIG_H
#define CONFIG_H
#include <iostream>
#include <vector>
#include <map>
#include <string>

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

/*template<std::size_t n>
std::array<Leg, n>default_leg_info(){
    if(n==4){
        return std::array<Leg, n> {Phy1, Phy2, Phy3, Phy4};
    }else{
        return std::array<Leg, n> {};
    }
}*/
// cpp的类型推演不够强的问题？
#define EIGEN_TENSOR_PLUGIN "eigen_tensor_plugin.h"
#include <Eigen/CXX11/Tensor>

/*template<typename Der, typename Acc>
Eigen::array<Leg, Eigen::internal::traits<Der>::NumDimensions>
TensorBase<Der, Acc>::leg_info*/

template<typename S, int N, int O, typename I>
void __debug_tensor(Eigen::Tensor<S, N, O, I>& t, const char* s) {
    std::clog << " " << s << "= { rank=" << t.NumDimensions << " dims=[";
    for(auto i=0;i<t.NumDimensions;i++){
        std::clog << "(" << t.dimension(i) << "|" << t.leg_info[i] << "), ";
    }
    std::clog << "]";
    if(t.size()<500){std::clog << "\n" << t << " }\n";}
    else{std::clog << "}";}
}
#define debug_tensor(x) __debug_tensor(x, #x)
#endif