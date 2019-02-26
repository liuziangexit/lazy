#pragma once
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#include "../include/lazy.h"
#include <iostream>
#include <map>
using namespace liuziangexit_lazy;

struct argclass {
  argclass(std::map<int, int> &map, int id) : m_map(map), m_id(id) {
    m_map[m_id] = 0;
  }
  argclass(const argclass &rhs) : m_id(rhs.m_id), m_map(rhs.m_map) {
    int &current = m_map[m_id];
    current++;
  }
  ~argclass() {}

  int m_id;
  std::map<int, int> &m_map;
};

struct valueclass {
  valueclass() : arg_count(0) {}
  valueclass(const argclass &) : arg_count(1) {}
  valueclass(const argclass &, const argclass &) : arg_count(2) {}
  int arg_count;
};

void case_assert(bool v) {
  if (!v) {
    abort();
  }
}

int main() {
  {
    std::map<int, int> map;
    argclass arg(map, 1);

    auto lazy1 = make_lazy<valueclass>();
    case_assert(lazy1.is_instance_created() == false);
    case_assert(lazy1.get_instance().arg_count == 0);
    case_assert(lazy1.is_instance_created() == true);
    case_assert(lazy1.get_instance().arg_count == 0);
    case_assert(lazy1.is_instance_created() == true);
    auto lazy2 = std::move(lazy1);
    case_assert(lazy1.is_instance_created() == false);
    case_assert(lazy2.is_instance_created() == true);
    case_assert(lazy2.get_instance().arg_count == 0);
    case_assert(map[1] == 0);
    auto lazy3 = make_lazy<valueclass>(arg, arg);
    case_assert(map[1] == 2);
    case_assert(lazy3.is_instance_created() == false);
    case_assert(lazy3.get_instance().arg_count == 2);
    case_assert(lazy3.is_instance_created() == true);

    std::cout << "GREAT SUCCESS!";
    // std::cin.get();
  }
#ifdef _WIN32
  _CrtDumpMemoryLeaks();
#endif
}
