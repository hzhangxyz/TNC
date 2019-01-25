#include "eigen_node.hpp"

static const double Hamiltonian_data[] = {1, 0, 0, 0, 0, -1, 2, 0, 0, 2, -1, 0, 0, 0, 0, 1};
static const Eigen::TensorMap<Eigen::Tensor<double, 4>> Hamiltonian = {(double *)Hamiltonian_data, {2, 2, 2, 2}};

int main()
{
    return 0;
}
  