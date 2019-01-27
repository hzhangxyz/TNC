#include <iostream>
#include <cassert>
#include "eigen_node.hpp"

void test_copy_and_leg_info(){
  // Tensor
  Eigen::Tensor<float, 4> T1;
  assert(T1.leg_info[1]==Phy2);
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
  auto/*Eigen::Tensor<double, 4>*/ AB = Node::contract<1>(A, B, {Right1}, {Left2}, {{Phy1, Phy3}}, {{Phy2, Phy4}});
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
  auto svd = Node::svd<2>(A, {Up, Down}, Phy, Phy);
  //debug_tensor(A);
  //debug_tensor(svd.U());
  //debug_tensor(svd.S());
  //debug_tensor(svd.V());
  float tmp = 0;
  for(int i=0;i<10;i++){
    tmp += svd.U()(2,3,i) * svd.S()(i) * svd.V()(1,4,i);
  }
  assert(abs(tmp-A(1,2,3,4))<1e-5);
  tmp = 0;
  for(int i=0;i<10;i++){
    tmp += svd.U()(1,3,i) * svd.S()(i) * svd.V()(1,2,i);
  }
  assert(abs(tmp-A(1,1,3,2))<1e-5);
  int cut = 8;
  auto svd_cut = Node::svd<2>(A, {Up, Down}, Phy, Phy, cut);
  tmp = 0;
  for(int i=0;i<cut;i++){
    tmp += svd.U()(1,3,i) * svd.S()(i) * svd.V()(1,2,i);
  }
  //for(int i=0;i<cut;i++){
  //  std::cout << svd.S()(i) << " ";
  //}
  //std::cout << "\n" << tmp << "-" << A(1,1,3,2) << "=" << tmp-A(1,1,3,2) << std::endl;
  assert(svd.S()(0)>svd.S()(1));
  assert(abs(tmp-A(1,1,3,2))<0.1);
}

void test_qr(){
  Eigen::TensorFixedSize<double, Eigen::Sizes<2,3,4,5>> A;
  A.setRandom();
  A.leg_info = {Left, Up, Down, Right};
  auto qr = Node::qr<2>(A, {Up, Down}, Phy, Phy);
  //debug_tensor(A);
  //debug_tensor(qr.Q());
  //debug_tensor(qr.R());
  double tmp = 0;
  for(int i=0;i<10;i++){
    tmp += qr.Q()(1,1,i) * qr.R()(i,0,3);
  }
  assert(abs(tmp-A(0,1,1,3))<1e-5);
  assert(qr.Q().leg_info[0]==Up);
  assert(qr.Q().leg_info[1]==Down);
  assert(qr.Q().leg_info[2]==Phy);
  assert(qr.R().leg_info[0]==Phy);
  assert(qr.R().leg_info[1]==Left);
  assert(qr.R().leg_info[2]==Right);
  Eigen::TensorFixedSize<double, Eigen::Sizes<2,3,4,5>> B;
  B.setRandom();
  B.leg_info = {Left, Up, Down, Right};
  auto qr2 = Node::qr<2>(B, {Left, Right}, Phy, Phy, false);
  //debug_tensor(B);
  //debug_tensor(std::get<0>(qr2));
  //debug_tensor(std::get<1>(qr2));
  auto qr3 = Node::qr<2>(B, {Left, Right}, Phy, Phy);
  //debug_tensor(std::get<0>(qr3));
  //debug_tensor(std::get<1>(qr3));
  assert(qr2.Q()(1,1,1)=qr3.Q()(1,1,1));
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
  Eigen::TensorFixedSize<int, Eigen::Sizes<2,3,4,5>> A;
  A.setRandom();
  A.leg_info = {Left, Up, Down, Right};
  Eigen::Tensor<int, 4> B = Node::transpose(A, {Right, Up, Left, Down});
  //debug_tensor(A);
  //debug_tensor(B);
  assert(B.dimension(0)==5);
  assert(B.dimension(1)==3);
  assert(B.dimension(2)==2);
  assert(B.dimension(3)==4);
  assert(A(0,1,2,3)==B(3,1,0,2));
}

void test_multiple(){
  Eigen::TensorFixedSize<float, Eigen::Sizes<2,3,4,5>> A;
  A.setRandom();
  A.leg_info = {Left, Up, Down, Right};
  Eigen::TensorFixedSize<float, Eigen::Sizes<4>> B;
  B.setValues({1, 2, 3, 4});
  decltype(A) C = Node::multiple(A, B, Down);
  assert(A(1,2,3,4)*4==C(1,2,3,4));
  assert(A(1,0,3,2)*4==C(1,0,3,2));
  assert(A(1,0,2,2)*3==C(1,0,2,2));
  assert(A(1,1,0,2)==C(1,1,0,2));
  assert(A.leg_info[0]==C.leg_info[0]);
  assert(A.leg_info[1]==C.leg_info[1]);
  assert(A.leg_info[2]==C.leg_info[2]);
  assert(A.leg_info[3]==C.leg_info[3]);
}

int main(){
  test_copy_and_leg_info();
  test_contract();
  test_svd();
  test_qr();
  test_scalar();
  test_transpose();
  test_multiple();
  return 0;
}
