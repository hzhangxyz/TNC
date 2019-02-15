#include "eigen_node.hpp"

template<typename Base=double>
struct MPS
{
    int D;
    int L;
    int Dphy;
    Base delta_t;

    Eigen::Tensor<Base, 4> Hamiltonian;
    Eigen::Tensor<Base, 4> Identity;
    Eigen::Tensor<Base, 4> Update;

    std::vector<Eigen::Tensor<Base, 3>> lattice;
    std::vector<Eigen::Tensor<Base, 4>> __left_contract;
    std::vector<Eigen::Tensor<Base, 4>> __right_contract;

    MPS(int _D, int _L, float _delta_t = 0.01, int _Dphy = 2,
        Base* hamiltonian_data = nullptr) : D(_D), L(_L), delta_t(Base(_delta_t)), Dphy(_Dphy)
    {
        Base default_hamiltonian_data[16] = {
            1/4. , 0   , 0   , 0   ,
            0    ,-1/4., 2/4., 0   ,
            0    , 2/4.,-1/4., 0   ,
            0    , 0   ,    0, 1/4.};
        if (hamiltonian_data == nullptr)
        {
            hamiltonian_data = (Base*)default_hamiltonian_data;
        }

        Hamiltonian = Eigen::TensorMap<Eigen::Tensor<Base, 4>> {
            (Base *)hamiltonian_data,
            {Dphy, Dphy, Dphy, Dphy}
        };

        Identity = Eigen::TensorMap<Eigen::Tensor<Base, 4>> {
            (Base *) Eigen::Matrix<Base, Eigen::Dynamic, Eigen::Dynamic>
                ::Identity(Dphy*Dphy, Dphy*Dphy).eval().data(),
            {Dphy, Dphy, Dphy, Dphy}
        };
        Update = Identity - delta_t * Hamiltonian;

        lattice = std::vector<Eigen::Tensor<Base, 3>>(L, Eigen::Tensor<Base, 3>{});
        __left_contract = std::vector<Eigen::Tensor<Base, 4>>(L, Eigen::Tensor<Base, 4>{});
        __right_contract = std::vector<Eigen::Tensor<Base, 4>>(L, Eigen::Tensor<Base, 4>{});

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

        std::cout << "Created a MPS\n";
    }
    Eigen::Tensor<Base, 4> get_one()
    {
        static Eigen::Tensor<Base, 4> res;
        if (res.size() == 0)
        {
            res = Eigen::Tensor<Base, 4> (1,1,1,1);
            res(0, 0, 0, 0) = 1;
            res.leg_info = {Left1, Left2, Right1, Right2};
        }
        return res;
    }
    Eigen::Tensor<Base, 4> left_contract(int i)
    {
        if(i==-1)
        {
            return get_one();
        }
        if(__left_contract[i].size()==0)
        {
            if(i==0)
            {
                __left_contract[i] = Node::contract<1>(
                    lattice[i], lattice[i],
                    {Phy}, {Phy},
                    {{Left, Left1}, {Right, Right1}},
                    {{Left, Left2}, {Right, Right2}}
                );
            }
            else
            {
                __left_contract[i] = Node::contract<2>(
                    left_contract(i-1),
                    Node::contract<1>(
                        lattice[i],
                        lattice[i],
                        {Phy}, {Phy},
                        {{Left, Left1}, {Right, Right1}},
                        {{Left, Left2}, {Right, Right2}}
                    ),
                    {Right1, Right2}, {Left1, Left2});
            }
        }
        return __left_contract[i];
    }
    Eigen::Tensor<Base, 4> right_contract(int i)
    {
        if(i==L)
        {
            return get_one();
        }
        if(__right_contract[i].size()==0)
        {
            if(i==L-1)
            {
                __right_contract[i] = Node::contract<1>(
                    lattice[i], lattice[i],
                    {Phy}, {Phy},
                    {{Left, Left1}, {Right, Right1}},
                    {{Left, Left2}, {Right, Right2}}
                );
            }
            else
            {
                __right_contract[i] = Node::contract<2>(
                    right_contract(i+1),
                    Node::contract<1>(
                        lattice[i],
                        lattice[i],
                        {Phy}, {Phy},
                        {{Left, Left1}, {Right, Right1}},
                        {{Left, Left2}, {Right, Right2}}
                    ),
                    {Left1, Left2}, {Right1, Right2});
            }
        }
        return __right_contract[i];
    }
    void pre()
    {
        for(int i=L-1;i>1;i--)
        {
            auto qr = Node::qr<2>(lattice[i], {Phy, Right}, Left, Right);
            lattice[i] = qr.Q();
            lattice[i-1] = Node::contract<1>(lattice[i-1], qr.R(), {Right}, {Left});
        }
    }
    void update()
    {
        for(int i=0;i<L-1;i++)
        {
            auto big = Node::contract<1>(lattice[i], lattice[i+1],
                                      {Right}, {Left},
                                      {{Phy, Phy1}}, {{Phy, Phy2}});
            auto Big = Node::contract<2>(big, Update,
                                      {Phy1, Phy2}, {Phy1, Phy2});
            auto svd = Node::svd<2>(Big,
                                    {Left, Phy3}, Right, Left, D);
            lattice[i] = svd.U();
            lattice[i].leg_rename({{Phy3,Phy}});
            lattice[i+1] = Node::multiple(svd.V(), svd.S(), Left);
            lattice[i+1].leg_rename({{Phy4,Phy}});
        }
        for(int i=L-1;i>0;i--)
        {
            auto big = Node::contract<1>(lattice[i], lattice[i-1],
                                      {Left}, {Right},
                                      {{Phy, Phy1}}, {{Phy, Phy2}});
            auto Big = Node::contract<2>(big, Update,
                                      {Phy1, Phy2}, {Phy1, Phy2});
            auto svd = Node::svd<2>(Big,
                                    {Right, Phy3}, Left, Right, D);
            lattice[i] = svd.U();
            lattice[i].leg_rename({{Phy3,Phy}});
            lattice[i-1] = Node::multiple(svd.V(), svd.S(), Right);
            lattice[i-1].leg_rename({{Phy4,Phy}});
        }
        for(int i=0;i<L;i++)
        {
            lattice[i] = Node::max_normalize(lattice[i]);
        }
    }
    void update(int n)
    {
        pre();
        for(int i=0;i<n;i++)
        {
            update();
            std::cout << i << "\r" << std::flush;
        }
        std::cout << "\n";
    }
    Base energy_at_i_and_i_plus_1(int i)
    {
        auto psi = Node::contract<1>(
            lattice[i],
            lattice[i+1],
            {Right}, {Left},
            {{Phy, Phy1}}, {{Phy, Phy2}}
        );
        auto Hpsi = Node::contract<2>(
            psi,
            Hamiltonian,
            {Phy1, Phy2}, {Phy1, Phy2}
        );
        auto psiHpsi = Node::contract<2>(
            Hpsi,
            psi,
            {Phy3, Phy4}, {Phy1, Phy2},
            {{Left, Left1}, {Right, Right1}},
            {{Left, Left2}, {Right, Right2}}
        );
        auto leftpsiHpsi = Node::contract<2>(
            psiHpsi,
            left_contract(i-1),
            {Left1, Left2}, {Right1, Right2}
        );
        auto res = Node::contract<2>(
            leftpsiHpsi,
            right_contract(i+2),
            {Right1, Right2}, {Left1, Left2}
        );
        return res(0,0,0,0);
    }
    Base energy()
    {
        Base total = Base(0);
        for(int i=0;i<L-1;i++)
        {
            total += energy_at_i_and_i_plus_1(i);
        }
        total /= left_contract(L-1)(0,0,0,0);
        return total/L;
    }
};

int main(int n, char** argv)// D, L, T
{
    MPS<> mps {atoi(argv[1]), atoi(argv[2])};
    mps.update(atoi(argv[3]));
    std::cout << " E= " << mps.energy() << "\n";
    return 0;
}
