#ifndef EIGEN_NODE_HPP_
#define EIGEN_NODE_HPP_
#include <iostream>
#include <tuple>
#include <array>
#include <map>
#include <string>
#include <algorithm>

namespace Node
{
/* 初始化Leg，并重载ostream */
// 定义Leg， 5遍5个脚
enum class Leg
{
  #define CreateLeg(x) Left##x, Right##x, Up##x, Down##x, Phy##x
  CreateLeg(),
  CreateLeg(1),
  CreateLeg(2),
  CreateLeg(3),
  CreateLeg(4)
  #undef CreateLeg
};

// ostream重载，使用一个static map来完成
std::ostream& operator<<(std::ostream& out, const Leg& value)
{
  static std::map<Leg, std::string> strings;
  if (strings.size() == 0)
  {
    // 5x5的macro repeat
    #define IncEnum(p) strings[Leg::p] = #p
    #define IncGroup(x) IncEnum(Left##x); IncEnum(Right##x); IncEnum(Up##x); IncEnum(Down##x); IncEnum(Phy##x)
    IncGroup();
    IncGroup(1);
    IncGroup(2);
    IncGroup(3);
    IncGroup(4);
    #undef IncGroup
    #undef IncEnum
  }
  // 好可以输出了
  return out << strings[value];
}

/* 设置默认Leg */
// 使用一个Trait来实现，TensorBase内部使用leg_info = DefaultLeg<DerivedTraits::NumDimensions>::value;来调用这里
template<std::size_t rank>
struct DefaultLeg
{
  static const std::array<Leg, rank> value;
  // static const Leg value[rank];
};
// 如果是4 rank，那大概是哈密顿量，设置一下leg
template <>
const std::array<Leg, 4> DefaultLeg<4>::value = {Leg::Phy1, Leg::Phy2, Leg::Phy3, Leg::Phy4};
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
}  // namespace Node

/* 设置好Leg那些东西后,载入Eigen，注意Eigen内部实际上也是做了一些变化的 */
// 可能需要mkl blas那些东西
// #define EIGEN_USE_MKL_ALL
// #define EIGEN_USE_MKL_VML
#define EIGEN_USE_LAPACKE
#define EIGEN_USE_BLAS
#include <Eigen/Dense>
#include <Eigen/CXX11/Tensor>

