CXXFLAGS= -I./eigen-git-mirror -I./eigen-git-mirror/unsupported -g -L/opt/intel/mkl/lib/intel64 -Wl,--no-as-needed -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lpthread -lm -ldl
gcc:
	g++ main.cpp $(CXXFLAGS)
	./a.out

icc:
	icc main.cpp $(CXXFLAGS)
	./a.out

clang:
	clang++ main.cpp $(CXXFLAGS)
	./a.out
