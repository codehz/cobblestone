#pragma once

#include <vector>

class ItemStack;

class PlayerInventoryProxy {
public:
  bool add(ItemStack &, bool);
  bool canAdd(ItemStack const &);
  void clearInventory(int);
  void clearInventoryWithDefault(bool);
  int getHotbarSize();
  std::vector<ItemStack const *> getSlots() const;
};