#include "config.h"

const int L=10;
const int D=4;
const int phy_D = 2;
const double Hamiltonian_data[] = {1,0,0,0,0,-1,2,0,0,2,-1,0,0,0,0,1};

const double delta_t = 0.1;
const int step = 100;

int main(){
  Eigen::TensorMap<Eigen::Tensor<double, 4>> Hamiltonian = {(double *)Hamiltonian_data, {2,2,2,2}};
  Hamiltonian.leg_info = {Left,Left,Left,Left};

  Eigen::Tensor<double, 3> A,B;
  std::cout << "size=" << A.size() << ", rank=" <<  A.NumDimensions << std::endl;
  A = Eigen::Tensor<double, 3>(phy_D,D,D*2);\
  A.setRandom();
  A.set_leg({Phy1,Left,Right});
  B = Eigen::Tensor<double, 3>(phy_D,D*2,D);
  B.setRandom();
  B.set_leg({Phy2,Left,Right});// 顺序不能换 random 和set leg

  std::cout << A << std::endl;
  std::cout << A.leg_info[0] << std::endl;
  //std::cout << Hamiltonian.leg_info[0] << std::endl;
  Eigen::Tensor<double, 3> tt = A;
  std::cout << tt.leg_info[0] << std::endl;//!!!!! when copy, leg info lost
  Eigen::IndexPair<int> pair {2, 1};
  std::array<Eigen::IndexPair<int>, 1> pairs {pair};
  Eigen::Tensor<double, 4> AXB = A.contract(B,pairs);
  std::cout << AXB.size() << std::endl;
  Eigen::Tensor<double, 4> AB = A.node_contract(B,Eigen::array<Leg, 1>{Right},{Left},{{Phy1,Phy3}});
  debug_tensor(AB);
  //debug_tensor(BA);
}
