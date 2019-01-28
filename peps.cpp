#include "eigen_node.hpp"

template<typename Base=double>
class PEPS
{
    int N = 4;
    int M = 4;
    int D = 4;
    int Dphy = 2;

    std::vector<std::vector<Eigen::Tensor<Base, 5>>> lattice;

    PEPS(int _N, int _M) : N(_N), M(_M)
    {
        lattice = std::vector<std::vector<Eigen::Tensor<Base, 5>>>(N,
            std::vector<Eigen::Tensor<Base, 5>>(M,
                Eigen::Tensor<Base, 5>{}
            )
        );
    }
};

int main()
{
    return 0;
}