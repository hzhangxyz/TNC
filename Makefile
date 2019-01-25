CXX=clang++
CXXFLAGS+= -I./eigen-git-mirror -I./eigen-git-mirror/unsupported -g -lblas -llapacke

object = main mps_heisenburg

default: main

$(object): %: %.cpp
	echo Compiling $@
	$(CXX) $(CXXFLAGS) $< -o $@.out
	echo Compiled $@
