#include "args.hxx"
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

#include <iostream>
#include <args.hxx>
int main(int argc, char **argv)
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<int> M(parser, "integer", "The integer flag", {'M'});
    args::ValueFlag<int> N(parser, "integer", "The integer flag", {'N'});
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    if (M) { std::cout << "M: " << args::get(M) << std::endl; }
    if (N) { std::cout << "N: " << args::get(N) << std::endl; }
    return 0;
}