namespace Node
{
// check Tensor的一个macro
template<typename TensorType>
void __debug_tensor(const TensorType& x, const char* name, std::ostream& os)
{
  os << " " << name << "= { rank=" << x.NumDimensions << " dims=[";
  for (auto i = 0; i < x.NumDimensions; i++)
  {
    os << "(" << x.dimension(i) << "|" << x.leg_info[i] << "), ";
  }
  os << "], size=" << x.size();
  if (x.size() < 500)
  {
    os << ", data=\n" << x << " }\n";
  }
  else
  {
    os << "}";
  }
}
#define debug_tensor(x) Node::__debug_tensor(x, #x, std::clog)

// leg name 是 Leg
// index的序号是 Index
// rank 是 std::size_t
// dimension, size 是 Index

/* contraction */
// 定义三个index用的macro，这样方便，写函数的话vector和map的格式都不一样，这里类型检查看起来也没比宏强多少
/* template<typename ContainerType, typename T>
inline auto find_in(const T& it, const ContainerType& pool)
{
  return std::find(pool.begin(), pool.end(), it);
} */

template<typename ContainerType, typename T>
inline typename ContainerType::const_iterator
__find_in(const T& it, const ContainerType& pool)
{
  return std::find(pool.begin(), pool.end(), it);
}
// #define find_in(it, pool) std::find((pool).begin(), (pool).end(), it)

template<typename ContainerType, typename T>
inline bool __not_found(const T& it, const ContainerType& pool)
{
  return __find_in(it, pool) == pool.end();
}
// #define not_found(it, pool) find_in(it, pool) == (pool).end()

template<typename ContainerType, typename T>
inline int __get_index(const T& it, const ContainerType& pool)
{
  return std::distance(pool.begin(), __find_in(it, pool));
}
// #define get_index(it, pool) std::distance((pool).begin(), find_in(it, pool))

// 好，这是contract，第一个参数是缩并脚标的类型，index类型使用了第一个tensor的trait
// 不返回op了,直接返回eval后的东西
// 注意contract num在最前面,为了partial deduction
template<std::size_t ContractNum, typename TensorType1, typename TensorType2>
EIGEN_DEVICE_FUNC Eigen::Tensor<
                    typename TensorType1::Scalar,
                    TensorType1::NumDimensions + TensorType2::NumDimensions -
                      2*ContractNum>
contract(const TensorType1& tensor1,
         const TensorType2& tensor2,
         const Eigen::array<Leg, ContractNum>& leg1,
         const Eigen::array<Leg, ContractNum>& leg2,
         const std::map<Leg, Leg>& map1 = {},
         const std::map<Leg, Leg>& map2 = {})
{
  // 构造给eigen用的缩并脚标对
  // typedef Eigen::internal::traits<TensorType1> Traits;
  typedef typename TensorType1::Index Index;
  Eigen::array<Eigen::IndexPair<Index>, ContractNum> dims;
  for (auto i = 0; i < ContractNum; i++)
  {
    dims[i].first = __get_index(leg1[i], tensor1.leg_info);
  }
  for (auto i =0; i < ContractNum; i++)
  {
    dims[i].second = __get_index(leg2[i], tensor2.leg_info);
  }
  // 然后运行，注意这里的auto返回的是一个op，是lazy的
  auto res = tensor1.contract(tensor2, dims);
  // 创建新的tensor的leg
  auto i = 0;
  // 根据是否在leg内，是否map，来更新result的leg info, 两部分一样，所以写成个宏
  #define check_in_and_map(it, leg, map) {\
    if (__not_found(it, leg))\
    {\
      auto l = map.find(it);\
      if (l == map.end())\
      {\
        res.leg_info[i++] = it;\
      }\
      else\
      {\
        res.leg_info[i++] = l->second;\
      }\
    }\
  }
  // 对于两个tensor之前的每个leg， 不在legs里则加入res，但如果还在map里需要先map
  for (auto j = 0; j < tensor1.leg_info.size(); j++)
  {
    check_in_and_map(tensor1.leg_info[j], leg1, map1);
  }
  for (auto j = 0; j < tensor2.leg_info.size(); j++)
  {
    check_in_and_map(tensor2.leg_info[j], leg2, map2);
  }
  #undef check_in_and_map
  return res;
}

template<typename T1, typename T2, typename T3>
class __svd_res : public std::tuple<T1, T2, T3>
{
 public:
  __svd_res(T1 t1, T2 t2, T3 t3) : std::tuple<T1, T2, T3>(t1, t2, t3) {}
  inline T1& U()
  {
    return std::get<0>(*this);
  }
  inline T2& S()
  {
    return std::get<1>(*this);
  }
  inline T3& V()
  {
    return std::get<2>(*this);
  }
};

/* svd */
// svd返回的是含有U,S,V的一个tuple
template<std::size_t SplitNum, typename TensorType>
EIGEN_DEVICE_FUNC __svd_res<
                    Eigen::Tensor<
                      typename TensorType::Scalar,
                      SplitNum+1>,
                    Eigen::Tensor<typename TensorType::Scalar, 1>,
                    Eigen::Tensor<
                      typename TensorType::Scalar,
                      TensorType::NumDimensions-SplitNum+1>>
