#include <iostream>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

int main()
{
  MatrixXf A(MatrixXf::Random(5,3)), Q, R, h;
  A.setRandom();
  std ::cout << " A=\n" << A << "\n\n";
  HouseholderQR<MatrixXf> qr(A);
  Q = qr.householderQ();
  std::cout << " householderQ=\n" << Q << "\n\n";
  std::cout << " matrixQR=\n" << qr.matrixQR() << "\n\n";
  std::cout << " hCoeffs=\n" << qr.hCoeffs() << "\n\n";
}
