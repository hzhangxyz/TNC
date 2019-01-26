#include <iostream>
#include <cassert>
#include "../eigen_node.hpp"

//using TensorType=Eigen::Tensor<int, 4>;

template <typename TensorType>
EIGEN_DEVICE_FUNC TensorType
transpose(const TensorType& tensor)
{
  typedef typename TensorType::Index Index;
  static const std::size_t Rank = TensorType::NumDimensions;
  TensorType res = tensor.shuffle(Eigen::array<Index, Rank>{2,1,0,3});
  return res;
}

int main(){
  Eigen::TensorFixedSize<int, Eigen::Sizes<2,3,4,5>> A;
  A.setRandom();
  Eigen::Tensor<int, 4> B = transpose(A);
}