/*#include <iostream>

class Base
{
public:
  int data;
  Base(int a, int b)
  {
    data = a+b;
  }
};

class Son : public Base
{
public:
  Son(int a, int b){};
  void print()
  {
    std::cout << data << "\n";
  }
};

int main()
{
  Son A(1,2);
  A.print();
}*/

#include<iostream> 
using namespace std; 
class Person { 
   // Data members of person  
public: 
    Person(int x)  { cout << "Person::Person(int ) called with " << x << endl;   } 
}; 

class Student : public Person { 
   // data members of Student 
public: 
    Student(int x):Person(x) {  
        cout<<"Student::Student(int ) called with"<< x << endl; 
    } 
}; 

int main()  { 
    Student ta1(30); 
} 