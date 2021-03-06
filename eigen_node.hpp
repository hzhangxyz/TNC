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
    CreateLeg(), CreateLeg(1), CreateLeg(2), CreateLeg(3), CreateLeg(4)
    #undef CreateLeg
  };

  namespace internal
  {
    #define IncEnum(p) {Leg::p, #p}
    #define IncGroup(x) IncEnum(Left##x), IncEnum(Right##x), IncEnum(Up##x), IncEnum(Down##x), IncEnum(Phy##x)
    static const std::map<Leg, std::string> leg_str = {IncGroup(), IncGroup(1), IncGroup(2), IncGroup(3), IncGroup(4)};
    // 如果const的话会报[]没有mark as const的错误
    #undef IncGroup
    #undef IncEnum
  }

  // ostream重载，使用一个static map来完成
  inline std::ostream& operator<<(std::ostream& out, const Leg& value)
  {
    try
    {
      return out << internal::leg_str.at(value);
    }
    catch(const std::out_of_range& e)
    {
      return out;
    }
  }

  namespace internal
  {
    /* 设置默认Leg */
    // 使用一个Trait来实现，TensorBase内部使用leg_info = DefaultLeg<DerivedTraits::NumDimensions>::value;来调用这里
    template<std::size_t rank>
    static const std::array<Leg, rank> DefaultLeg = {};
    // 如果是4 rank，那大概是哈密顿量，设置一下leg
    template <>
    const std::array<Leg, 4> DefaultLeg<4> = {Leg::Phy1, Leg::Phy2, Leg::Phy3, Leg::Phy4};
  }
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
// #define EIGEN_USE_LAPACKE
// #define EIGEN_USE_BLAS
#include <Eigen/Dense>
#include <Eigen/CXX11/Tensor>

namespace Node
{
  namespace internal
  {
    // check Tensor的一个macro
    void __debug_tensor(const auto& x, const char* name, std::ostream& os)
    {
      os << " " << name << "= { rank=" << x.NumDimensions << " dims=[";
      for (auto i = 0; i < x.NumDimensions; i++)
      {
        os << "(" << x.dimension(i) << "|" << x.leg_info[i] << "), ";
      }
      os << "], size=" << x.size();
      if (x.size() < 500)
      {
        os << ", data=" << std::endl << x << " }" << std::endl;
      }
      else
      {
        os << "}";
      }
    }
    #define debug_tensor(x) Node::internal::debug_tensor(x, #x, std::clog)
  }

  // leg name 是 Leg
  // index的序号是 Index
  // rank 是 std::size_t
  // dimension, size 是 Index

  namespace internal
  {
    EIGEN_DEVICE_FUNC inline auto find_in(const auto& it, const auto& pool)
    {
      return std::find(pool.begin(), pool.end(), it);
    }

    EIGEN_DEVICE_FUNC inline bool not_found(const auto& it, const auto& pool)
    {
      return find_in(it, pool) == pool.end();
    }

    EIGEN_DEVICE_FUNC inline int get_index(const auto& it, const auto& pool)
    {
      return std::distance(pool.begin(), find_in(it, pool));
    }

    template<typename TensorType>
    EIGEN_DEVICE_FUNC inline auto to_tensor(const TensorType& tensor)
    {
      return Eigen::Tensor<typename TensorType::Scalar, TensorType::NumDimensions> {tensor};
    }

    EIGEN_DEVICE_FUNC inline void check_in_and_map
    (const auto& it, const auto& leg, const auto& map, auto& res, auto& i)
    {
      if (not_found(it, leg))
      {
        auto l = map.find(it);
        if (l == map.end())
        {
          res.leg_info[i++] = it;
        }
        else
        {
          res.leg_info[i++] = l->second;
        }
      }
    }
  }

  /* contraction */
  // 好，这是contract，第一个参数是缩并脚标的类型，index类型使用了第一个tensor的trait
  // 不返回op了,直接返回eval后的东西
  // 注意contract num在最前面,为了partial deduction
  template<std::size_t ContractNum, typename TensorType1, typename TensorType2>
  EIGEN_DEVICE_FUNC auto contract
  (const TensorType1& tensor1,
   const TensorType2& tensor2,
   const Eigen::array<Leg, ContractNum>& leg1,
   const Eigen::array<Leg, ContractNum>& leg2,
   const std::map<Leg, Leg>& map1 = {},
   const std::map<Leg, Leg>& map2 = {})
  {
    // 构造给eigen用的缩并脚标对
    // typedef Eigen::internal::traits<TensorType1> Traits;
    using Index = typename TensorType1::Index;
    Eigen::array<Eigen::IndexPair<Index>, ContractNum> dims;
    for (auto i = 0; i < ContractNum; i++)
    {
      dims[i].first = internal::get_index(leg1[i], tensor1.leg_info);
      dims[i].second = internal::get_index(leg2[i], tensor2.leg_info);
    }
    // 然后运行，注意这里的auto返回的是一个op，是lazy的
    auto res = tensor1.contract(tensor2, dims);
    // 创建新的tensor的leg
    auto i = 0;
    // 根据是否在leg内，是否map，来更新result的leg info, 两部分一样，所以写成个宏
    // 改成函数了
    // 对于两个tensor之前的每个leg， 不在legs里则加入res，但如果还在map里需要先map
    for (auto j = 0; j < tensor1.leg_info.size(); j++)
    {
      internal::check_in_and_map(tensor1.leg_info[j], leg1, map1, res, i);
    }
    for (auto j = 0; j < tensor2.leg_info.size(); j++)
    {
      internal::check_in_and_map(tensor2.leg_info[j], leg2, map2, res, i);
    }
    return internal::to_tensor(res);
  }