svd(const TensorType& tensor,
    const Eigen::array<Leg, SplitNum>& legs,
    Leg new_leg1,
    Leg new_leg2,
    typename TensorType::Index cut = -1)
{
  // 先各种重命名烦人的东西
  // typedef Eigen::internal::traits<TensorType> Traits;
  typedef typename TensorType::Index Index;
  typedef typename TensorType::Scalar Scalar;
  static const std::size_t Rank = TensorType::NumDimensions;
  static const std::size_t LeftRank = SplitNum;
  static const std::size_t RightRank = Rank - SplitNum;
  typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> MatrixS;
  // 首先按照leg，分别把整个tensor的legs分左右两边
  Index left_size = 1, right_size = 1;
  Index left_index = 0, right_index = 0;
  Eigen::array<Index, Rank> to_shuffle;
  // 需要计算一下U, V矩阵的样子
  Eigen::array<Index, LeftRank + 1> left_new_shape;
  Eigen::array<Index, RightRank + 1> right_new_shape;
  Eigen::array<Leg, LeftRank + 1> left_new_leg;
  Eigen::array<Leg, RightRank + 1> right_new_leg;
  // 开始分了, 记录总size, shape和leg
  for (auto i = 0; i < Rank; i++)
  {
    if (__not_found(tensor.leg_info[i], legs))
    {
      // 放右边
      right_size *= tensor.dimension(i);
      right_new_shape[right_index] = tensor.dimension(i);
      right_new_leg[right_index] = tensor.leg_info[i];
      to_shuffle[SplitNum+right_index] = i;
      right_index++;
    }
    else
    {
      // 放左边
      left_size *= tensor.dimension(i);
      left_new_shape[left_index] = tensor.dimension(i);
      left_new_leg[left_index] = tensor.leg_info[i];
      to_shuffle[left_index] = i;
      left_index++;
    }
  }
  // 考虑中间的那个cut
  auto min_size = (left_size < right_size)?left_size:right_size;
  if ((cut != -1)&&(cut < min_size))
  {
    min_size = cut;
  }
  // 然后把最后一个脚加上去
  left_new_leg[left_index] = new_leg1;
  right_new_leg[right_index] = new_leg2;
  left_new_shape[left_index] = right_new_shape[right_index] = min_size;
  // shuffle并reshape，当作matrix然后就可以svd了
  auto shuffled = tensor.shuffle(to_shuffle);
  Eigen::Tensor<Scalar, 2> reshaped = shuffled.reshape(Eigen::array<Index, 2>{left_size, right_size});
  Eigen::Map<MatrixS> matrix(reshaped.data(), left_size, right_size);
  // Eigen::JacobiSVD<MatrixS, Eigen::HouseholderQRPreconditioner> svd(matrix, Eigen::ComputeThinU | Eigen::ComputeThinV);
  Eigen::BDCSVD<MatrixS> svd(matrix, Eigen::ComputeThinU | Eigen::ComputeThinV);
  // 再把矩阵变回tensor
  Eigen::Tensor<Scalar, LeftRank+1> U(left_new_shape);
  Eigen::Tensor<Scalar, 1> S(Eigen::array<Index, 1>{min_size});
  Eigen::Tensor<Scalar, RightRank+1> V(right_new_shape);
  // 注意,这里的截断使用了列优先的性质,需要截断的那个脚是在最后面的
  #define copy_data(src, dst) std::copy(src.data(), src.data()+dst.size(), dst.data())
  copy_data(svd.matrixU(), U);
  copy_data(svd.singularValues(), S);
  copy_data(svd.matrixV(), V);
  #undef copy_data
  U.leg_info = left_new_leg;
  if (new_leg1 == new_leg2)
  {
    S.leg_info = Eigen::array<Leg, 1>{new_leg1};
  }
  V.leg_info = right_new_leg;
  return __svd_res<Eigen::Tensor<Scalar, LeftRank+1>,
                    Eigen::Tensor<Scalar, 1>,
                    Eigen::Tensor<Scalar, RightRank+1>> {U, S, V};
}

template<typename T1, typename T2>
class __qr_res : public std::tuple<T1, T2>
{
 public:
  __qr_res(T1 t1, T2 t2) : std::tuple<T1, T2>(t1, t2) {}
  inline T1& Q()
  {
    return std::get<0>(*this);
  }
  inline T2& R()
  {
    return std::get<1>(*this);
  }
};

/* qr */
// qr外部和svd一样，里面需要处理一下
// http://www.netlib.org/lapack/explore-html/df/dc5/group__variants_g_ecomputational_ga3766ea903391b5cf9008132f7440ec7b.html
template<std::size_t SplitNum, typename TensorType>
EIGEN_DEVICE_FUNC __qr_res<
                    Eigen::Tensor<
                      typename TensorType::Scalar,
                      SplitNum+1>,
                    Eigen::Tensor<
                      typename TensorType::Scalar,
                      TensorType::NumDimensions-SplitNum+1>>
