#include "config.h"
#include <iostream>
#include <vector>
enum Leg {Left,Right,Up,Down,Phy,Left1,Right1,Up1,Down1,Phy1,Left2,Right2,Up2,Down2,Phy2,Left3,Right3,Up3,Down3,Phy3,Left4,Right4,Up4,Down4,Phy4};
#define EIGEN_TENSOR_PLUGIN "plugin.h"
#include <Eigen/CXX11/Tensor>

const int L=10;
const int D=4;
const int phy_D = 2;
const double Hamiltonian_data[] = {1,0,0,0,0,-1,2,0,0,2,-1,0,0,0,0,1};
const Eigen::TensorMap<Eigen::Tensor<double, 4>> Hamiltonian = {(double *)Hamiltonian_data, {2,2,2,2}};
const double delta_t = 0.1;
const int step = 100;

int main(){
  Eigen::Tensor<double, 3> lattice[L];
  lattice[0] = Eigen::Tensor<double, 3>(phy_D,1,D);
  lattice[0].setRandom();
  for(int i=1;i<L-1;i++){
    lattice[i] = Eigen::Tensor<double, 3>(phy_D,D,D);
    lattice[i].setRandom();
  }
  lattice[L-1] = Eigen::Tensor<double, 3>(phy_D,D,1);
  lattice[L-1].setRandom();
  
  std::cout << lattice[L-1] << std::endl;
  lattice[L-1].set_leg({Left,Up,Right});
  std::cout << lattice[L-1].leg_info[1] << std::endl;
  std::cout << Hamiltonian.leg_info[1] << std::endl;
  Eigen::Tensor<double, 3> tt = lattice[L-1];
  std::cout << tt.leg_info[1] << std::endl;//!!!!!
}