  namespace internal
  {
    template<typename T1, typename T2, typename T3>
    class svd_res : public std::tuple<T1, T2, T3>
    {
    public:
      svd_res() : std::tuple<T1, T2, T3>(T1 {}, T2 {}, T3 {}) {}
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

    EIGEN_DEVICE_FUNC inline void copy_data(const auto& src, auto& dst)
    {
      std::copy(src.data(), src.data()+dst.size(), dst.data());
    }
  }

  /* svd */
  // svd返回的是含有U,S,V的一个tuple
  template<std::size_t SplitNum, typename TensorType>
  EIGEN_DEVICE_FUNC auto svd
  (const TensorType& tensor,
   const Eigen::array<Leg, SplitNum>& legs,
   Leg new_leg1,
   Leg new_leg2,
   typename TensorType::Index cut = -1)
  {
    // 先各种重命名烦人的东西
    // typedef Eigen::internal::traits<TensorType> Traits;
    using Index = typename TensorType::Index;
    using Scalar = typename TensorType::Scalar;
    using MatrixS = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;
    static const std::size_t Rank = TensorType::NumDimensions;
    static const std::size_t LeftRank = SplitNum;
    static const std::size_t RightRank = Rank - SplitNum;
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
      if (internal::not_found(tensor.leg_info[i], legs))
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
    left_new_shape[left_index] = min_size;
    right_new_shape[right_index] = min_size;
    // shuffle并reshape，当作matrix然后就可以svd了
    auto shuffled = tensor.shuffle(to_shuffle);
    Eigen::Tensor<Scalar, 2> reshaped = shuffled.reshape(Eigen::array<Index, 2>{left_size, right_size});
    Eigen::Map<MatrixS> matrix {reshaped.data(), left_size, right_size};
    // Eigen::JacobiSVD<MatrixS, Eigen::HouseholderQRPreconditioner> svd(matrix, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::BDCSVD<MatrixS> svd {matrix, Eigen::ComputeThinU | Eigen::ComputeThinV};
    // 再把矩阵变回tensor
    using UType = Eigen::Tensor<Scalar, LeftRank+1>;
    using SType = Eigen::Tensor<Scalar, 1>;
    using VType =Eigen::Tensor<Scalar, RightRank+1>;
    internal::svd_res<UType, SType, VType> res;
    res.U() = UType {std::move(left_new_shape)};
    res.S() = SType {Eigen::array<Index, 1> {min_size}};
    res.V() = VType {std::move(right_new_shape)};
    // 注意,这里的截断使用了列优先的性质,需要截断的那个脚是在最后面的
    //#define copy_data(src, dst) std::copy(src.data(), src.data()+dst.size(), dst.data())
    internal::copy_data(svd.matrixU(), res.U());
    internal::copy_data(svd.singularValues(), res.S());
    internal::copy_data(svd.matrixV(), res.V());
    //#undef copy_data
    res.U().leg_info = std::move(left_new_leg);
    if (new_leg1 == new_leg2)
    {
      res.S().leg_info = Eigen::array<Leg, 1> {new_leg1};
    }
    res.V().leg_info = std::move(right_new_leg);
    return res;
  }

  namespace internal
  {
    template<typename T1, typename T2>
    class qr_res : public std::tuple<T1, T2>
    {
    public:
      qr_res() : std::tuple<T1, T2>(T1 {}, T2 {}) {}
      inline T1& Q()
      {
        return std::get<0>(*this);
      }
      inline T2& R()
      {
        return std::get<1>(*this);
      }
    };
  }