qr(const TensorType& tensor,
   const Eigen::array<Leg, SplitNum>& legs,
   Leg new_leg1,
   Leg new_leg2,
   bool computeR = true)
{
  // 先各种重命名烦人的东西
  // typedef Eigen::internal::traits<TensorType> Traits;
  typedef typename TensorType::Index Index;
  typedef typename TensorType::Scalar Scalar;
  static const std::size_t Rank = TensorType::NumDimensions;
  static const std::size_t LeftRank = SplitNum;
  static const std::size_t RightRank = Rank - SplitNum;
  typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> MatrixS;
  // 首先按照leg，分别把整个tensor的legs分左右两边
  Index left_size = 1, right_size = 1;
  Index left_index = 0, right_index = 0;
  Eigen::array<Index, Rank> to_shuffle;
  // 需要计算一下U, V矩阵的样子
  Eigen::array<Index, LeftRank + 1> left_new_shape;
  Eigen::array<Index, RightRank + 1> right_new_shape;
  Eigen::array<Leg, LeftRank + 1> left_new_leg;
  Eigen::array<Leg, RightRank + 1> right_new_leg;
  // 开始分了, 记录总size, shape和leg， 注意方向和svd不同
  for (auto i = 0; i < Rank; i++)
  {
    if (__not_found(tensor.leg_info[i], legs))
    {
      // 放右边
      right_size *= tensor.dimension(i);
      right_new_shape[right_index+1] = tensor.dimension(i);
      right_new_leg[right_index+1] = tensor.leg_info[i];
      to_shuffle[SplitNum+right_index] = i;
      right_index++;
    }
    else
    {
      // 放左边
      left_size *= tensor.dimension(i);
      left_new_shape[left_index] = tensor.dimension(i);
      left_new_leg[left_index] = tensor.leg_info[i];
      to_shuffle[left_index] = i;
      left_index++;
    }
  }
  // 考虑中间的那个cut
  auto min_size = (left_size < right_size)?left_size:right_size;
  // 然后把最后一个脚加上去
  left_new_leg[left_index] = new_leg1;
  right_new_leg[0] = new_leg2;
  left_new_shape[left_index] = right_new_shape[0] = min_size;
  // shuffle并reshape，当作matrix然后就可以分解了
  auto shuffled = tensor.shuffle(to_shuffle);
  Eigen::Tensor<Scalar, 2> reshaped = shuffled.reshape(Eigen::array<Index, 2>{left_size, right_size});
  Eigen::Map<MatrixS> matrix(reshaped.data(), left_size, right_size);
  // 调用householder QR
  Eigen::HouseholderQR<Eigen::Ref<MatrixS>> qr(matrix);
  // 再把矩阵变回tensor, R先不malloc，因为可能不需要
  Eigen::Tensor<Scalar, LeftRank+1> Q(left_new_shape);
  Eigen::Tensor<Scalar, RightRank+1> R;  // (right_new_shape);
  // 创建matrix map， 然后Q使用eigen的函数乘上identity，R使用matrixQR再删掉一些东西
  Eigen::Map<MatrixS> matrixQ(Q.data(), left_size, min_size);
  matrixQ = qr.householderQ() * MatrixS::Identity(left_size, min_size);
  if (computeR)
  {
    // 如果设置了computeR（默认true），那么算一下R矩阵
    R = Eigen::Tensor<Scalar, RightRank+1> (right_new_shape);
    Eigen::Map<MatrixS> matrixR(R.data(), min_size, right_size);
    auto matrixQR = qr.matrixQR();
    for (auto j = 0; j < right_size; j++)
    {
      // 截取上三角区域作为R，下面那半都是零
      auto fill_num = (min_size < (j+1))?min_size:(j+1);
      std::copy(&matrixQR(0, j), &matrixQR(0, j)+fill_num, &matrixR(0, j));
      std::fill(&matrixR(0, j)+fill_num, &matrixR(0, j)+min_size, 0);
    }
  }
  // leg处理一下
  Q.leg_info = left_new_leg;
  R.leg_info = right_new_leg;
  return __qr_res<Eigen::Tensor<Scalar, LeftRank+1>,
                    Eigen::Tensor<Scalar, RightRank+1>> {Q, R};
}

