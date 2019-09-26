#pragma once

#include "Mob.h"

class Packet;
class ItemStack;

class Player : public Mob {
public:
  virtual ~Player() override;

  bool canUseAbility(AbilitiesIndex) const;
  PlayerInventoryProxy &getSupplies() const;
  ItemStack &getCarriedItem() const;
  void setCarriedItem(ItemStack const &);

  unsigned char getPlayerPermissionLevel() const;
  unsigned char getCommandPermissionLevel() const;
};

class ServerPlayer : public Player {
public:
  void sendNetworkPacket(Packet &) const;
  void disconnect(void);
  void sendInventory(bool);
};
