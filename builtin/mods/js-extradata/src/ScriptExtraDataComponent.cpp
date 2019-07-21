#include "common.h"

#include "ScriptExtraDataComponent.h"

#include <minecraft/actor/Actor.h>
#include <minecraft/block/BlockActor.h>
#include <minecraft/block/BlockSource.h>
#include <mods-js-nbt/nbt.hpp>

std::string ScriptExtraDataComponent::getName() { return "stone:extra_data"; }

ScriptExtraDataComponent::ScriptExtraDataComponent() {}
ScriptExtraDataComponent::~ScriptExtraDataComponent() {}

// * Actor * //
bool ScriptExtraDataComponent::applyComponentTo(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &,
                                                ScriptApi::ScriptObjectHandle const &) const {
  return false;
}
bool ScriptExtraDataComponent::retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &ctx, Actor &actor,
                                                     ScriptApi::ScriptObjectHandle &target) const {
  if (auto tag = std::make_unique<CompoundTag>(); actor.save(*tag)) {
    target = create_tag(tag->copy());
    return true;
  }
  scriptengine->getScriptReportQueue().addError("Failed to retrieve NBT from actor");
  return false;
}
bool ScriptExtraDataComponent::hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &,
                                            bool &result) const {
  result = true;
  return true;
}

// * Block * //
bool ScriptExtraDataComponent::retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Block const &block,
                                                     BlockSource &source, BlockPos &pos, ScriptApi::ScriptObjectHandle &target) const {
  if (auto actor = source.getBlockEntity(pos); actor) {
    if (auto tag = std::make_unique<CompoundTag>(); actor->save(*tag)) {
      target = create_tag(tag->copy());
      return true;
    } else {
      scriptengine->getScriptReportQueue().addError("Failed to retrieve NBT from block: failed to serialize block extra data");
      return false;
    }
  }
  scriptengine->getScriptReportQueue().addError("Failed to retrieve NBT from block: no block actor");
  return false;
}
bool ScriptExtraDataComponent::hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Block const &block,
                                            BlockSource &source, BlockPos &pos, bool &value) const {
  value = !!source.getBlockEntity(pos);
  return true;
}