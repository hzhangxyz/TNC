#include <Eigen/CXX11/Tensor>
#include <iostream>

int main(){
    Eigen::Tensor<double, 4> T (2, 2, 2, 2);
    T.setRandom();
    Eigen::Tensor<double, 2> A (2, 2);
    A.setRandom();

    Eigen::array<Eigen::IndexPair<int>, 1> product_dims = { Eigen::IndexPair<int>(0, 0) };
    Eigen::Tensor<double, 4> M = T.contract(A, product_dims);
    std::cout << M.NumDimensions << "\n" ;
}
