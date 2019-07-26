#pragma once

#include "Actor.h"

class Packet;

class Player : public Actor {
public:
  MakeAccessor(getName, std::string, 7224);
  bool canUseAbility(AbilitiesIndex) const;
  PlayerInventoryProxy const &getSupplies() const;
};

class ServerPlayer : public Player {
public:
  void sendNetworkPacket(Packet &) const;
};