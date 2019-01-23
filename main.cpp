#include "include.h"

const int L=10;
const int D=4;
const int phy_D = 2;
const double Hamiltonian_data[] = {1,0,0,0,0,-1,2,0,0,2,-1,0,0,0,0,1};
const Eigen::TensorMap<Eigen::Tensor<double, 4>> Hamiltonian = {(double *)Hamiltonian_data, {2,2,2,2}};
const double delta_t = 0.1;
const int step = 100;
const Eigen::TensorFixedSize<double,Eigen::Sizes<2,2,2,2>> another;// = {1,0,0,0,0,-1,2,0,0,2,-1,0,0,0,0,1};

int main(){


  Eigen::Tensor<double, 3> A,B;
  std::cout << "size=" << A.size() << ", rank=" <<  A.NumDimensions << std::endl;
  A = Eigen::Tensor<double, 3>(phy_D,D,D*2);\
  A.setRandom();
  A.set_leg({Phy1,Left,Right});
  B = Eigen::Tensor<double, 3>(phy_D,D*2,D);
  B.setRandom();
  B.set_leg({Phy2,Left,Right});// 顺序不能换 random 和set leg

  debug_tensor(A);
  //std::cout << Hamiltonian.leg_info[0] << std::endl;
  Eigen::Tensor<double, 3> tt = A;
  debug_tensor(tt);
  Eigen::IndexPair<int> pair {2, 1};
  std::array<Eigen::IndexPair<int>, 1> pairs {pair};
  Eigen::Tensor<double, 4> AXB = A.contract(B,pairs);
  std::cout << AXB.size() << std::endl;
  Eigen::Tensor<double, 4> AB = A.node_contract(B,Eigen::array<Leg, 1>{Right},{Left},{{Phy1,Phy3}});
  debug_tensor(AB);
  //debug_tensor(BA);
  Eigen::Tensor<double, 3> C = Eigen::Tensor<double, 3>(phy_D,D,D*2);\
  debug_tensor(C);
  //auto hl = AB.__get_default_leg_info();
  //std::cout << hl[0] << hl[1] << hl[2] << hl[3] << std::endl;
  debug_tensor(Hamiltonian);
  debug_tensor(another);
}
