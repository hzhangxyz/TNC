#include <iostream>
#include <cassert>
#include "eigen_tensor_interface.h"

void test_copy_and_leg_info(){
  // Tensor
  Eigen::Tensor<float, 4> T1;
  assert(T1.leg_info[1]=Phy2);
  Eigen::Tensor<int, 3> T2(2,3,4);
  T2.leg_info = {Left1, Left2, Left3};
  Eigen::Tensor<int, 3> T3 = T2;
  assert(T2.data()!=T3.data());
  assert(T2.leg_info[2]==T3.leg_info[2]);
  // TensorFixedSize
  Eigen::TensorFixedSize<double, Eigen::Sizes<2,3,4,5>> T4;
  assert(T4.leg_info[3]==Phy4);
  Eigen::TensorFixedSize<int, Eigen::Sizes<2,3,4>> T5 = T3;
  assert(T5.leg_info[1]==T2.leg_info[1]);
  assert(T5(1,2,3)==T3(1,2,3));
  assert(T5.data()!=T3.data());
  // TensorMap
  const double Hamiltonian_data[] = {1,0,0,0,0,-1,2,0,0,2,-1,0,0,0,0,1};
  const Eigen::TensorMap<Eigen::Tensor<double, 4>> Hamiltonian = {(double *)Hamiltonian_data, {2,2,2,2}};
  assert(Hamiltonian.leg_info[3]==Phy4);
  assert(Hamiltonian(0,1,1,0)==2);
}

void test_contract(){
  Eigen::Tensor<double, 3> A(3,4,2);
  Eigen::Tensor<double, 3> B(4,3,2);
  A.setRandom();
  B.setRandom();
  A.leg_info = {Phy1, Left1, Right1};
  B.leg_info = {Phy2, Left2, Right2};
  Eigen::Tensor<double, 4> AB = node_contract(A, B, Eigen::array<Leg,1>{Right1}, {Left2}, {{Phy1, Phy3}}, {{Phy2, Phy4}});
}

const int L=10;
const int D=3;
const int phy_D = 2;
const double Hamiltonian_data[] = {1,0,0,0,0,-1,2,0,0,2,-1,0,0,0,0,1};
const Eigen::TensorMap<Eigen::Tensor<double, 4>> Hamiltonian = {(double *)Hamiltonian_data, {2,2,2,2}};
const double delta_t = 0.1;
const int step = 100;

int main(){
  test_copy_and_leg_info();
  /*
  Eigen::Tensor<double, 3> A,B;
  A = Eigen::Tensor<double, 3>(phy_D,D,D*2);\
  A.setRandom();
  A.leg_info = {Phy1,Left,Right};
  B = Eigen::Tensor<double, 3>(phy_D,D*2,D);
  B.setRandom();
  B.leg_info = {Phy2,Left,Right};// 顺序不能换 random 和set leg
  Eigen::Tensor<double, 3> tt = A;
  debug_tensor(tt);

  Eigen::Tensor<double, 4> AB = node_contract(A,B,Eigen::array<Leg, 1>{Right},{Left},{{Phy1,Phy3}});
  debug_tensor(AB);

  debug_tensor(Hamiltonian);
  Eigen::Tensor<double, 4> Test(2,3,4,5);
  Test.setRandom();
  Test(0,0,0,0) = 0;
  Test(1,0,0,0) = 2;
  Test(0,1,0,0) = 3;
  Test(0,0,1,0) = 4;
  Test(0,0,0,1) = 5;
  debug_tensor(Test);
  auto svd = node_svd(Test, Eigen::array<Leg, 2>{Phy3, Phy2}, Right3);
  debug_tensor(std::get<0>(svd));
  debug_tensor(std::get<1>(svd));
  debug_tensor(std::get<2>(svd));*/
  return 0;
}
