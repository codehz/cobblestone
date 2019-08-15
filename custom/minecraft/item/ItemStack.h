#pragma once

#include <string>
#include <vector>
class Item;
class ItemInstance;

class ItemStack {
  char filler[136];

public:
  ItemStack();
  ItemStack(ItemStack const &);
  ItemStack(ItemInstance const &);
  ItemStack(const Item&,int amo);
  ItemStack &operator=(ItemStack const &);
  std::vector<std::string> getCustomLore() const;
  bool isNull() const;
  void setCustomLore(std::vector<std::string> const &);
  operator bool(void) const;
  inline ~ItemStack() { asm("call _ZN9ItemStackD2Ev"); };
  std::string toString() const;
  bool operator!=(ItemStack const &) const;
  bool operator==(ItemStack const &) const;
  
};
