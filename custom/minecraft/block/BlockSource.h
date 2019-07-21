#pragma once

#include "../core/types.h"

class Block;
class BlockActor;

class BlockSource {
public:
  bool hasBlock(BlockPos const &) const;
  Block &getBlock(BlockPos const&) const;

  BlockActor *getBlockEntity(BlockPos const&);
};