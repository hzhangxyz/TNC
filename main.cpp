#include <iostream>
#include <cassert>
#include "eigen_node.hpp"

void test_copy_and_leg_info(){
  // Tensor
  Eigen::Tensor<float, 4> T1;
  assert(T1.leg_info[1]=Phy2);
  Eigen::Tensor<int, 3> T2(2, 3, 4);
  T2.leg_info = {Left1, Left2, Left3};
  Eigen::Tensor<int, 3> T3 = T2;
  assert(T2.data()!=T3.data());
  assert(T2.leg_info[2]==T3.leg_info[2]);
  // TensorFixedSize
  Eigen::TensorFixedSize<double, Eigen::Sizes<2, 3, 4, 5>> T4;
  assert(T4.leg_info[3]==Phy4);
  Eigen::TensorFixedSize<int, Eigen::Sizes<2, 3, 4>> T5 = T3;
  assert(T5.leg_info[1]==T2.leg_info[1]);
  assert(T5(1, 2, 3)==T3(1, 2, 3));
  assert(T5.data()!=T3.data());
  // TensorMap
  const double Hamiltonian_data[] = {1, 0, 0, 0, 0, -1, 2, 0, 0, 2, -1, 0, 0, 0, 0, 1};
  const Eigen::TensorMap<Eigen::Tensor<double, 4>> Hamiltonian = {(double *)Hamiltonian_data, {2, 2, 2, 2}};
  assert(Hamiltonian.leg_info[3]==Phy4);
  assert(Hamiltonian(0, 1, 1, 0)==2);
}

void test_contract(){
  Eigen::TensorFixedSize<double, Eigen::Sizes<3, 4, 2>> A;
  Eigen::Tensor<double, 3> B(4, 3, 2);
  A.setRandom();
  B.setRandom();
  A.leg_info = {Left1, Right1, Phy1};
  B.leg_info = {Left2, Right2, Phy2};
  Eigen::Tensor<double, 4> AB = node_contract(A, B, Eigen::array<Leg,1>{Right1}, {Left2}, {{Phy1, Phy3}}, {{Phy2, Phy4}});
  assert(AB.leg_info[0]==Left1);
  assert(AB.leg_info[1]==Phy3);
  assert(AB.leg_info[2]==Right2);
  assert(AB.leg_info[3]==Phy4);
  assert(A(2,0,1)*B(0,1,0)+A(2,1,1)*B(1,1,0)+A(2,2,1)*B(2,1,0)+A(2,3,1)*B(3,1,0)==AB(2,1,1,0));
}

void test_svd(){
  Eigen::TensorFixedSize<float, Eigen::Sizes<2,3,4,5>> A;
  A.setRandom();
  A.leg_info = {Left, Up, Down, Right};
  auto svd = node_svd(A, Eigen::array<Leg, 2>{Up, Down}, Phy);
  //debug_tensor(A);
  //debug_tensor(std::get<0>(svd));
  //debug_tensor(std::get<1>(svd));
  //debug_tensor(std::get<2>(svd));
  float tmp = 0;
  for(int i=0;i<10;i++){
    tmp += std::get<0>(svd)(2,3,i) * std::get<1>(svd)(i) * std::get<2>(svd)(1,4,i);
  }
  assert(abs(tmp-A(1,2,3,4))<1e-5);
  tmp = 0;
  for(int i=0;i<10;i++){
    tmp += std::get<0>(svd)(1,3,i) * std::get<1>(svd)(i) * std::get<2>(svd)(1,2,i);
  }
  assert(abs(tmp-A(1,1,3,2))<1e-5);
  int cut = 8;
  auto svd_cut = node_svd(A, Eigen::array<Leg, 2>{Up, Down}, Phy, cut);
  tmp = 0;
  for(int i=0;i<cut;i++){
    tmp += std::get<0>(svd)(1,3,i) * std::get<1>(svd)(i) * std::get<2>(svd)(1,2,i);
  }
  //for(int i=0;i<cut;i++){
  //  std::cout << std::get<1>(svd)(i) << " ";
  //}
  //std::cout << "\n" << tmp << "-" << A(1,1,3,2) << "=" << tmp-A(1,1,3,2) << std::endl;
  assert(std::get<1>(svd)(0)>std::get<1>(svd)(1));
  assert(abs(tmp-A(1,1,3,2))<0.1);
}

void test_qr(){
  Eigen::TensorFixedSize<double, Eigen::Sizes<2,3,4,5>> A;
  A.setRandom();
  A.leg_info = {Left, Up, Down, Right};
  auto qr = node_qr(A, Eigen::array<Leg, 2>{Up, Down}, Phy);
  //debug_tensor(A);
  //debug_tensor(std::get<0>(qr));
  //debug_tensor(std::get<1>(qr));
  double tmp = 0;
  for(int i=0;i<10;i++){
    tmp += std::get<0>(qr)(1,1,i) * std::get<1>(qr)(i,0,3);
  }
  assert(abs(tmp-A(0,1,1,3))<1e-5);
  assert(std::get<0>(qr).leg_info[0]==Up);
  assert(std::get<0>(qr).leg_info[1]==Down);
  assert(std::get<0>(qr).leg_info[2]==Phy);
  assert(std::get<1>(qr).leg_info[0]==Phy);
  assert(std::get<1>(qr).leg_info[1]==Left);
  assert(std::get<1>(qr).leg_info[2]==Right);
  Eigen::TensorFixedSize<double, Eigen::Sizes<2,3,4,5>> B;
  B.setRandom();
  B.leg_info = {Left, Up, Down, Right};
  auto qr2 = node_qr(B, Eigen::array<Leg, 2>{Left, Right}, Phy, false);
  //debug_tensor(B);
  //debug_tensor(std::get<0>(qr2));
  //debug_tensor(std::get<1>(qr2));
  auto qr3 = node_qr(B, Eigen::array<Leg, 2>{Left, Right}, Phy);
  //debug_tensor(std::get<0>(qr3));
  //debug_tensor(std::get<1>(qr3));
  assert(std::get<0>(qr2)(1,1,1)=std::get<0>(qr3)(1,1,1));
}

void test_scalar(){
  Eigen::TensorFixedSize<double, Eigen::Sizes<2,3,4,5>> A;
  A.setRandom();
  A.leg_info = {Left, Up, Down, Right};
  Eigen::TensorFixedSize<double, Eigen::Sizes<2,3,4,5>> B = A * 2.0;
  assert(A.leg_info[0]==B.leg_info[0]);
  assert(A.leg_info[1]==B.leg_info[1]);
  assert(A.leg_info[2]==B.leg_info[2]);
  assert(A.leg_info[3]==B.leg_info[3]);
  assert(A(0,0,1,1)*2.0==B(0,0,1,1));
}

void test_transpose(){
  Eigen::TensorFixedSize<double, Eigen::Sizes<2,3,4,5>> A;
  A.setRandom();
  A.leg_info = {Left, Up, Down, Right};
  Eigen::Tensor<double, 4> B = node_transpose(A, Eigen::array<Leg, 4>{Right, Up, Left, Down});
  //debug_tensor(A);
  //debug_tensor(B);
  assert(B.dimension(0)==5);
  assert(B.dimension(1)==3);
  assert(B.dimension(2)==2);
  assert(B.dimension(3)==4);
  assert(A(0,1,2,3)==B(3,1,0,2));
}

int main(){
  test_copy_and_leg_info();
  test_contract();
  test_svd();
  test_qr();
  test_scalar();
  test_transpose();
  return 0;
}
