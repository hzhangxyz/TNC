#ifndef EIGEN_TENSOR_INTERFACE_H
#define EIGEN_TENSOR_INTERFACE_H
#include <iostream>
#include <array>
#include <vector>
#include <map>
#include <string>

/* 初始化Leg，并重载ostream */
// 定义Leg， 5遍5个脚
enum Leg {Left,Right,Up,Down,Phy,Left1,Right1,Up1,Down1,Phy1,Left2,Right2,Up2,Down2,Phy2,Left3,Right3,Up3,Down3,Phy3,Left4,Right4,Up4,Down4,Phy4};
// ostream重载，使用一个static map来完成
std::ostream& operator<<(std::ostream& out, const Leg value){
  static std::map<Leg, std::string> strings;
  if (strings.size() == 0){
#define IncEnum(p) strings[p] = #p
// 5x5的macro repeat
#define IncGroup(x) IncEnum(Left##x);IncEnum(Right##x);IncEnum(Up##x);IncEnum(Down##x);IncEnum(Phy##x);
IncGroup() IncGroup(1) IncGroup(2) IncGroup(3) IncGroup(4)
#undef IncGroup
#undef IncEnum
  }
// 好可以输出了
  return out << strings[value];
}

/* 设置默认Leg */
// 使用一个Trait来实现，TensorBase内部使用leg_info = DefaultLeg<DerivedTraits::NumDimensions>::value;来调用这里
template<std::size_t rank>
struct DefaultLeg{
  static const std::array<Leg, rank> value;
};
// 如果是4 rank，那大概是哈密顿量，设置一下leg
template <>
const std::array<Leg, 4> DefaultLeg<4>::value = {Phy1,Phy2,Phy3,Phy4};
// 不然的话那就算了
template <std::size_t other>
const std::array<Leg, other> DefaultLeg<other>::value = {};
/*
Hamiltonian的4个脚长这个样子
1 2
| |
OOO
| |
3 4
*/

/* 使用plugin并载入Eigen，注意Eigen内部实际上也是做了一些变化的 */
#define EIGEN_TENSOR_PLUGIN "eigen_tensor_plugin.h"
// 可能需要mkl blas那些东西
#define EIGEN_USE_MKL_ALL
//#define EIGEN_USE_MKL_VML
//#define EIGEN_USE_LAPACKE
//#define EIGEN_USE_BLAS
#include <Eigen/Dense>
#include <Eigen/CXX11/Tensor>

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

/* contraction */
//定义三个index用的macro，这样方便，写函数的话vector和map的格式都不一样，这里类型检查看起来也没比宏强多少
#define find_in(it, pool) std::find((pool).begin(), (pool).end(), it)
#define not_found(it, pool) find_in(it, pool) == (pool).end()
#define get_index(it, pool) std::distance((pool).begin(), find_in(it, pool))

// 好，这是contract，第一个参数是缩并脚标的类型，index类型使用了第一个tensor的trait
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
  // 构造给eigen用的缩并脚标对
  typedef Eigen::internal::traits<TensorType1> Traits;
  typedef typename Traits::Index Index;
  Eigen::array<Eigen::IndexPair<Index>, ContractNum> dims;
  for(auto i=0; i<ContractNum; i++){
    dims[i].first = get_index(leg1[i], tensor1.leg_info);
  }
  for(auto i=0; i<ContractNum; i++){
    dims[i].second = get_index(leg2[i], tensor2.leg_info);
  }
  // 然后运行，注意这里的auto返回的是一个op，是lazy的
  auto res = tensor1.contract(tensor2, dims);
  auto i=0;
  // 根据是否在leg内，是否map，来更新result的leg info, 两部分一样，所以写成个宏
  #define check_in_and_map(it, leg, map) {\
    if(not_found(it, leg)){\
      auto l = map.find(it);\
      if(l==map.end()){\
        res.leg_info[i++] = it;\
      }else{\
        res.leg_info[i++] = l->second;\
      }\
    }\
  }
  // 对于两个tensor之前的每个leg， 不在legs里则加入res，但如果还在map里需要先map
  for(auto j=0;j<tensor1.leg_info.size();j++){
    check_in_and_map(tensor1.leg_info[j], leg1, map1);
  }
  for(auto j=0;j<tensor2.leg_info.size();j++){
    check_in_and_map(tensor2.leg_info[j], leg2, map2);
  }
  #undef check_in_and_map
  return res;
}

/* svd */

template<typename TensorType, std::size_t SplitNum>
EIGEN_DEVICE_FUNC Eigen::JacobiSVD<Eigen::Matrix<typename Eigen::internal::traits<TensorType>::Scalar,
                                                 Eigen::Dynamic,
                                                 Eigen::Dynamic>, Eigen::HouseholderQRPreconditioner>
node_svd(const TensorType& tensor, const Eigen::array<Leg, SplitNum>& legs) {
  typedef Eigen::internal::traits<TensorType> Traits;
  typedef typename Traits::Index Index;
  typedef typename Traits::Scalar Scalar;
  static const int Rank = Traits::NumDimensions;
  static const int LeftRank = SplitNum;
  static const int RightRank = Rank - SplitNum;
  // 首先按照leg，分别把整个tensor的legs分左右两边
  Index left_size=1, right_size=1;
  Index left_index = 0, right_index = SplitNum;
  Eigen::array<Index, Rank> to_shuffle;
  for(int i=0;i<Rank;i++){
    if(not_found(tensor.leg_info[i], legs)){
      // 放右边
      right_size *= tensor.dimension(i);
      to_shuffle[right_index++] = i;
    }else{
      // 放左边
      left_size *= tensor.dimension(i);
      to_shuffle[left_index++] = i;
    }
  }
  auto shuffled = tensor.shuffle(to_shuffle);
  Eigen::Tensor<Scalar, 2> reshaped = shuffled.reshape(Eigen::array<Index, 2>{left_size, right_size});
  // shuffle并reshape，当作matrix然后就可以svd了
  typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> MatrixS;
  Eigen::Map<MatrixS> matrix(reshaped.data(), left_size, right_size);
  //debug_tensor(reshaped);
  std::cout << matrix << std::endl;
  return Eigen::JacobiSVD<MatrixS, Eigen::HouseholderQRPreconditioner> (matrix, Eigen::ComputeThinU | Eigen::ComputeThinV);
}

#undef get_index
#undef not_found
#undef find_in


#endif