/**
 * @author  liuziang
 * @contact liuziang@liuziangexit.com
 * @date    2/25/2019
 *
 * lazy
 *
 * A thread-safe wrapper class that provides lazy initialization semantics for
 * any type.
 *
 * requires C++17.
 *
 */
#pragma once
#ifndef _liuziangexit_lazy
#define _liuziangexit_lazy
#include <atomic>
#include <cstddef>
#include <exception>
#include <memory>
#include <mutex>
#include <tuple>
#include <type_traits>

namespace liuziangexit_lazy {

namespace detail {

template <std::size_t...> struct sequence {};

template <std::size_t _Size, std::size_t... _Sequence>
struct make_index_sequence
    : make_index_sequence<_Size - 1, _Size - 1, _Sequence...> {};

template <std::size_t... _Sequence>
struct make_index_sequence<0, _Sequence...> {
  using type = sequence<_Sequence...>;
};

template <typename _Func, typename _Tuple, std::size_t... index_sequence>
void do_call(const _Func &func, const _Tuple &tuple,
             sequence<index_sequence...>) {
  func(std::get<index_sequence>(tuple)...);
}

template <typename _Func, typename _Tuple>
void function_call(const _Func &func, const _Tuple &tuple) {
  do_call(func, tuple,
          typename make_index_sequence<std::tuple_size_v<_Tuple>>::type());
}

} // namespace detail

class construction_error : public std::exception {
  virtual const char *what() const noexcept override {
    return "constructor throws an exception";
  }
};

template <typename _Ty, typename _Alloc, typename... _ConstructorArgs>
class lazy {
public:
  using value_type =
      typename std::remove_reference_t<typename std::remove_cv_t<_Ty>>;
  using allocator_type = _Alloc;
  static_assert(
      std::is_same_v<value_type, typename std::allocator_traits<
                                     allocator_type>::value_type>,
      "lazy::value_type should equals to lazy::allocator_type::value_type");
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;

private:
  using constructor_args_tuple = std::tuple<_ConstructorArgs...>;

public:
  constexpr explicit lazy(const allocator_type &alloc,
                          const _ConstructorArgs &... args)
      : m_allocator(alloc), m_constructor_args(std::make_tuple(args...)),
        m_instance(nullptr) {}

  lazy(const lazy &) = delete;

  lazy(lazy &&rhs)
      : m_instance(rhs.m_instance.load(std::memory_order_relaxed)),
        m_allocator(std::move(rhs.m_allocator)),
        m_constructor_args(std::move(rhs.m_constructor_args)) {
    rhs.m_instance.store(nullptr, std::memory_order_relaxed);
  }

  lazy operator=(const lazy &) = delete;

  lazy operator=(lazy &&rhs) {
    this->~lazy();
    new (this) lazy(std::move(rhs));
  }

  ~lazy() noexcept {
    this->m_instance.load(std::memory_order_relaxed)->~value_type();
    this->m_allocator.deallocate(
        this->m_instance.load(std::memory_order_relaxed), 1);
  }

public:
  // get the lazily initialized value
  // if memory allocation fails, std::bad_alloc will be thrown
  // if constructor throws an exception, construction_error will be thrown
  value_type &get_instance() {
    if (!m_instance.load(std::memory_order::memory_order_acquire)) {
      {
        std::lock_guard<std::mutex> guard(m_lock);
        if (!m_instance.load(std::memory_order::memory_order_relaxed)) {
          // allocate memory
          value_type *new_instance = this->m_allocator.allocate(1);
          try {
            // invoke constructor
            detail::function_call(
                [new_instance](const auto &... args) {
                  new (new_instance) value_type(args...);
                },
                this->m_constructor_args);
          } catch (...) {
            this->m_allocator.deallocate(new_instance, 1);
            throw construction_error();
          }
          m_instance.store(new_instance,
                           std::memory_order::memory_order_release);
        }
      } // lock_guard
    }
    return *m_instance.load(std::memory_order::memory_order_relaxed);
  }

  // indicates whether a value has been created
  bool is_instance_created() {
    return m_instance.load(std::memory_order::memory_order_acquire);
  }

private:
  std::atomic<pointer> m_instance;
  allocator_type m_allocator;
  constructor_args_tuple m_constructor_args;
  std::mutex m_lock;
};

template <typename _Ty, typename... _ConstructorArgs>
auto make_lazy(const _ConstructorArgs &... constructor_args) {
  return lazy<_Ty, std::allocator<_Ty>, _ConstructorArgs...>(
      std::allocator<_Ty>(), constructor_args...);
}

} // namespace liuziangexit_lazy
#endif
