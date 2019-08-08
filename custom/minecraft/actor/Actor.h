#pragma once

#include "../core/types.h"

#include "../../aux.h"

#include <memory>

class EntityContext;
class SimpleContainer;
class ActorDefinitionDescriptor;
class Attribute;
class BaseAttributeMap;
class AttributeInstance;
class PlayerInventoryProxy;
class BlockSource;
class TickingArea;
class CompoundTag;
class Dimension;

class Actor {
public:
  MakeAccessor(getActorDefinitionDescriptor, std::unique_ptr<ActorDefinitionDescriptor>, 192);
  MakeAccessor(getBlockSource, std::unique_ptr<BlockSource>, 5952);
  std::string const &getNameTag() const;
  bool hasTickingArea() const;
  bool hasCategory(ActorCategory) const;
  EntityContext &getEntity();
  ActorUniqueID const &getUniqueID() const;
  SimpleContainer &getArmorContainer();
  SimpleContainer &getHandContainer();
  BaseAttributeMap *getAttributes();
  AttributeInstance const &getAttribute(Attribute const &) const;
  TickingArea *getTickingArea();
  bool save(CompoundTag &);
  int getEntityTypeId() const;

  void changeDimension(AutomaticID<Dimension, int>, bool);
  int getDimensionId() const;

  template <typename T> T *tryGetComponent();

  virtual ~Actor();
};