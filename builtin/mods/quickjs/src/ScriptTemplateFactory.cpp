#include <mods-quickjs/quickjs.hpp>

#include <minecraft/script/ScriptBinderComponent.h>
#include <minecraft/script/ScriptObjectBinder.h>
#include <minecraft/script/ScriptTemplateFactory.h>
#include <mods-quickjs/advcomponent.hpp>

bool ScriptTemplateFactory<ScriptServerContext>::Component::applyComponentTo(ScriptApi::ScriptVersionInfo const &version, ScriptEngine &engine, ScriptServerContext &ctx,
                                                                             ScriptApi::ScriptObjectHandle const &source, ScriptApi::ScriptObjectHandle const &target) const {
  if (auto binder = scriptengine->helpDeserialize(source); binder) {
    if (binder->hasComponent<ScriptActorUniqueIdBinderComponent>()) {
      Actor *actor = nullptr;
      QCHECK(scriptengine->helpGetActor(source, &actor));
      if (actor)
        return applyComponentTo(version, engine, ctx, *actor, target);
      return false;
    } else if (binder->hasComponent<ScriptLevelBinderComponent>()) {
      Level *level = nullptr;
      QCHECK(scriptengine->helpGetLevel(source, &level));
      if (level)
        return applyComponentTo(version, engine, ctx, *level, target);
      return false;
    } else if (auto pos_comp = binder->getComponent<ScriptBlockPositionBinderComponent>(); pos_comp && binder->hasComponent<ScriptTickingAreaBinderComponent>()) {
      BlockSource *blocksource;
      QCHECK(scriptengine->helpGetBlockSource(source, &blocksource));
      Block const *block;
      QCHECK(scriptengine->helpGetBlock(source, &block, *blocksource));
      auto pos = pos_comp->getPosition();
      return applyComponentTo(version, engine, ctx, *block, *blocksource, pos, target);
    } else {
      if (auto adv = dynamic_cast<AdvanceComponent const *>(this); adv) {
        ItemInstance item;
        QCHECK(scriptengine->helpGetItemStackFromPath(item, source));
        QCHECK(adv->applyComponentTo(version, engine, ctx, item, target));
        return scriptengine->helpApplyItemStackWithPath(item, source);
      }
    }
  }
  return false;
}

bool ScriptTemplateFactory<ScriptServerContext>::Component::retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &version, ScriptEngine &engine, ScriptServerContext &ctx,
                                                                                  ScriptApi::ScriptObjectHandle const &source, ScriptApi::ScriptObjectHandle &target) const {
  if (auto binder = scriptengine->helpDeserialize(source); binder) {
    if (binder->hasComponent<ScriptActorUniqueIdBinderComponent>()) {
      Actor *actor = nullptr;
      QCHECK(scriptengine->helpGetActor(source, &actor));
      if (actor)
        return retrieveComponentFrom(version, engine, ctx, *actor, target);
      return false;
    } else if (binder->hasComponent<ScriptLevelBinderComponent>()) {
      Level *level = nullptr;
      QCHECK(scriptengine->helpGetLevel(source, &level));
      if (level)
        return retrieveComponentFrom(version, engine, ctx, *level, target);
      return false;
    } else if (auto pos_comp = binder->getComponent<ScriptBlockPositionBinderComponent>(); pos_comp && binder->hasComponent<ScriptTickingAreaBinderComponent>()) {
      BlockSource *blocksource;
      QCHECK(scriptengine->helpGetBlockSource(source, &blocksource));
      Block const *block;
      QCHECK(scriptengine->helpGetBlock(source, &block, *blocksource));
      if (auto adv = dynamic_cast<AdvanceComponent const *>(this); adv) {
        auto pos = pos_comp->getPosition();
        return adv->retrieveComponentFrom(version, engine, ctx, *block, *blocksource, pos, target);
      } else
        return retrieveComponentFrom(version, engine, ctx, *block, target);
    } else {
      if (auto adv = dynamic_cast<AdvanceComponent const *>(this); adv) {
        ItemInstance item;
        QCHECK(scriptengine->helpGetItemStackFromPath(item, source));
        return adv->retrieveComponentFrom(version, engine, ctx, item, target);
      }
    }
  }
  return false;
}

bool ScriptTemplateFactory<ScriptServerContext>::Component::hasComponent(ScriptApi::ScriptVersionInfo const &version, ScriptEngine &engine, ScriptServerContext &ctx,
                                                                         ScriptApi::ScriptObjectHandle const &source, bool &target) const {
  if (auto binder = scriptengine->helpDeserialize(source); binder) {
    if (binder->hasComponent<ScriptActorUniqueIdBinderComponent>()) {
      Actor *actor = nullptr;
      QCHECK(scriptengine->helpGetActor(source, &actor));
      if (actor)
        return hasComponent(version, engine, ctx, *actor, target);
    } else if (binder->hasComponent<ScriptLevelBinderComponent>()) {
      Level *level = nullptr;
      QCHECK(scriptengine->helpGetLevel(source, &level));
      if (level)
        return hasComponent(version, engine, ctx, *level, target);
    } else if (auto pos_comp = binder->getComponent<ScriptBlockPositionBinderComponent>(); pos_comp && binder->hasComponent<ScriptTickingAreaBinderComponent>()) {
      BlockSource *blocksource;
      QCHECK(scriptengine->helpGetBlockSource(source, &blocksource));
      Block const *block;
      QCHECK(scriptengine->helpGetBlock(source, &block, *blocksource));
      if (auto adv = dynamic_cast<AdvanceComponent const *>(this); adv) {
        auto pos = pos_comp->getPosition();
        return adv->hasComponent(version, engine, ctx, *block, *blocksource, pos, target);
      } else
        return hasComponent(version, engine, ctx, *block, target);
    } else {
      if (auto adv = dynamic_cast<AdvanceComponent const *>(this); adv) {
        ItemInstance item;
        QCHECK(scriptengine->helpGetItemStackFromPath(item, source));
        return adv->hasComponent(version, engine, ctx, item, target);
      }
    }
  }
  return false;
}