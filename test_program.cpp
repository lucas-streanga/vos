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

  string32 v;
  v = {'h', 'e'};
  std::cout << v << std::endl;
  v = "hello world how are you?";
  vos<10> v2 = "A new test";
  for(int i = 0; i < 3; i++)
    v2.push_back('!');
  v2.append(v);
  v2 += "__e__";
  std::string sss("***");
  v2 += sss;
  v2 += v;
  std::cout << (v + v2) << std::endl;
  v2 = v;
  v2 = "kk";
  //std::cout << stoull(v2) << std::endl;
  //v2 = std::move(v);
  vos<1> v3;
  vos<5> v4 = v.substr<5>(0, 5);
  std::cout << v4.c_str() << "!!\n";

  for(int i = 0; i < v2.size(); i++)
  {
    std::cout << v2[i] << '\n';
  }

  std::cout << ".";

  //std::cout << v.std_str() << std::endl;

  std::cout << "length of v: " << v.length() << " capacity: " << v.capacity() << '\n';
  std::cout << "length of v2: " << v2.length() << " capacity: " << v2.capacity() << '\n';
  std::cout << "length of v3: " << v3.length() << " capacity: " << v3.capacity() << '\n';



  std::cout << s.size() << std::endl;

  std::cout << v.capacity() << '\n';


  //getline(std::cin, v);
  std::cin >> v;
  std::cout << v << std::endl;
  std::cout << v.capacity() << '\n';
  std::cout << "Size of V2: " << v2.capacity() << '\n';
  v2 = "hello world!";
  v2.shrink_to_fit();
  std::cout << "Size of V2: " << v2.capacity() << '\n';
  std::cout << v2 << '\n';

}
