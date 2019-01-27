#include <iostream>

template<int times, typename T>
void print_several_times(T it)
{
    for(int i=0;i<times;i++)
        std::cout << it << " ";
    std::cout << std::endl;
}

int main()
{
    print_several_times<5, int>(10);
    print_several_times<7, char>('a');
    //print_several_times(1.2); //Error: couldn't deduce template parameter ‘times’
    print_several_times<5>(1.2); // so how to achieve partial deduce? just lik this line?
    return 0;
}
