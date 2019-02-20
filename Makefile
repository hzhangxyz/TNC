CXX=g++
CXXFLAGS+= -I./eigen-git-mirror -I./eigen-git-mirror/unsupported -I./args
CXXFLAGS+= -L/opt/intel/mkl/lib/intel64 -lmkl_intel_lp64 -lmkl_core -lmkl_sequential
CXXFLAGS+= -lpthread -lrt -DEIGEN_USE_MKL_ALL -I/opt/intel/mkl/include
CXXFLAGS+= --std=c++14 -fconcepts
CXXFLAGS+= -O3 -march=native -fwhole-program

DEBUG?=0
ifeq ($(DEBUG), 0)
else
	CXXFLAGS+= -g
endif

object = main mps_heisenburg peps

default: main

$(object): %: %.cpp
	echo Compiling $@
	$(CXX) $(CXXFLAGS) $< -o $@.out
	echo Compiled $@