/* transpose */
template <typename TensorType>
EIGEN_DEVICE_FUNC Eigen::Tensor<
                    typename TensorType::Scalar,
                    TensorType::NumDimensions>
transpose(const TensorType& tensor,
          const Eigen::array<Leg, TensorType::NumDimensions>& new_legs)
{
  // 惯例，alias各种东西
  // typedef Eigen::internal::traits<TensorType> Traits;
  typedef typename TensorType::Index Index;
  typedef typename TensorType::Scalar Scalar;
  static const std::size_t Rank = TensorType::NumDimensions;
  // 准备转置后的脚
  Eigen::array<Index, Rank> to_shuffle;
  for (auto i = 0; i < Rank; i++)
  {
    // 在原来的leg中一个一个找新的leg
    to_shuffle[i] = __get_index(new_legs[i], tensor.leg_info);
  }
  // 然后就可以转置并设置新的leg了
  auto res = tensor.shuffle(to_shuffle);
  res.leg_info = new_legs;
  return res;
}

/* multiple */
// https://stackoverflow.com/questions/47040173/how-to-multiple-two-eigen-tensors-along-batch-dimension
template <typename TensorType>
EIGEN_DEVICE_FUNC Eigen::Tensor<
                    typename TensorType::Scalar,
                    TensorType::NumDimensions>
multiple(const TensorType& tensor,
         const Eigen::Tensor<typename TensorType::Scalar, 1>& vector,
         Leg leg)
{
  // typedef Eigen::internal::traits<TensorType> Traits;
  typedef typename TensorType::Index Index;
  typedef typename TensorType::Scalar Scalar;
  static const std::size_t Rank = TensorType::NumDimensions;
  auto index = __get_index(leg, tensor.leg_info);
  // 分别创建to_reshape和to_bcast，然后直接cwise乘起来
  Eigen::array<Index, Rank> to_reshape;  // {1,1,1,1,n,1,1,1,1}
  std::fill(to_reshape.begin(), to_reshape.end(), 1);
  to_reshape[index] = tensor.dimension(index);
  // reshape 后的bcast方式
  Eigen::array<Index, Rank> to_bcast;
  for (auto i = 0; i < Rank; i++)
  {
    to_bcast[i] = tensor.dimension(i);
  }
  to_bcast[index] = 1;
  // 注意cwise乘积顺序, leg是根据左边那个来的
  return tensor * vector.reshape(to_reshape).broadcast(to_bcast);
}

template <typename TensorType>
EIGEN_DEVICE_FUNC Eigen::Tensor<
                    typename TensorType::Scalar,
                    TensorType::NumDimensions>
max_normalize(const TensorType& tensor)
{
  typedef typename TensorType::Scalar Scalar;
  Scalar norm = Eigen::Tensor<Scalar, 0>(tensor.abs().maximum())();
  return tensor/norm;
}

template <typename TensorType>
EIGEN_DEVICE_FUNC Eigen::Tensor<
                    typename TensorType::Scalar,
                    TensorType::NumDimensions>
normalize(const TensorType& tensor)
{
  typedef typename TensorType::Scalar Scalar;
  Scalar norm = Eigen::Tensor<Scalar, 0>(tensor.square().sum())();
  return tensor/norm;
}

// #undef get_index
// #undef not_found
// #undef find_in

}  // namespace Node

#define DefineLeg(x) static Node::Leg x = Node::Leg::x
#define DefineLegs(n) DefineLeg(Left##n); DefineLeg(Right##n); DefineLeg(Up##n); DefineLeg(Down##n); DefineLeg(Phy##n)
DefineLegs();
DefineLegs(1);
DefineLegs(2);
DefineLegs(3);
DefineLegs(4);
#undef DefineLegs
#undef DefineLeg

// using Leg = Node::Leg;

// what export:
// include Eigen
// include Node(Leg, DefaultLeg, 几个运算用的函数, __*)
// macro debug_tensor
// Leg's var

#endif  // EIGEN_NODE_HPP_
