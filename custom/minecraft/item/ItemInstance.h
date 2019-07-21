#pragma once

class Item;

class ItemStack;

class ItemInstance {
  char filler[136];

public:
  ItemInstance();
  ItemInstance(ItemStack const &);
  inline ~ItemInstance() {
    asm("call _ZN12ItemInstanceD2Ev@plt");
  }
};