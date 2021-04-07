/*
variably optimized strings
*/

#include"vos.h"
using namespace ls;
#include<stdio.h>

int main(void)
{
  stack_buffer<10, int> s;
  printf("Size of vos<8>: %lu\n", sizeof(stack_buffer<8, char>) + 8);
  s[9] = 99;
  std::cout << s[9] << '\n';
  //s[10] = 0;

  heap_buffer<double> h(10);
  h[9] = 88;
  std::cout << h[9] << '\n';
  h.resize(h.size() * 2);
  h[10] = 0;
  h[19] = 0;

  vos<30> v("hello world how are you?");
  vos<10> v2 = "-200";
  std::cout << stoull(v2) << std::endl;
  v2 = std::move(v);
  vos<1> v3;
  v3 = "";

  for(auto c : v)
    std::cout << c << '\n';

  //std::cout << v.std_str() << std::endl;

  std::cout << "length of v: " << v.length() << " capacity: " << v.capacity() << '\n';
  std::cout << "length of v2: " << v2.length() << " capacity: " << v2.capacity() << '\n';
  std::cout << "length of v3: " << v3.length() << " capacity: " << v3.capacity() << '\n';



  std::cout << s.size() << std::endl;
}
