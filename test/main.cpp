#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#include "../include/lazy.h"
#include <cassert>
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

int main() {
  {
    std::map<int, int> map;
    argclass argv(map, 1);

    auto lazy1 = make_lazy<valueclass>();
    assert(lazy1.is_instance_created() == false);
    assert(lazy1.get_instance().arg_count == 0);
    assert(lazy1.is_instance_created() == true);
    assert(lazy1.get_instance().arg_count == 0);
    assert(lazy1.is_instance_created() == true);
    auto lazy2 = std::move(lazy1);
    assert(lazy1.is_instance_created() == false);
    assert(lazy2.is_instance_created() == true);
    assert(lazy2.get_instance().arg_count == 0);
    assert(map[1] == 0);
    auto lazy3 = make_lazy<valueclass>(argv, argv);
    assert(map[1] == 2);
    assert(lazy3.is_instance_created() == false);
    assert(lazy3.get_instance().arg_count == 2);
    assert(lazy3.is_instance_created() == true);
    assert(&lazy3.get_instance() == &lazy3.get_instance());
    auto lazy4 = make_lazy<bomb>();
    try {
      lazy4.get_instance();
      assert(false);
    } catch (const std::runtime_error &ex) {
      assert(strcmp(ex.what(), "naive") == 0);
    } catch (...) {
      assert(false);
    }

    int empty = 0, copy = 0, move = 0;
    empty_count = &empty;
    copy_count = &copy;
    move_count = &move;
    test_forwarding aaaa;
    assert(empty == 1);
    auto lazy5 =
        liuziangexit_lazy::make_lazy<test_forwarding_holder>(std::move(aaaa));
    assert(empty == 1);
    assert(move == 1);
    lazy5.get_instance();
    assert(empty == 1);
    assert(move == 2);
    assert(copy == 0);
    copy = 0;
    move = 0;

    lazy5 = liuziangexit_lazy::make_lazy<test_forwarding_holder>(aaaa);
    assert(empty == 1);
    assert(copy == 1);
    assert(move == 1); // move element while moving the whole lazy object
    lazy5.get_instance();
    assert(empty == 1);
    assert(copy == 1);
    assert(move == 2);

    std::cout << "GREAT SUCCESS!";
    // std::cin.get();
  }
#ifdef _WIN32
  _CrtDumpMemoryLeaks();
#endif
}
