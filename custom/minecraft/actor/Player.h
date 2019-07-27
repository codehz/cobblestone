#pragma once

#include "Actor.h"

class Packet;
class ItemStack;

class Player : public Actor {
public:
  MakeAccessor(getName, std::string, 7224);
  bool canUseAbility(AbilitiesIndex) const;
  PlayerInventoryProxy &getSupplies() const;
  ItemStack &getCarriedItem() const;
  void setCarriedItem(ItemStack const&);
};

class ServerPlayer : public Player {
public:
  void sendNetworkPacket(Packet &) const;
};