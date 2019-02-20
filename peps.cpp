#include <args.hxx>
#include "eigen_node.hpp"

template<typename Base = double>
class PEPS
{
  public:
    std::size_t M;
    std::size_t N;
    int D;
    int D_c;
    int scan;
    int markov;
    int step;
    std::string save_prefix;
    std::string load_from;

    int Dphy = 2;
    using Site = Eigen::Tensor<Base, 5>;
    using OneLine = std::vector<Site>;
    using Lattice = std::vector<OneLine>;
    Lattice lattice;

    PEPS(auto _M, auto _N, int _D, int _D_c, int _scan, int _markov, double _step, std::string _save, std::string _load)
     : M(_M), N(_N), D(_D), D_c(_D_c), scan(_scan), markov(_markov), step(_step), save_prefix(_save), load_from(_load)
    {
      auto t1 = Site {D, D, D, D, 2};
      auto t2 = OneLine {N, t1};
      lattice = Lattice {M, t2};//列优先
      std::cout << load_from.empty() << "\n";
      std::cout << load_from << "\n";
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
  args::ValueFlag<int> D_c(runGM, "D_cut", "dimension cut in MPO", {'D',"dim-cut"});
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
  if (runGM)
  {
    std::string load_from = "";
    if (continue_run)
    {
      load_from = args::get(save) + "/last/last.save";
    }
    if (load)
    {
      load_from = args::get(load);
    }
    PEPS<> lattice
    {
      args::get(M),
      args::get(N),
      args::get(D),
      args::get(D_c),
      args::get(scan),
      args::get(markov),
      args::get(step),
      args::get(save),
      load_from
    };
  }
  else
  {
    std::cout << parser;
  }

  return 0;
}
