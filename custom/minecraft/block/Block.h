#pragma once

#include "../../aux.h"
#include "../core/NBT.h"
#include <memory>
#include <string>

class BlockLegacy {
public:
  std::string const &getRawNameId() const;
};

class Block {
public:
  CompoundTag const &getSerializationId() const;
  std::string const &getDescriptionId() const;
  std::string getFullName() const;
  BlockLegacy *getLegacyBlock() const;
  MakeAccessor(getTag, CompoundTag, 80);
  MakeAccessor(getLegacy, BlockLegacy *, 16);
};