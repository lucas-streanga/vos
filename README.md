# vos
Variably Optimized Strings for C++17 and on.

VOS is a complete reimplementation of std::string with support for templated short-string optimaztion size.

# Features
  - Support for standard string member functions such as ```push_back()```, ```append()```, ```length()```, ```substr()```, etc.
  - Support for standard string operators such as ```[]```, ```+=```, ```+```, etc.
  - Support for iostream operators insertion and extraction operators (```>>``` and ```<<```)
  - Support for all overloads of ```getline()```
  - Support for ```stoi()```, ```stod()```, ```stoll()```, etc.
  - Support for random-access iterators allowing for range-based for loops
  - Support for conversions between ```vos```, ```std::string```, and ```char *```

# Usage
A VOS may be declared with ```ls::vos<SIZE> var_name;```. The size passed in will be immediately allocated to the stack. If a size of 0 is passed in,
the VOS will always be heap allocated. The VOS will stay stack allocated until the content is too large to fit into the specified size. In such a case,
the contents will be copied to the heap and the stack buffer destroyed. 

## Implementation Notes
  - A stack buffer and a heap buffer will never exist at the same time and therefore a VOS will only need space for one at any given time.
  - sizeof(stack_buffer<i, type>) is always i * sizeof(type), i.e. equivilant to a simple array. 
  - sizeof(heap_buffer<char>) is 16 on 64-bit systems. 
  - Unlike std::string, a null terminator (ascii 0) is always stored in the underlying buffer. 
  - Memory will not be deallocated or shrunk from the heap unless there is an explicit call to ```shrink_to_fit()```. 
  - ```shrink_to_fit()``` will move content back to the stack if the content is small enough to fit. 
  - Resizing the underlying buffer atleast doubles the underlying buffer (growth factor of 2x)
  - On 64-bit systems, sizeof(vos<16>) is equal to sizeof(vos<15>), meaning the minimum size for a vos should be 16 to avoid wasting space. 
