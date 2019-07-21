#pragma once

#include "Actor.h"

class Player : public Actor {
public:
  PlayerInventoryProxy const &getSupplies() const;
};