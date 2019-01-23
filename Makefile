gcc:
	g++ main.cpp -I./eigen-git-mirror/unsupported -g
	./a.out

icc:
	icc main.cpp -I./eigen-git-mirror/unsupported -g
	./a.out

clang:
	clang++ main.cpp -I./eigen-git-mirror/unsupported -g
	./a.out
