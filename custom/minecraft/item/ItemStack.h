#pragma once

#include <string>
#include <vector>

class ItemInstance;

class ItemStack {
  char filler[136];

public:
  ItemStack();
  ItemStack(ItemInstance const &);
  ItemStack &operator=(ItemStack const &);
  std::vector<std::string> getCustomLore() const;
  void setCustomLore(std::vector<std::string> const &);
  operator bool(void) const;
  inline ~ItemStack() { asm("call _ZN9ItemStackD2Ev"); };
};