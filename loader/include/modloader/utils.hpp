#pragma once

#include <aux.h>
#include <cstddef>

template <typename F> struct guard {
  F f;

  inline ~guard() { f(); }
};

template <typename F> guard(F f)->guard<F>;

// clang-format off
#define GUARD(name, ...) guard name##_guard { [&] { __VA_ARGS__; } }
// clang-format on

template <typename F> struct lazy {
  inline lazy(F f) { f(); }
};

template <typename F> lazy(F f)->lazy<F>;

// clang-format off
#define LAZY(name, ...) static lazy name##_lazy { [] { __VA_ARGS__; } }
// clang-format on

template <typename R, typename T> static R union_cast(T hook) {
  union {
    T a;
    R b;
  } temp;
  temp.a = hook;
  return temp.b;
}

template <typename R, size_t offset, typename T> static R &fetch(T *self) { return *union_cast<R *>(union_cast<size_t>(self) + offset); }

void print_stacktrace(unsigned int max_frames = 63);