#pragma once

template <typename T> class StaticReference {
  T *target;

public:
  T *operator->() { return target; }
  T &operator*() { return *target; }
  void operator=(T *rhs) { target = rhs; }
};

template <typename T> extern StaticReference<T> refs;

#define MakeStaticReference(T) template <> StaticReference<T> refs<T>

template <typename T> extern T &Instance();

#define MakeSingleInstance(T, ...)                                                                                                                   \
  template <> T &Instance<T>() {                                                                                                                     \
    static T value{ __VA_ARGS__ };                                                                                                                   \
    return value;                                                                                                                                    \
  }
