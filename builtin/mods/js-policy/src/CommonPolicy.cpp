#include <mods-js-policy/policy.h>

#include <minecraft/actor/ItemActor.h>
#include <minecraft/actor/Player.h>
#include <minecraft/block/BlockSource.h>
#include <minecraft/core/GameMode.h>
#include <minecraft/item/ItemInstance.h>
#include <minecraft/item/ItemStack.h>
#include <minecraft/script/ScriptVersionInfo.h>
#include <minecraft/script/details/BinderTemplates.h>

#define BUILD_HEADER autohandle handle = JS_NewObject(js_context)
#define BUILD_FOOTER return handle.transfer()
#define NEW_OBJECT(name) autohandle name = JS_NewObject(js_context)

#define FOR_ENTITY(name, var)                                                                                                                                                                          \
  ({                                                                                                                                                                                                   \
    NEW_OBJECT(temp);                                                                                                                                                                                  \
    if (scriptengine->helpDefineActor(var, temp)) {                                                                                                                                                    \
      scriptengine->setMember(handle, name, temp.transfer());                                                                                                                                          \
    } else {                                                                                                                                                                                           \
      scriptengine->setMember(handle, name, JS_NULL);                                                                                                                                                  \
    }                                                                                                                                                                                                  \
  })

#define FOR_ITEM(name, var)                                                                                                                                                                            \
  ({                                                                                                                                                                                                   \
    NEW_OBJECT(temp);                                                                                                                                                                                  \
    if (scriptengine->helpDefineItemStack(var, temp))                                                                                                                                                  \
      scriptengine->setMember(handle, name, temp.transfer());                                                                                                                                          \
  })

#define FOR_POS(name, var)                                                                                                                                                                             \
  ({                                                                                                                                                                                                   \
    NEW_OBJECT(temp);                                                                                                                                                                                  \
    if (scriptengine->helpDefinePosition(var, temp))                                                                                                                                                   \
      scriptengine->setMember(handle, name, temp.transfer());                                                                                                                                          \
  })

#define FOR_BLOCK(name, actor, pos, var)                                                                                                                                                               \
  {                                                                                                                                                                                                    \
    ScriptApi::ScriptVersionInfo version;                                                                                                                                                              \
    NEW_OBJECT(ticking_area);                                                                                                                                                                          \
    NEW_OBJECT(temp);                                                                                                                                                                                  \
    if (scriptengine->helpDefineTickingArea(version, ticking_area, actor->getUniqueID()))                                                                                                              \
      if (auto builder = ScriptBinderBlockTemplate::build(var, pos, ticking_area.transfer()); builder) {                                                                                               \
        builder->serialize(*scriptengine, temp);                                                                                                                                                       \
        scriptengine->setMember(handle, name, temp.transfer());                                                                                                                                        \
      }                                                                                                                                                                                                \
  }

inline namespace Policy {

// * player_attack_entity * //
class ScriptPlayerAttackActorPolicy : public BasePolicy {
  Player *player;
  Actor *target;

public:
  static inline std::string TAG = "stone:player_attack_entity";
  ScriptPlayerAttackActorPolicy(Player *player, Actor *target) : BasePolicy(true), player(player), target(target) {}
  virtual ~ScriptPlayerAttackActorPolicy() {}
  virtual std::string const &getName() const override { return TAG; }
  virtual JSValue build() override {
    BUILD_HEADER;
    FOR_ENTITY("player", *player);
    FOR_ENTITY("target", *target);
    BUILD_FOOTER;
  }
};

// * entity_pick_item_up * //
class ScriptActorPickItemUpPolicy : public BasePolicy {
  Actor *actor;
  ItemInstance item;

public:
  static inline std::string TAG = "stone:entity_pick_item_up";
  ScriptActorPickItemUpPolicy(Actor *actor, ItemStack const &item) : BasePolicy(true), actor(actor), item(item) {}
  virtual ~ScriptActorPickItemUpPolicy() {}
  virtual std::string const &getName() const override { return TAG; }
  virtual JSValue build() override {
    BUILD_HEADER;
    FOR_ENTITY("entity", *actor);
    FOR_ITEM("item", item);
    BUILD_FOOTER;
  }
};

// * entity_drop_item * //
class ScriptActorDropItemPolicy : public BasePolicy {
  Actor *actor;
  ItemInstance item;

public:
  static inline std::string TAG = "stone:entity_drop_item";
  ScriptActorDropItemPolicy(Actor *actor, ItemStack const &item) : BasePolicy(true), actor(actor), item(item) {}
  virtual ~ScriptActorDropItemPolicy() {}
  virtual std::string const &getName() const override { return TAG; }
  virtual JSValue build() override {
    BUILD_HEADER;
    FOR_ENTITY("entity", *actor);
    FOR_ITEM("item", item);
    BUILD_FOOTER;
  }
};

// * player_use_item * //
class ScriptPlayerUseItemPolicy : public BasePolicy {
  Player *player;
  ItemInstance item;

public:
  static inline std::string TAG = "stone:player_use_item";
  ScriptPlayerUseItemPolicy(Player *player, ItemStack const &item) : BasePolicy(true), player(player), item(item) {}
  virtual ~ScriptPlayerUseItemPolicy() {}
  virtual std::string const &getName() const override { return TAG; }
  virtual JSValue build() override {
    BUILD_HEADER;
    FOR_ENTITY("player", *player);
    FOR_ITEM("item", item);
    BUILD_FOOTER;
  }
};

// * player_use_item_on * //
class ScriptPlayerUseItemOnPolicy : public BasePolicy {
  Player *player;
  BlockPos block_pos;
  Vec3 pos;
  ItemInstance item;
  Block const *block;

public:
  static inline std::string TAG = "stone:player_use_item_on";
  ScriptPlayerUseItemOnPolicy(Player *player, BlockPos block_pos, Vec3 pos, ItemStack const &item, Block const *block)
      : BasePolicy(true), player(player), block_pos(block_pos), pos(pos), item(item), block(block) {}
  virtual ~ScriptPlayerUseItemOnPolicy() {}
  virtual std::string const &getName() const override { return TAG; }
  virtual JSValue build() override {
    BUILD_HEADER;
    FOR_ENTITY("player", *player);
    FOR_POS("pos", pos);
    FOR_ITEM("item", item);
    FOR_BLOCK("block", player, block_pos, *block);
    BUILD_FOOTER;
  }
};

// * player_destroy_block * //
class ScriptPlayerDestroyBlockPolicy : public BasePolicy {
  Player *player;
  BlockPos pos;

public:
  static inline std::string TAG = "stone:player_destroy_block";
  ScriptPlayerDestroyBlockPolicy(Player *player, BlockPos pos) : BasePolicy(true), player(player), pos(pos) {}
  virtual ~ScriptPlayerDestroyBlockPolicy() {}
  virtual std::string const &getName() const override { return TAG; }
  virtual JSValue build() override {
    BUILD_HEADER;
    FOR_ENTITY("player", *player);
    auto &bs = player->getBlockSource();
    auto &block = bs->getBlock(pos);
    FOR_BLOCK("block", player, pos, block);
    BUILD_FOOTER;
  }
};

} // namespace Policy

