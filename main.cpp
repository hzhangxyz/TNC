#include "eigen_tensor_interface.h"

const int L=10;
const int D=3;
const int phy_D = 2;
const double Hamiltonian_data[] = {1,0,0,0,0,-1,2,0,0,2,-1,0,0,0,0,1};
const Eigen::TensorMap<Eigen::Tensor<double, 4>> Hamiltonian = {(double *)Hamiltonian_data, {2,2,2,2}};
const double delta_t = 0.1;
const int step = 100;
const Eigen::TensorFixedSize<double,Eigen::Sizes<2,2,2,2>> another;// = {1,0,0,0,0,-1,2,0,0,2,-1,0,0,0,0,1};

int main(){
  Eigen::Tensor<double, 3> A,B;
  debug_tensor(A);
  A = Eigen::Tensor<double, 3>(phy_D,D,D*2);\
  A.setRandom();
  A.leg_info = {Phy1,Left,Right};
  B = Eigen::Tensor<double, 3>(phy_D,D*2,D);
  B.setRandom();
  B.leg_info = {Phy2,Left,Right};// 顺序不能换 random 和set leg
  debug_tensor(A);
  debug_tensor(B);
  Eigen::Tensor<double, 3> tt = A;
  debug_tensor(tt);

  Eigen::Tensor<double, 4> AB = node_contract(A,B,Eigen::array<Leg, 1>{Right},{Left},{{Phy1,Phy3}});
  debug_tensor(AB);

  debug_tensor(Hamiltonian);
}
