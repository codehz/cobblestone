#pragma once

#include <functional>
#include <sys/mman.h>

template <typename R, typename T> struct FunctionWrapper {
  unsigned char asm0[7] = { 0x41, 0x54, 0x49, 0x89, 0xfc, 0x48, 0xbe };
  T *payload;
  unsigned char asm1[2] = { 0x48, 0xb8 };
  R (*ptr)(T *);
  unsigned char asm2[8] = { 0xff, 0xd0, 0x4c, 0x89, 0xe0, 0x41, 0x5c, 0xc3 };

  FunctionWrapper(T *payload, R (*ptr)(T *))
      : payload(payload)
      , ptr(ptr) {}

} __attribute__((packed));

template <typename T, size_t unit = 100> void *alloc_executable_memory() {
  static T *stop = nullptr;
  static T *cur  = nullptr;
  if (stop == cur) {
    cur = (T *)mmap(0, sizeof(T) * unit, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (cur == (void *)-1) {
      perror("mmap");
      return NULL;
    }
    stop = cur + unit;
  }
  return cur++;
}

template <typename T, typename R> auto gen_function(T *payload, R (*func)(T *)) {
  return (R(*)()) new (alloc_executable_memory<FunctionWrapper<R, T>>()) FunctionWrapper<R, T>(payload, func);
}