// ~ player_attack_entity ~ //
TInstanceHook(bool, _ZN6Player6attackER5Actor, Player, Actor *target) {
  if (this->getCommandPermissionLevel() == 0) return false;
  ScriptPlayerAttackActorPolicy policy(this, target);
  Instance<PolicyManager>().dispatch(policy);
  if (!policy.result)
    return false;
  return original(this, target);
}

// ~ entity_pick_item_up ~ //
TInstanceHook(bool, _ZN5Actor10pickUpItemER9ItemActor, Actor, ItemActor *item) {
  ScriptActorPickItemUpPolicy policy(this, item->getItemStack());
  Instance<PolicyManager>().dispatch(policy);
  if (!policy.result)
    return false;
  return original(this, item);
}
TInstanceHook(bool, _ZN6Player4takeER5Actorii, Player, Actor &actor, int a, int b) {
  if (this->getCommandPermissionLevel() == 0) return false;
  if (actor.hasCategory(ActorCategory::Item)) {
    ScriptActorPickItemUpPolicy policy(this, ((ItemActor &)actor).getItemStack());
    Instance<PolicyManager>().dispatch(policy);
    if (!policy.result)
      return false;
  }
  return original(this, actor, a, b);
}

// ~ entity_drop_item ~ //
TInstanceHook(void, _ZN5Actor4dropERK9ItemStackb, Actor, ItemStack const &stack, bool flag) {
  ScriptActorDropItemPolicy policy(this, stack);
  Instance<PolicyManager>().dispatch(policy);
  if (!policy.result)
    return;
  return original(this, stack, flag);
}
TInstanceHook(void, _ZN5Actor11dropTowardsERK9ItemStack4Vec3, Actor, ItemStack const &stack, Vec3 vec) {
  ScriptActorDropItemPolicy policy(this, stack);
  Instance<PolicyManager>().dispatch(policy);
  if (!policy.result)
    return;
  return original(this, stack, vec);
}

// ~ player_use_item ~ //
TInstanceHook(bool, _ZN8GameMode7useItemER9ItemStack, GameMode, ItemStack &stack) {
  if (this->getPlayer()->getCommandPermissionLevel() == 0) return false;
  ScriptPlayerUseItemPolicy policy(this->getPlayer(), stack);
  Instance<PolicyManager>().dispatch(policy);
  if (!policy.result)
    return false;
  return original(this, stack);
}

// ~ player_use_item_on ~ //
TInstanceHook(bool, _ZN8GameMode9useItemOnER9ItemStackRK8BlockPoshRK4Vec3PK5Block, GameMode, ItemStack &stack, BlockPos const &pos, unsigned char unch, Vec3 const &vec, Block const *block) {
  if (this->getPlayer()->getCommandPermissionLevel() == 0) return false;
  ScriptPlayerUseItemOnPolicy policy(this->getPlayer(), pos, vec, stack, block);
  Instance<PolicyManager>().dispatch(policy);
  if (!policy.result)
    return false;
  return original(this, stack, pos, unch, vec, block);
}

// ~ player_destroy_block ~ //
TInstanceHook(bool, _ZN8GameMode12destroyBlockERK8BlockPosh, GameMode, BlockPos const &pos, unsigned char flag) {
  if (this->getPlayer()->getCommandPermissionLevel() == 0) return false;
  ScriptPlayerDestroyBlockPolicy policy(this->getPlayer(), pos);
  Instance<PolicyManager>().dispatch(policy);
  if (!policy.result)
    return false;
  return original(this, pos, flag);
}

// ! register ! //
LAZY(register, {
  auto &mgr = Instance<PolicyManager>();
  mgr.registerPolicy(ScriptPlayerAttackActorPolicy::TAG);
  mgr.registerPolicy(ScriptActorPickItemUpPolicy::TAG);
  mgr.registerPolicy(ScriptActorDropItemPolicy::TAG);
  mgr.registerPolicy(ScriptPlayerUseItemPolicy::TAG);
  mgr.registerPolicy(ScriptPlayerUseItemOnPolicy::TAG);
  mgr.registerPolicy(ScriptPlayerDestroyBlockPolicy::TAG);
});