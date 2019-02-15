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
    args::Group runGM(parser, "runGM system parameter", args::Group::Validators::AllOrNone);
    args::ValueFlag<int> M(runGM, "M", "system size m", {'M',"size-m"});
    args::ValueFlag<int> N(runGM, "N", "system size n", {'N',"size-n"});
    args::ValueFlag<int> D(runGM, "D", "bond dimension", {'d',"dim"});
    args::ValueFlag<int> D_cut(runGM, "D_cut", "dimension cut in MPO", {'D',"dim-cut"});
    args::ValueFlag<int> scan(runGM, "SCAN_TIME", "scan time in MPO", {'s',"scan-time"});
    args::ValueFlag<int> markov(runGM, "MARKOV_LENGTH", "markov chain length", {'m',"markov"});
    args::ValueFlag<double> step(runGM, "STEP_SIZE", "step size in SU or GM", {'l',"step-size"});
    args::ValueFlag<std::string> save(parser, "SAVE_PREFIX", "prefix for saving data", {'p',"save-prefix"}, "run", false);
    args::Group continue_or_from(parser, "continue or load from file", args::Group::Validators::AtMostOne);
    args::ValueFlag<std::string> load(continue_or_from, "LOAD_FROM", "load from file", {'f',"load-from"});
    args::Flag continue_run(continue_or_from, "", "continue run", {'c', "continue"});
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
    std::cout << "save-prefix: " << args::get(save) << std::endl;
    return 0;
}
