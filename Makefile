CXXFLAGS= -I./eigen-git-mirror -I./eigen-git-mirror/unsupported -g -llapacke -lblas
gcc:
	g++ main.cpp $(CXXFLAGS)
	./a.out

icc:
	icc main.cpp $(CXXFLAGS)
	./a.out

clang:
	clang++ main.cpp $(CXXFLAGS)
	./a.out
