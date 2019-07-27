#pragma once

#include <memory>
#include <string>

class BlockLegacy;

class BlockTypeRegistry {
public:
  static std::weak_ptr<BlockLegacy> lookupByName(std::string const &);
};