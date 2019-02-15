#include <args.hxx>
#include "eigen_node.hpp"

template<typename Base = double>
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

int main(int argc, char **argv)
{
    args::ArgumentParser parser("This is a test program.", "This goes after the options.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<int> M(parser, "M", "system size m", {'M',"size-m"});
    args::ValueFlag<int> N(parser, "N", "system size n", {'N',"size-n"});
    args::ValueFlag<int> D(parser, "D", "bond dimension", {'d',"dim"});
    args::ValueFlag<int> D_cut(parser, "D_cut", "dimension cut in MPO", {'D',"dim-cut"});
    args::ValueFlag<int> scan(parser, "SCAN_TIME", "scan time in MPO", {'s',"scan-time"});
    args::ValueFlag<int> markov(parser, "MARKOV_LENGTH", "markov chain length", {'m',"markov"});
    args::ValueFlag<double> step(parser, "STEP_SIZE", "step size in SU or GM", {'l',"step-size"});
    args::ValueFlag<std::string> load(parser, "LOAD_FROM", "load from file", {'f',"load-from"});
    args::ValueFlag<std::string> save(parser, "SAVE_PREFIX", "prefix for saving data", {'p',"save-prefix"});
    args::Flag continue_run(parser, "", "continue run", {'c', "continue"});
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
