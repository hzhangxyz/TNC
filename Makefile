CXX=clang++
CXXFLAGS+= -I./eigen-git-mirror -I./eigen-git-mirror/unsupported -lblas -llapacke --std=c++11

DEBUG?=0
ifeq ($(DEBUG), 0)
	CXXFLAGS+= -O3 -march=native
else
	CXXFLAGS+= -g
endif

object = main mps_heisenburg

default: main

$(object): %: %.cpp
	echo Compiling $@
	$(CXX) $(CXXFLAGS) $< -o $@.out
	echo Compiled $@
