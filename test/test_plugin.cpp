#define EIGEN_MATRIX_PLUGIN "test_plugin.h"
#include <Eigen/Dense>
#include <iostream>

using namespace Eigen;

int main(){
  MatrixXd a(2,2);
  a << 1,2,3,4;
  std::cout << a << "\n";
  std::cout << a.leg_info << "\n";
  a.leg_info = 24;
  std::cout << a.leg_info << "\n";
  MatrixXd b = a;
  std::cout << b.leg_info << "\n";
}
