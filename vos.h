#ifndef _H_VOS
#define _H_VOS


#include<cstdlib>
#include<iostream>
#include<stdexcept>
#include<variant>
#include<string.h>
#include<initializer_list>
#include<cmath>
#include<climits>
#include<iostream>

namespace ls
{

template<std::size_t i, typename T>
class stack_buffer
{
  T internal_data[i];
  std::size_t internal_size = i;

public:
  stack_buffer()
  {
    //Nothing to do here...
    std::cout << "stack_buffer created.\n";
  }
  T * data()
  {
    return internal_data;
  }

  const T * const_data() const
  {
    return internal_data;
  }

  std::size_t size() const
  {
    return internal_size;
  }

  T& operator[](std::size_t index)
  {
    if (index >= internal_size)
    {
        throw std::out_of_range("Out of range index on stack_buffer object.");
    }
    return internal_data[index];
  }

  ~stack_buffer()
  {
    //Nothing to do here...
    std::cout << "stack_buffer destroyed.\n";
  }
};

template<typename T>
class heap_buffer
{
  T * internal_data;
  std::size_t internal_size;

public:
  heap_buffer(std::size_t size)
  {
    if(size == 0)
    {
      internal_size = 0;
      internal_data = NULL;
      return;
    }
    internal_size = size;
    internal_data = (T *) malloc(sizeof(T) * internal_size);
    if(internal_data == NULL)
    {
      throw std::bad_alloc();
    }
    std::cout << "heap_buffer created.\n";
  }
  ~heap_buffer()
  {
    //Free the data!
    if(internal_data)
      free(internal_data);
    std::cout << "heap_buffer destroyed.\n";
  }

  void resize(std::size_t size)
  {
    if(size <= internal_size)
    {
      //Shrink!
      T * temp = internal_data;
      internal_data = (T *) malloc(sizeof(T) * size);
      if(internal_data == NULL)
      {
        internal_data = temp;
        throw std::bad_alloc();
      }
      else
      {
        internal_size = size;
        memcpy(internal_data, temp, sizeof(T) * size);
        free(temp);
      }
      return;
    }
    T * temp;
    temp = (T *) realloc(internal_data, sizeof(T) * size);
    if(temp == NULL)
    {
      throw std::bad_alloc();
    }
    else
    {
      internal_data = temp;
      internal_size = size;
    }
  }

  T * data()
  {
    return internal_data;
  }

  const T * const_data() const
  {
    return internal_data;
  }

  std::size_t size() const
  {
    return internal_size;
  }

  T& operator[](std::size_t index)
  {
    if (index >= internal_size)
    {
        throw std::out_of_range("Out of range index on heap_buffer object.");
    }
    return internal_data[index];
  }

};

template<std::size_t i>
class vos
{
  std::variant<stack_buffer<i, char>, heap_buffer<char>> buffer;
  std::size_t cur_len;

  template <std::size_t U >
  friend class vos;

  void construct_content(const char * s)
  {
    cur_len = strlen(s);
    if(cur_len >= i)
    {
      //Have to assign a heap buffer (the buffer controls its own memory for the most part)
      buffer.template emplace<heap_buffer<char>>((cur_len + 1));
      strcpy(std::get<heap_buffer<char>>(buffer).data(), s);
    }
    else
    {
      //We can use a stack_buffer here
      buffer.template emplace<stack_buffer<i, char>>();
      strcpy(std::get<stack_buffer<i, char>>(buffer).data(), s);
    }
  }

