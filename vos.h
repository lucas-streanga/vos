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

namespace ls
{

template<std::size_t i, typename T>
class stack_buffer
{
  T internal_data[i];
  std::size_t internal_size = i;

public:
  T * data()
  {
    return internal_data;
  }

  const T * const_data() const
  {
    return internal_data;
  }

  std::size_t size()
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
  }
  ~heap_buffer()
  {
    //Free the data!
    if(internal_data)
      free(internal_data);
  }

  void resize(std::size_t size)
  {
    if(size <= internal_size)
      return;
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

  std::size_t size()
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
      //We can use a stack_buffer here!
      buffer.template emplace<stack_buffer<i, char>>();
      strcpy(std::get<stack_buffer<i, char>>(buffer).data(), s);
    }
  }

  void assign_content(const char * s)
  {
    //We may need to resize, then just copy s to the potentially resized buffer
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

  std::size_t length()
  {
    return cur_len;
  }

  std::size_t size()
  {
    return cur_len;
  }

  std::size_t capacity()
  {
    std::size_t ret;
    if(auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
      ret = ptr->size();
    else if(auto ptr = std::get_if<heap_buffer<char>>(&buffer))
      ret = ptr->size();
    return ret;
  }

  const char * c_str() const
  {
    if(const auto ptr = std::get_if<stack_buffer<i, char>>(&buffer))
      return ptr->const_data();
    else if(const auto ptr = std::get_if<heap_buffer<char>>(&buffer))
      return ptr->const_data();
    return NULL;
  }

  //Operators

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
    char temp[il.size() + 1];
    strncpy(temp, il.begin(), il.size());
    temp[il.size()] = 0;
    this->assign_content(temp);

    return *this;
  }

  //std::string
  vos<i>& operator= (const std::string & str)
  {
    this->assign_content(str.c_str());
    return *this;
  }

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
      return iterator(ptr->data() + ptr->size() );
    else if(auto ptr = std::get_if<heap_buffer<char>>(&buffer))
      return iterator(ptr->data() + ptr->size() );

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

}
#endif
