#pragma once

class ItemInstance;

class ItemStack {
  char filler[136];

public:
  ItemStack();
  ItemStack(ItemInstance const &);
  ItemStack &operator=(ItemStack const &);
  operator bool(void) const;
  ~ItemStack();
};