  void assign_content(const char * s)
  {
    //We may need to resize, then just copy s to the potentially resized buffer
    cur_len = strlen(s);
    try_resize(strlen(s) + 1);
    if(auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
    {
      //Copy the new content to the buffer
      strcpy(ptr->data(), s);

    }
    else if(auto ptr = std::get_if<heap_buffer<char>>(&buffer))
    {
      //Copy the new content to the buffer
      strcpy(ptr->data(), s);
    }
  }

  void try_resize(std::size_t new_size)
  {
    if(new_size <= capacity())
      return;

    //We ideally want to resize with a min of 2 times the previos capacity
    if(new_size < capacity() << 1)
      new_size = capacity() << 1;

    //We have to necessarily put the data on the heap...
    //if it is currently on the stack, copy the data and then put it on the heap
    if(auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
    {
      char temp[i];
      strcpy(temp, ptr->data());
      buffer.template emplace<heap_buffer<char>>(new_size);
      strcpy(std::get<heap_buffer<char>>(buffer).data(), temp);
    }
    else if(auto ptr = std::get_if<heap_buffer<char>>(&buffer))
    {
      //A heap buffer already exists, we just need to resize it.
      ptr->resize(new_size);
    }
  }

  void try_shrink(std::size_t new_size)
  {
    if(new_size >= capacity())
      return;

    //If already on the stack, can shrink to stack
    //If on heap, may need to shrink to stack

    if(auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
    {
      //Basically just insert a null term and update the length, dont do any realloc
      cur_len = new_size;
      ptr->data()[cur_len] = 0;
    }
    else if(auto ptr = std::get_if<heap_buffer<char>>(&buffer))
    {
      std::cout << "!!" << '\n';
      //If it can fit on the stack, move it there
      if(new_size < i)
      {
        char temp[i];
        std::cout << ptr->data() << std::endl;
        strncpy(temp, ptr->data(), new_size);
        buffer.template emplace<stack_buffer<i, char>>();
        strncpy(std::get<stack_buffer<i, char>>(buffer).data(), temp, new_size);
        cur_len = new_size;
        std::get<stack_buffer<i, char>>(buffer).data()[cur_len - 1] = 0;
      }
      else   //It needs to stay on the heap
      {
        //Modify the internal buffer a bit to force a shrink...
        ptr->resize(new_size);
        cur_len = new_size;
        ptr->data()[cur_len - 1] = 0;
      }
    }
  }

public:
  vos(const char * s)
  {
    construct_content(s);
  }

  //default constructor, empty string
  vos()
  {
    construct_content("");
  }

  struct iterator
  {
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = char;
    using pointer           = char*;
    using reference         = char&;

    iterator(pointer ptr) : m_ptr(ptr) {}

    reference operator*() const { return *m_ptr; }
    pointer operator->() { return m_ptr; }

    // Prefix increment
    iterator& operator++() { m_ptr++; return *this; }

    // Postfix increment
    iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

    friend bool operator== (const iterator& a, const iterator& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const iterator& a, const iterator& b) { return a.m_ptr != b.m_ptr; };

  private:
    pointer m_ptr;
  };

  std::size_t length() const
  {
    return cur_len;
  }

  std::size_t size() const
  {
    return cur_len;
  }

  std::size_t capacity() const
  {
    std::size_t ret;
    if(auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
      ret = ptr->size();
    else if(auto ptr = std::get_if<heap_buffer<char>>(&buffer))
      ret = ptr->size();
    return ret;
  }

  void shrink_to_fit()
  {
    //Attempt to shrink...
    try_shrink(cur_len + 1);
  }

  void clear()
  {
    assign_content("");
  }

  bool empty() const
  {
    return (!cur_len);
  }

  char & front()
  {
    return (*this)[0];
  }

  template <std::size_t j = i>
  vos<j> substr(std::size_t pos, std::size_t len)
  {
    if(pos >= len)
    {
      throw std::invalid_argument("substr in vos object.");
    }
    if(pos + len >= cur_len)
    {
      throw std::out_of_range("substr in vos object.");
    }

    vos<j> ret;
    //resize the return to fit the substring
    ret.try_resize(len + 1);
    //Copy the current string buffer with n characters
    if(const auto ptr = std::get_if<stack_buffer<j, char>>(&ret.buffer))
    {
      std::size_t z;
      for(z = 0; z < len; z++)
        ptr->data()[z] = (*this)[pos + z];
      //null term
      ptr->data()[z] = 0;
    }
    else if(const auto ptr = std::get_if<heap_buffer<char>>(&ret.buffer))
    {
      std::size_t z;
      for(z = 0; z < len; z++)
        ptr->data()[z] = (*this)[pos + z];
      //null term
      ptr->data()[z] = 0;
    }

    //put the correct length
    ret.cur_len = len;

    return ret;
  }


  char & back()
  {
    return (*this)[cur_len - 1];
  }

  const char * c_str() const
  {
    if(const auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
      return ptr->const_data();
    else if(const auto ptr = std::get_if<heap_buffer<char>>(&buffer))
      return ptr->const_data();
    return NULL;
  }

  //push_back and append

  void push_back(char c)
  {
    //We need to attempt a resize if needed...
    //We use two to account for the null term and then the extra char pushed back
    try_resize(cur_len + 2);

    //Now we just insert the char, update the length, and put a new null terminator
    if(const auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
    {
      ptr->data()[cur_len] = c;
      ptr->data()[cur_len + 1] = 0;
    }
    else if(const auto ptr = std::get_if<heap_buffer<char>>(&buffer))
    {
      ptr->data()[cur_len] = c;
      ptr->data()[cur_len + 1] = 0;
    }
    cur_len++;
  }

  void append(const char * str)
  {
    //First we need to try a resize with the new length
    std::size_t appended_str_len = strlen(str);
    //resize the underlying buffer if needed
    try_resize(this->capacity() + appended_str_len);

    if(const auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
    {
      strncpy(ptr->data() + cur_len, str, appended_str_len + 1);
    }
    else if(const auto ptr = std::get_if<heap_buffer<char>>(&buffer))
    {
      strncpy(ptr->data() + cur_len, str, appended_str_len + 1);
    }
    cur_len += appended_str_len;
  }

  //justi include this for ease of use but basically just push_back
  void append(char c)
  {
    push_back(c);
  }

  template<std::size_t j>
  void append(const vos<j> & str)
  {
    append(str.c_str());
  }

  //for use with std string
  void append(const std::string & str)
  {
    append(str.c_str());
  }

  //Operators

  //access operator

  char& operator[](std::size_t index)
  {
    if (index >= cur_len)
    {
        throw std::out_of_range("Out of range index on vos object.");
    }

    if(auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
      return (*ptr)[index];
    auto ptr = std::get_if<heap_buffer<char>>(&buffer);
    return (*ptr)[index];
  }

  //Addition (string concatenation)
  template <std::size_t j>
  friend vos<i> operator+ (const vos<i>& lhs, const vos<j>& rhs)
  {
    //the return vos
    vos<i> ret;
    ret.try_resize(lhs.length() + rhs.length() + 1);

    if(const auto ptr = std::get_if<stack_buffer<i, char>>(&ret.buffer))
    {
      strncpy(ptr->data(), lhs.c_str(), lhs.length());
      strncpy(ptr->data() + lhs.length(), rhs.c_str(), rhs.length() + 1);
    }
    else if(const auto ptr = std::get_if<heap_buffer<char>>(&ret.buffer))
    {
      strncpy(ptr->data(), lhs.c_str(), lhs.length());
      strncpy(ptr->data() + lhs.length(), rhs.c_str(), rhs.length() + 1);
    }
    ret.cur_len = lhs.length() + rhs.length();

    return ret;
  }

  //Ostream insert Operator
  friend std::ostream& operator<< (std::ostream& os, const vos<i>& str)
  {
    os << str.c_str();
    return os;
  }

  friend std::istream& operator>> (std::istream& is, vos<i>& str)
  {
    //Just delim based on any whitespace and put it into the string.
    char c;
    str.clear();
    do
    {
      c = is.get();
      if(!isspace(c))
      {
        str.push_back(c);
      }
    }
    while(!isspace(c) && is.good());

    return is;
  }

  //+= operator (same as append())
  vos<i>& operator+= (const char * str)
  {
    append(str);
    return *this;
  }
  vos<i>& operator+= (char c)
  {
    push_back(c);
    return *this;
  }
  template<std::size_t j>
  vos<i>& operator+= (const vos<j> & str)
  {
    append(str);
    return *this;
  }
  vos<i>& operator+= (const std::string & str)
  {
    append(str);
    return *this;
  }

  //Assign a c string
  vos<i>& operator= (const char * str)
  {
    this->assign_content(str);
    return *this;
  }

  //Assign another vos (copy)
  template <std::size_t j>
  vos<i>& operator= (const vos<j>& str)
  {
    this->assign_content(str.c_str());
    return *this;
  }

  //Assign a char
  vos<i>& operator= (char c)
  {
    char temp[2];
    temp[0] = c;
    temp[1] = 0;

    this->assign_content(temp);

    return *this;
  }

  //initializer_list
  vos<i>& operator= (std::initializer_list<char> il)
  {
    try_resize(il.size() + 1);
    std::cout << il.size();

    if(const auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
    {
      strncpy(ptr->data(), il.begin(), il.size());
      ptr->data()[il.size()] = 0;
    }
    else if(const auto ptr = std::get_if<heap_buffer<char>>(&buffer))
    {
      strncpy(ptr->data(), il.begin(), il.size());
      ptr->data()[il.size()] = 0;
    }
    cur_len = il.size();
    return *this;
  }

  //std::string
  vos<i>& operator= (const std::string & str)
  {
    this->assign_content(str.c_str());
    return *this;
  }

  //iterators

  iterator begin()
  {
    if(auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
      return iterator(ptr->data());
    else if(auto ptr = std::get_if<heap_buffer<char>>(&buffer))
      return iterator(ptr->data());

    return NULL;
  }

  iterator end()
  {
    if(auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
      return iterator(ptr->data() + cur_len );
    else if(auto ptr = std::get_if<heap_buffer<char>>(&buffer))
      return iterator(ptr->data() + cur_len );

    return NULL;
  }

  std::string std_str()
  {
    std::string s(this->c_str());
    return s;
  }

  ~vos()
  {

  }
};

// Common string functions, like stod, etc.
//It's safe to access .c_str()[0] as it must contain altleast one char (null-terminated)

template<std::size_t i>
double stod(const vos<i> & str)
{
  double ret;
  ret = strtod(str.c_str(), NULL);

  //unable to perform conversion
  if(ret == 0.0 && str.c_str()[0] != '0')
    throw std::invalid_argument("stod with a vos object.");
  else if(ret == HUGE_VAL || ret == -HUGE_VAL)
    throw std::out_of_range("stod with a vos object.");

  return ret;
}

template<std::size_t i>
float stof(const vos<i> & str)
{
  float ret;
  ret = strtof(str.c_str(), NULL);

  //unable to perform conversion
  if(ret == 0.0 && str.c_str()[0] != '0')
    throw std::invalid_argument("stof with a vos object.");
  else if(ret == HUGE_VALF || ret == -HUGE_VALF)
    throw std::out_of_range("stof with a vos object.");

  return ret;
}

template<std::size_t i>
int stoi(const vos<i> & str, int base = 10)
{
  int ret;
  ret = strtol(str.c_str(), NULL, base);

  //unable to perform conversion
  if(ret == 0 && str.c_str()[0] != '0')
    throw std::invalid_argument("stoi with a vos object.");
  else if(ret == LONG_MAX || ret == LONG_MIN)
    throw std::out_of_range("stoi with a vos object");

  return ret;
}

template<std::size_t i>
long stol(const vos<i> & str, int base = 10)
{
  long ret;
  ret = strtol(str.c_str(), NULL, base);

  //unable to perform conversion
  if(ret == 0 && str.c_str()[0] != '0')
    throw std::invalid_argument("stol with a vos object.");
  else if(ret == LONG_MAX || ret == LONG_MIN)
    throw std::out_of_range("stol with a vos object");

  return ret;
}

template<std::size_t i>
long long stoll(const vos<i> & str, int base = 10)
{
  long long ret;
  ret = strtoll(str.c_str(), NULL, base);

  //unable to perform conversion
  if(ret == 0 && str.c_str()[0] != '0')
    throw std::invalid_argument("stoll with a vos object.");
  else if(ret == LLONG_MAX || ret == LLONG_MIN)
    throw std::out_of_range("stoll with a vos object");

  return ret;
}

template<std::size_t i>
unsigned long stoul(const vos<i> & str, int base = 10)
{
  unsigned long ret;
  ret = strtoul(str.c_str(), NULL, base);

  //unable to perform conversion
  if(ret == 0 && str.c_str()[0] != '0')
    throw std::invalid_argument("stoll with a vos object.");
  else if(ret == ULONG_MAX)
    throw std::out_of_range("stoll with a vos object");

  return ret;
}

template<std::size_t i>
unsigned long long stoull(const vos<i> & str, int base = 10)
{
  unsigned long long ret;
  ret = strtoull(str.c_str(), NULL, base);

  //unable to perform conversion
  if(ret == 0 && str.c_str()[0] != '0')
    throw std::invalid_argument("stoll with a vos object.");
  else if(ret == ULLONG_MAX)
    throw std::out_of_range("stoll with a vos object");

  return ret;
}

// getline overloads!
template<std::size_t i>
std::istream& getline (std::istream&  is, vos<i>& str, char delim)
{
  char c;
  str.clear();
  do
  {
    c = is.get();
    if(c != delim)
    {
      str.push_back(c);
    }
  }
  while(c != delim && is.good());

  return is;
}

template<std::size_t i>
std::istream& getline (std::istream&& is, vos<i>& str, char delim)
{
  char c;
  str.clear();
  do
  {
    c = is.get();
    if(c != delim)
    {
      str.push_back(c);
    }
  }
  while(c != delim && is.good());

  return is;
}

template<std::size_t i>
std::istream& getline (std::istream&  is, vos<i>& str)
{
  char c;
  char delim = '\n';
  str.clear();
  do
  {
    c = is.get();
    if(c != delim)
    {
      str.push_back(c);
    }
  }
  while(c != delim && is.good());

  return is;
}

template<std::size_t i>
std::istream& getline (std::istream&& is, vos<i>& str)
{
  char c;
  char delim = '\n';
  str.clear();
  do
  {
    c = is.get();
    if(c != delim)
    {
      str.push_back(c);
    }
  }
  while(c != delim && is.good());

  return is;
}

// Some typedef for some types
typedef   vos<8>    string8;
typedef   vos<16>   string16;
typedef   vos<32>   string32;
typedef   vos<64>   string64;
typedef   vos<128>  string128;

}
#endif