  /* qr */
  // qr外部和svd一样，里面需要处理一下
  // http://www.netlib.org/lapack/explore-html/df/dc5/group__variants_g_ecomputational_ga3766ea903391b5cf9008132f7440ec7b.html
  template<std::size_t SplitNum, typename TensorType>
  EIGEN_DEVICE_FUNC auto qr
  (const TensorType& tensor,
   const Eigen::array<Leg, SplitNum>& legs,
   Leg new_leg1,
   Leg new_leg2,
   bool computeR = true)
  {
    // 先各种重命名烦人的东西
    // typedef Eigen::internal::traits<TensorType> Traits;
    using Index = typename TensorType::Index;
    using Scalar = typename TensorType::Scalar;
    using MatrixS = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;
    static const std::size_t Rank = TensorType::NumDimensions;
    static const std::size_t LeftRank = SplitNum;
    static const std::size_t RightRank = Rank - SplitNum;
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
      if (internal::not_found(tensor.leg_info[i], legs))
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
    Eigen::Map<MatrixS> matrix {reshaped.data(), left_size, right_size};
    // 调用householder QR
    Eigen::HouseholderQR<Eigen::Ref<MatrixS>> qr {matrix};
    // 再把矩阵变回tensor, R先不malloc，因为可能不需要
    using QType = Eigen::Tensor<Scalar, LeftRank+1>;
    using RType = Eigen::Tensor<Scalar, RightRank+1>;
    internal::qr_res<QType, RType> res;
    res.Q() = QType {std::move(left_new_shape)};
    // 创建matrix map， 然后Q使用eigen的函数乘上identity，R使用matrixQR再删掉一些东西
    Eigen::Map<MatrixS> matrixQ {res.Q().data(), left_size, min_size};
    matrixQ = qr.householderQ() * MatrixS::Identity(left_size, min_size);
    if (computeR)
    {
      // 如果设置了computeR（默认true），那么算一下R矩阵
      res.R() = RType {std::move(right_new_shape)};
      Eigen::Map<MatrixS> matrixR {res.R().data(), min_size, right_size};
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
    res.Q().leg_info = std::move(left_new_leg);
    res.R().leg_info = std::move(right_new_leg);
    return res;
  }

  /* transpose */
  template <typename TensorType>
  EIGEN_DEVICE_FUNC auto transpose
  (const TensorType& tensor,
   const Eigen::array<Leg, TensorType::NumDimensions>& new_legs)
  {
    // 惯例，alias各种东西
    // typedef Eigen::internal::traits<TensorType> Traits;
    using Index = typename TensorType::Index;
    using Scalar = typename TensorType::Scalar;
    static const std::size_t Rank = TensorType::NumDimensions;
    // 准备转置后的脚
    Eigen::array<Index, Rank> to_shuffle;
    for (auto i = 0; i < Rank; i++)
    {
      // 在原来的leg中一个一个找新的leg
      to_shuffle[i] = internal::get_index(new_legs[i], tensor.leg_info);
    }
    // 然后就可以转置并设置新的leg了
    auto res = tensor.shuffle(to_shuffle);
    res.leg_info = new_legs;
    return internal::to_tensor(res);
  }

  /* multiple */
  // https://stackoverflow.com/questions/47040173/how-to-multiple-two-eigen-tensors-along-batch-dimension

  template <typename TensorType>
  EIGEN_DEVICE_FUNC auto multiple
  (const TensorType& tensor,
   const Eigen::Tensor<typename TensorType::Scalar, 1>& vector,
   Leg leg)
  {
    // typedef Eigen::internal::traits<TensorType> Traits;
    using Index = typename TensorType::Index;
    using Scalar = typename TensorType::Scalar;
    static const std::size_t Rank = TensorType::NumDimensions;
    auto index = internal::get_index(leg, tensor.leg_info);
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
    // 这里这么做是因为fixed size的dimensions()并不能转换为array
    to_bcast[index] = 1;
    // 注意cwise乘积顺序, leg是根据左边那个来的
    return internal::to_tensor(tensor * vector.reshape(to_reshape).broadcast(to_bcast));
  }

  template <typename TensorType>
  EIGEN_DEVICE_FUNC auto
  max_normalize(const TensorType& tensor)
  {
    using Scalar = typename TensorType::Scalar;
    Scalar norm = Eigen::Tensor<Scalar, 0>(tensor.abs().maximum())();
    return internal::to_tensor(tensor/norm);
  }

  template <typename TensorType>
  EIGEN_DEVICE_FUNC auto
  normalize(const TensorType& tensor)
  {
    using Scalar = typename TensorType::Scalar;
    Scalar norm = Eigen::Tensor<Scalar, 0>(tensor.square().sum())();
    return internal::to_tensor(tensor/norm);
  }

}  // namespace Node

#define DefineLeg(x) static const Node::Leg x = Node::Leg::x
#define DefineLegs(n) DefineLeg(Left##n); DefineLeg(Right##n); DefineLeg(Up##n); DefineLeg(Down##n); DefineLeg(Phy##n)
DefineLegs(); DefineLegs(1); DefineLegs(2); DefineLegs(3); DefineLegs(4);
#undef DefineLegs
#undef DefineLeg

// using Leg = Node::Leg;

// what export:
// include Eigen
// include Node(Leg, DefaultLeg, 几个运算用的函数, __*)
// macro debug_tensor
// Leg's var

#endif  // EIGEN_NODE_HPP_
