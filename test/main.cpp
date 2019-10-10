#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#include "../include/lazy.h"
#include <cstring>
#include <exception>
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

struct bomb {
  bomb() { throw std::runtime_error("naive"); }
};

static int *empty_count = nullptr;
static int *copy_count = nullptr;
static int *move_count = nullptr;

class test_forwarding {
public:
  test_forwarding() { (*empty_count)++; }

  test_forwarding(const test_forwarding &rhs) { (*copy_count)++; }

  test_forwarding(test_forwarding &&rhs) { (*move_count)++; }
};

class test_forwarding_holder {
public:
  test_forwarding_holder(test_forwarding &&a) : m(std::move(a)) {
    //(*move_count)++;
  }
  test_forwarding_holder(const test_forwarding &a) : m(a) {
    //(*copy_count)++;
  }
  test_forwarding m;
};

void test(bool b) {
  if (!b)
    throw std::exception();
}

int main() {
  {
    std::map<int, int> map;
    argclass argv(map, 1);

    // basic cases without ctor argument
    auto lazy1 = make_lazy<valueclass>();
    test(lazy1.is_instance_created() == false);
    test(lazy1.get_instance().arg_count == 0);
    test(lazy1.is_instance_created() == true);
    test(lazy1.get_instance().arg_count == 0);
    test(lazy1.is_instance_created() == true);
    // move ctor
    auto lazy2 = std::move(lazy1);
    test(lazy1.is_instance_created() == false);
    test(lazy2.is_instance_created() == true);
    test(lazy2.get_instance().arg_count == 0);
    test(map[1] == 0);
    //basic cases with ctor argument
    auto lazy3 = make_lazy<valueclass>(argv, argv);
    test(map[1] == 2);
    test(lazy3.is_instance_created() == false);
    test(lazy3.get_instance().arg_count == 2);
    test(lazy3.is_instance_created() == true);
    test(&lazy3.get_instance() == &lazy3.get_instance());
    //what if ctor throws an exception?
    //everything should be fine
    auto lazy4 = make_lazy<bomb>();
    try {
      lazy4.get_instance();
      test(false);
    } catch (const std::runtime_error &ex) {
      //is this the origin exception thrown by the ctor?
      test(strcmp(ex.what(), "naive") == 0);
    } catch (...) {
      //something wrong
      test(false);
    }

    //test ctor argument forwarding
    int empty = 0, copy = 0, move = 0;
    empty_count = &empty;
    copy_count = &copy;
    move_count = &move;
    test_forwarding aaaa;
    test(empty == 1);
    auto lazy5 =
        liuziangexit_lazy::make_lazy<test_forwarding_holder>(std::move(aaaa));
    test(empty == 1);
    test(move == 1);
    lazy5.get_instance();
    test(empty == 1);
    test(move == 2);
    test(copy == 0);
    copy = 0;
    move = 0;

    //test move operator
    lazy5 = liuziangexit_lazy::make_lazy<test_forwarding_holder>(aaaa);
    test(empty == 1);
    test(copy == 1);
    test(move == 1); // move element while moving the whole lazy object
    lazy5.get_instance();
    test(empty == 1);
    test(copy == 1);
    test(move == 2);

    std::cout << "GREAT SUCCESS!";
    // std::cin.get();
  }
#ifdef _WIN32
  _CrtDumpMemoryLeaks();
#endif
}
