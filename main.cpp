#include "config.h"
#include <iostream>
#include <vector>
enum Leg {Left,Right,Up,Down,Phy,Left1,Right1,Up1,Down1,Phy1,Left2,Right2,Up2,Down2,Phy2,Left3,Right3,Up3,Down3,Phy3,Left4,Right4,Up4,Down4,Phy4};
#define EIGEN_TENSOR_PLUGIN "eigen_tensor_plugin.h"
#include <Eigen/CXX11/Tensor>

const int L=10;
const int D=4;
const int phy_D = 2;
const double Hamiltonian_data[] = {1,0,0,0,0,-1,2,0,0,2,-1,0,0,0,0,1};
const Eigen::TensorMap<Eigen::Tensor<double, 4>> Hamiltonian = {(double *)Hamiltonian_data, {2,2,2,2}};
const double delta_t = 0.1;
const int step = 100;

int main(){
  Eigen::Tensor<double, 3> A,B;
  std::cout << "size=" << A.size() << ", rank=" <<  A.NumDimensions << std::endl;
  A = Eigen::Tensor<double, 3>(phy_D,D,D*2);\
  A.set_leg({Phy1,Left,Right});
  A.setRandom();
  B = Eigen::Tensor<double, 3>(phy_D,D*2,D);
  B.set_leg({Phy2,Left,Right});
  B.setRandom();
  
  std::cout << A << std::endl;
  std::cout << A.leg_info[0] << std::endl;
  //std::cout << Hamiltonian.leg_info[0] << std::endl;
  Eigen::Tensor<double, 3> tt = A;
  std::cout << tt.leg_info[0] << std::endl;//!!!!! when copy, leg info lost
  Eigen::IndexPair<int> pair {2, 1};
  std::array<Eigen::IndexPair<int>, 1> pairs {pair};
  Eigen::Tensor<double, 4> AXB = A.contract(B,pairs);
  std::cout << AXB.size() << std::endl;
  Eigen::Tensor<double, 4> AB = A.node_contract(B,Eigen::array<Leg, 1>{Right},Eigen::array<Leg, 1>{Left});
  Eigen::Tensor<double, 4> BA = A.node_contract(B,Eigen::array<Leg, 1>{Left},{Right});
  std::cout << AB.size() << " " << BA.size() << "\n";
  Eigen::array<Leg,3> lleg {Phy,Right,Phy1};
  auto qq = AB.leg_info;
  std::cout << "contract_leg_info " << qq[0] << " " << qq[1] << " " << qq[2] << " " << qq[3] << "\n";
  auto ii = A.get_index_from_leg(lleg);
  std::cout << ii[0] << " " << ii[1] << " " << ii[2] << "\n";
}
