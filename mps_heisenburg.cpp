#include "eigen_node.hpp"

template<int D, int L, typename Base=double>
struct MPS
{
    static const int Dphy = 2;
    Base Hamiltonian_data[16] = {
        1, 0, 0, 0,
        0,-1, 2, 0,
        0, 2,-1, 0,
        0, 0, 0, 1};
    Eigen::TensorMap<Eigen::Tensor<Base, 4>> Hamiltonian = {
        (Base *)Hamiltonian_data,
        {2, 2, 2, 2}
    };

    std::array<Eigen::Tensor<Base, 3>, L> lattice;
    //std::array<Eigen::Tensor<Base, 1>, L-1> env;
    MPS()
    {
        Hamiltonian = Hamiltonian/Base(4);
        for(int i=1;i<L-1;i++)
        {
            lattice[i] = Eigen::Tensor<Base, 3>(D, D, Dphy);
        }
        lattice[0] = Eigen::Tensor<Base, 3>(1, D, Dphy);
        lattice[L-1] = Eigen::Tensor<Base, 3>(D, 1, Dphy);
        for(int i=0;i<L;i++)
        {
            lattice[i] = lattice[i].random()*Base(2) - Base(1);
            lattice[i].leg_info = {Left, Right, Phy};
        }
        /*for(int i=0;i<L-1;i++)
        {
            env[i] = Eigen::Tensor<Base, 1>(D);
            env[i].setConstant(Base(1));
        }*/
    }

    void pre()
    {
        for(int i=L-1;i>1;i--)
        {
            auto qr = Node::qr(lattice[i], std::array<Leg,2>{Phy, Right}, Left, Right);
            lattice[i] = std::get<0>(qr);
            lattice[i-1] = Node::contract(lattice[i-1], qr, std::array<Leg,1>{Right}, {Left});
        }
    }
    void update()
    {
        for(int i=0;i<L-1;i++){
            //auto big = Node::contract(lattice[i], lattice[i+1])
        }
    }
};

int main()
{
    MPS<4, 4> mps;
    debug_tensor(mps.lattice[5]);
    mps.pre();
    debug_tensor(mps.lattice[5]);
    return 0;
}
  