#ifndef EIGEN_TENSOR_INTERFACE_H
#define EIGEN_TENSOR_INTERFACE_H
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
/*
Hamiltonian:

1 2
| |
OOO
| |
3 4
*/

// 使用plugin并载入Eigen，注意Eigen内部实际上也是做了一些变化的
#define EIGEN_TENSOR_PLUGIN "eigen_tensor_plugin.h"
#define EIGEN_USE_MKL_ALL
//#define EIGEN_USE_MKL_VML
//#define EIGEN_USE_LAPACKE
//#define EIGEN_USE_BLAS
#include <Eigen/CXX11/Tensor>

// contraction
#define find_in(it, pool) std::find((pool).begin(), (pool).end(), it)
#define not_found(it, pool) find_in(it, pool) == (pool).end()
#define get_index(it, pool) std::distance((pool).begin(), find_in(it, pool))

template<typename TensorType1, typename TensorType2, std::size_t ContractNum>
EIGEN_DEVICE_FUNC const Eigen::TensorContractionOp<const Eigen::array<Eigen::IndexPair<typename Eigen::internal::traits<TensorType1>::Index>, ContractNum>,
                                                   const TensorType1,
                                                   const TensorType2,
                                                   const Eigen::NoOpOutputKernel>
node_contract(const TensorType1 tensor1,
              const TensorType2 tensor2,
              const Eigen::array<Leg, ContractNum>& leg1,
              const Eigen::array<Leg, ContractNum>& leg2,
              std::map<Leg,Leg> map1=std::map<Leg,Leg>{},
              std::map<Leg,Leg> map2=std::map<Leg,Leg>{}) {
  Eigen::array<Eigen::IndexPair<typename Eigen::internal::traits<TensorType1>::Index>, ContractNum> dims;
  for(auto i=0; i<ContractNum; i++){
    dims[i].first = get_index(leg1[i], tensor1.leg_info);
  }
  for(auto i=0; i<ContractNum; i++){
    dims[i].second = get_index(leg2[i], tensor2.leg_info);
  }
  auto res = tensor1.contract(tensor2, dims);
  auto i=0;
  #define check_in_and_map(it, legs, map) {\
    if(not_found(it, legs)){\
      auto leg = map.find(it);\
      if(leg==map.end()){\
        res.leg_info[i++] = it;\
      }else{\
        res.leg_info[i++] = leg->second;\
      }\
    }\
  }
  for(auto j=0;j<tensor1.leg_info.size();j++){
    check_in_and_map(tensor1.leg_info[j], leg1, map1);
  }
  for(auto j=0;j<tensor2.leg_info.size();j++){
    check_in_and_map(tensor2.leg_info[j], leg2, map2);
  }
  #undef check_in_and_map
  return res;
}

#undef get_index
#undef not_found
#undef find_in
// svd and qr
/*#define find_in(it, pool) std::find((pool).begin(), (pool).end(), it)
template<typename TensorType, std::size_t SplitNum>
EIGEN_DEVICE_FUNC void node_svd(const TensorType& tensor, const Eigen::array<Leg, SplitNum>& legs) {
  typedef Eigen::internal::traits<TensorType> Traits;
  typedef typename Traits::Index Index;
  static const int Rank = Traits::NumDimensions;
  //first get the shape and analysis
  //auto whole_legs = tensor.dimensions();
  std::size_t left_size=1, right_size=1;
  std::size_t left_index = 0, right_index = SplitNum;
  for(int i=0;i<Rank;i++){
    auto index = find_in(tensor.leg_info[i], legs);
    if(index==legs.end()){
      right_size *= tensor.dimension(i);
      new_shape;
    }
  }
  auto left_index = tensor.get_index_from_leg(legs);
  Eigen::array<Index, Traits::NumDimensions-SplitNum> right_index;
  int head = 0;
  for(Index i=0;i<Traits::NumDimensions;i++){
    if(find_in(i, left_index)==left_index.end()){
      right_index[head++] = i;
    }
  }
}
#undef find_in
*/

// check Tensor的一个macro
template<typename TensorType>
void __debug_tensor(const TensorType& x, const char* name, std::ostream& os){
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