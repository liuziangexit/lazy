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
 * Requires C++17.
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
#include <new>
#include <tuple>
#include <type_traits>

namespace liuziangexit_lazy {

namespace detail {

template <std::size_t...> struct sequence {};

template <std::size_t _Size, std::size_t... _Sequence>
struct make_integer_sequence
    : make_integer_sequence<_Size - 1, _Size - 1, _Sequence...> {};

template <std::size_t... _Sequence>
struct make_integer_sequence<0, _Sequence...> {
  using type = sequence<_Sequence...>;
};

template <typename _Func, typename _Tuple, std::size_t... index>
constexpr void do_call(const _Func &func, _Tuple &&tuple, sequence<index...>) {
  func(std::get<index>(std::forward<_Tuple>(tuple))...);
}

template <typename _Func, typename _Tuple>
constexpr void function_call(const _Func &func, _Tuple &&tuple) {
  do_call(func, std::forward<_Tuple>(tuple),
          typename make_integer_sequence<
              std::tuple_size_v<std::remove_reference_t<_Tuple>>>::type());
}

} // namespace detail

template <typename _Ty, typename _Alloc, typename... _ConstructorArgs>
class lazy {
public:
  using value_type =
      typename std::remove_reference_t<typename std::remove_cv_t<_Ty>>;
  using allocator_type = _Alloc;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;

  static_assert(
      std::is_same_v<
          value_type, //
          typename std::allocator_traits<allocator_type>::value_type>,
      "lazy::value_type should equals to lazy::allocator_type::value_type");

private:
  using constructor_arguments_tuple = std::tuple<_ConstructorArgs...>;

public:
  /*
   Construct lazy object.

   Use _DeductionTrigger(type parameter of templated constructor) rather than
   _ConstructorArgs(type parameter of template class) to trigger a deduction.

   This deduction makes variable 'args' becomes a forwarding reference but not
   a rvalue reference.
  */
  template <typename... _DeductionTrigger>
  constexpr explicit lazy(const allocator_type &alloc,
                          _DeductionTrigger &&...args)
      : m_instance(nullptr), //
        m_allocator(alloc),  //
        m_constructor_arguments(
            std::make_tuple(std::forward<_DeductionTrigger>(args)...)) {}

  lazy(const lazy &) = delete;

  lazy(lazy &&rhs) noexcept
      : m_instance(rhs.m_instance.load(std::memory_order_relaxed)),
        m_allocator(std::move(rhs.m_allocator)),
        m_constructor_arguments(std::move(rhs.m_constructor_arguments)) {
    rhs.m_instance.store(nullptr, std::memory_order_relaxed);
  }

  lazy &operator=(const lazy &) = delete;

  lazy &operator=(lazy &&rhs) noexcept {
    if (this == &rhs)
      return *this;
    this->~lazy();
    return *new (this) lazy(std::move(rhs));
  }

  ~lazy() noexcept {
    this->m_instance.load(std::memory_order_relaxed)->~value_type();
    this->m_allocator.deallocate(
        this->m_instance.load(std::memory_order_relaxed), 1);
  }

public:
  /*
   Get the lazily initialized value.

   This function is exception-safe. if the constructor throws an exception,
   that exception WILL rethrow and WILL NOT cause any leak or illegal state.

   If memory allocation fails or constructor throws std::bad_alloc,
   std::bad_alloc will be thrown.
  */
  value_type &get_instance() {
    value_type *instance =
        m_instance.load(std::memory_order::memory_order_acquire);
    if (!instance) {
      std::lock_guard<std::mutex> guard(m_lock);
      instance = m_instance.load(std::memory_order::memory_order_relaxed);
      if (!instance) {
        // allocate memory
        instance = this->m_allocator.allocate(1);
        try {
          // invoke constructor
          detail::function_call(
              [instance](auto &&...args) {
                new (instance) value_type( //
                    std::forward<decltype(args)>(args)...);
              },
              std::move(this->m_constructor_arguments));
        } catch (...) {
          this->m_allocator.deallocate(instance, 1);
          std::rethrow_exception(std::current_exception());
        }
        m_instance.store(instance, std::memory_order::memory_order_release);
      }
    }
#ifdef __cpp_lib_launder
    return *std::launder(instance);
#else
    return *instance;
#endif
  }

  // Indicates whether a value has been created.
  bool is_instance_created() {
    return m_instance.load(std::memory_order::memory_order_acquire);
  }

private:
  std::atomic<pointer> m_instance;
  allocator_type m_allocator;
  constructor_arguments_tuple m_constructor_arguments;
  std::mutex m_lock;
};

template <typename _Ty, typename... _ConstructorArgs>
auto make_lazy(_ConstructorArgs &&...constructor_args) {
  return lazy<_Ty, std::allocator<_Ty>,
              std::remove_reference_t<std::remove_cv_t<_ConstructorArgs>>...>(
      std::allocator<_Ty>(),
      std::forward<_ConstructorArgs>(constructor_args)...);
}

template <typename _Ty, typename... _ConstructorArgs>
using lazy_t =
    lazy<_Ty, std::allocator<_Ty>,
         std::remove_reference_t<std::remove_cv_t<_ConstructorArgs>>...>;

} // namespace liuziangexit_lazy
#endif
