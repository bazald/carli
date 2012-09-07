#include "memory_pool.h"
#include "linked_list.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <list>

#ifdef WIN32
#include <Windows.h>
#endif

class Class {
public:
  Class(const int &i_ = 0)
    : i(i_)
  {
  }

  virtual ~Class() {}

  int i;
};

class Subclass : public Class {
public:
  Subclass() {}

private:
  char buf[64];
};

class Class_Derived;
class Class_Base : public Zeni::Pool_Allocator<Class_Derived> {
  Class_Base(const Class_Base &rhs);
  Class_Base & operator=(const Class_Base &rhs);

public:
  typedef Zeni::Linked_List<Class_Base> List;
  typedef List::iterator iterator;

  Class_Base(const int &i_ = 0)
    : list(this),
    i(i_)
  {
  }

  virtual ~Class_Base() {}

  Zeni::Linked_List<Class_Base> list;

  int i;
};

class Class_Derived : public Class_Base {
  Class_Derived(const Class_Derived &rhs);
  Class_Derived & operator=(const Class_Derived &rhs);

public:
  Class_Derived(const int &i_)
    : Class_Base(i_)
  {
  }

private:
  char buf[64];
};

int main(int argc, char **argv) {
  Zeni::register_new_handler();

#ifdef WIN32
  DWORD count = GetTickCount();
#endif

//  if(argc < 2 || !strcmp(argv[1], "phd")) {
//    for(int i = 0; i != 0x1000000; ++i)
//      std::unique_ptr<Class_Base> ptr(new Class_Derived);
//  }
//  else if(!strcmp(argv[1], "phb")) {
//    for(int i = 0; i != 0x1000000; ++i)
//      std::unique_ptr<Class_Base> ptr(new Class_Derived);
//  }
//  else if(!strcmp(argv[1], "psd")) {
//    for(int i = 0; i != 0x1000000; ++i)
//      Class_Derived value;
//  }
//  else if(!strcmp(argv[1], "psb")) {
//    for(int i = 0; i != 0x1000000; ++i)
//      Class_Base value;
//  }
//  else if(!strcmp(argv[1], "shd")) {
//    for(int i = 0; i != 0x1000000; ++i)
//      std::unique_ptr<Class> ptr(new Subclass);
//  }
//  else if(!strcmp(argv[1], "shb")) {
//    for(int i = 0; i != 0x1000000; ++i)
//      std::unique_ptr<Class> ptr(new Class);
//  }
//  else if(!strcmp(argv[1], "ssd")) {
//    for(int i = 0; i != 0x1000000; ++i)
//      Subclass value;
//  }
//  else if(!strcmp(argv[1], "ssb")) {
//    for(int i = 0; i != 0x1000000; ++i)
//      Class value;
//  }
//
//  std::cout << "sizeof(Class) = " << sizeof(Class) << std::endl;
//  std::cout << "sizeof(Subclass) = " << sizeof(Subclass) << std::endl;
//  std::cout << "sizeof(Class_Base) = " << sizeof(Class_Base) << std::endl;
//  std::cout << "sizeof(Class_Derived) = " << sizeof(Class_Derived) << std::endl;

  //for(int i = 0; i != 0x10000; ++i) {
  //  std::list<Class> list;
  //  for(int i = 0; i != 100; ++i)
  //    list.push_back(Class(i));
  //}

  for(int i = 0; i != 0x10000; ++i) {
    Class_Base::List * const head(&(new Class_Derived(0))->list);
    Class_Base::List * tail(head);
    for(int i = 1; i != 100; ++i, tail = tail->next())
      (new Class_Derived(i))->list.insert_after(tail);

    //for(Class_Base::iterator it = head->begin(), iend = head->end(); it != iend; ++it)
    //  std::cerr << it->i << std::endl;

    for(Class_Base::iterator it = head->begin(), iend = head->end(); it != iend; )
      delete it++.get();
  }

#ifdef WIN32
  std::cout << "\nInterval = " << GetTickCount()-count << " ms\n" << std::endl;
#endif

  return 0;
}
