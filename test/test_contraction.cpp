#include "eigen_node.hpp"
#include <iostream>

int main(){
    Eigen::Tensor<int, 2> a(2, 3);
    a.setValues({{1, 2, 3}, {6, 5, 4}});
    Eigen::Tensor<int, 2> b(3, 2);
    b.setValues({{1, 2}, {4, 5}, {5, 6}});

    // Compute the traditional matrix product
    Eigen::array<Eigen::IndexPair<int>, 1> product_dims = { Eigen::IndexPair<int>(1, 0) };
    Eigen::Tensor<int, 2> AB = a.contract(b, product_dims);
}