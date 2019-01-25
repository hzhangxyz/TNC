#include <Eigen/CXX11/Tensor>
#include <iostream>

int main(){
    Eigen::Tensor<int, 1> a(3);
    a.setRandom();
    Eigen::Tensor<int, 2> b(3, 2);
    b.setValues({{1, 2}, {4, 5}, {5, 6}});

    Eigen::array<Eigen::IndexPair<int>, 1> product_dims = { Eigen::IndexPair<int>(0, 0) };
    auto AB = a.contract(b, product_dims);
    std::cout << AB.NumDimensions << "\n" ;
}
