#include <Eigen/CXX11/Tensor>
#include <iostream>

int main(){
    Eigen::Tensor<double, 3> T (2, 2, 4);
    T.setRandom();
    Eigen::Tensor<double, 2> A (2, 5);
    A.setRandom();

    Eigen::array<Eigen::IndexPair<int>, 1> product_dims = { Eigen::IndexPair<int>(0, 0) };
    Eigen::Tensor<double, 3> M = T.contract(A, product_dims);
    std::cout << M.NumDimensions << "\n" ;
    std::cout << M.size() << "\n";
}
