CXX=clang++
CXXFLAGS+= -I./eigen-git-mirror -I./eigen-git-mirror/unsupported -lblas -llapacke --std=c++11

ifeq ($(DEBUG), YES)
	CXXFLAGS+= -g
else
	CXXFLAGS+= -O3 -march=native 
endif

object = main mps_heisenburg

default: main

$(object): %: %.cpp
	echo Compiling $@
	$(CXX) $(CXXFLAGS) $< -o $@.out
	echo Compiled $@
