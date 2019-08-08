#include <rpcws.hpp>

#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/refs.hpp>
#include <modloader/utils.hpp>

#include <mods-ip/bind.hpp>

#include <minecraft/actor/Player.h>
#include <minecraft/commands/CommandContext.h>
#include <minecraft/commands/CommandOrigin.h>
#include <minecraft/commands/CommandVersion.h>
#include <minecraft/commands/MinecraftCommands.h>
#include <minecraft/core/Minecraft.h>
#include <minecraft/core/ServerInstance.h>
#include <minecraft/core/ServiceLocator.h>
#include <minecraft/level/Level.h>
#include <minecraft/level/PlayerListEntry.h>
#include <minecraft/packet/TextPacket.h>

#include <memory>
#include <thread>

#include <pthread.h>

#include <cstdlib>

#include <optional>

using namespace rpcws;

MakeSingleInstance(std::shared_ptr<epoll>, std::make_shared<epoll>());

auto API_ENDPOINT = getenv("API_ENDPOINT") ?: "ws+unix://data/api.socket";

MakeSingleInstance(RPC, [] {
  bypass_force_bind = true;
  GUARD(bypass_force_bind, bypass_force_bind = false);
  return std::make_unique<server_wsio>(API_ENDPOINT, Instance<std::shared_ptr<epoll>>());
}());

struct DedicatedServer {
  char buffer[0x28];
  bool stopped;
  void stop();
};

class ApiCommandOrigin : public ScriptCommandOrigin {
  std::string name;
  std::function<void(Json::Value &&)> callback;

public:
  INLINE ApiCommandOrigin(ServerLevel &level, ScriptEngine &engine, std::string const &name, std::function<void(Json::Value &&)> callback)
      : ScriptCommandOrigin(level, engine), name(name), callback(callback) {}
  inline virtual ~ApiCommandOrigin() override {}
  inline virtual std::string getName() const override { return name; }
  inline virtual void handleCommandOutputCallback(Json::Value &&value) override { callback(std::move(value)); }
  inline virtual CommandPermissionLevel getPermissionsLevel() const override { return (CommandPermissionLevel)4; }
};

static std::optional<std::tuple<ServerInstance *, Minecraft *, Level *>> getStuff() USED; // ! FIX FOR CCLS
static std::optional<std::tuple<ServerInstance *, Minecraft *, Level *>> getStuff() {
  if (auto instance = ServiceLocator<ServerInstance>::get(); instance)
    if (auto minecraft = instance->getMinecraft(); minecraft)
      if (auto level = minecraft->getLevel(); Level::isUsableLevel(level))
        return std::make_tuple(instance, minecraft, level);
  return {};
}

LAZY(register, {
  auto &ins = Instance<RPC>();
  ins.event("core.log");
  ins.event("chat.recv");
  ins.reg("core.stop", [](auto client, json req) -> json {
    refs<DedicatedServer>->stop();
    return {};
  });
  ins.reg("core.ping", [](auto client, json req) -> json { return req; });
  ins.reg("core.tps", [](auto client, json req) -> json { return {20.0}; }); // TODO
  ins.reg("core.online_players", [](auto client, json req) -> json {
    if (auto stuff = getStuff(); stuff) {
      auto [instancce, mc, level] = *stuff;
      auto arr = json::array();
      for (auto &[uuid, entry] : level->getPlayerList()) {
        auto strUUID = entry.uuid.asString();
        arr.push_back(json::object({
            {"name", entry.name},
            {"xuid", entry.xuid},
            {"uuid", strUUID},
        }));
      }
      return arr;
    }
    throw std::runtime_error("Level is not loaded");
  });

  ins.reg("command.execute", [](auto client, json req) -> promise<json> {
    std::string name, command;
    name = req["name"].get<std::string>();
    command = req["command"].get<std::string>();
    if (auto stuff = getStuff(); stuff) {
      return promise<json>{[=](auto then_fn, auto fail_fn) {
        auto instance = std::get<0>(*stuff);
        auto mc = std::get<1>(*stuff);
        auto level = std::get<2>(*stuff);
        instance->queueForServerThread([=] {
          ScriptEngine *eng = nullptr;
          auto origin = std::make_unique<ApiCommandOrigin>(*(ServerLevel *)level, *eng, name, [=](Json::Value &&data) {
            Json::FastWriter writer;
            then_fn(json::parse(writer.write(data)));
          });
          auto ctx = std::make_shared<CommandContext>(command, std::move(origin), CommandVersion::CurrentVersion);
          mc->getCommands()->executeCommand(ctx, false);
        });
      }};
    }
    throw std::runtime_error("Level is not loaded");
  });

  ins.reg("chat.send", [](auto client, json req) -> json {
    if (auto stuff = getStuff(); stuff) {
      auto level = std::get<2>(*stuff);
      auto packet = TextPacket::createTranslatedAnnouncement(req["sender"], "[" + req["sender"].get<std::string>() + "]" + req["content"].get<std::string>(), "", "1");
      level->forEachPlayer([&](Player &player) {
        if (!player.canUseAbility(AbilitiesIndex::mute))
          if (auto server_player = dynamic_cast<ServerPlayer *>(&player); server_player) {
            server_player->sendNetworkPacket(packet);
          }
        return true;
      });
      return true;
    }
    return false;
  });

  ins.reg("chat.raw", [](auto client, json req) -> json {
    if (auto stuff = getStuff(); stuff) {
      auto level = std::get<2>(*stuff);
      auto packet = TextPacket::createTranslatedAnnouncement("", req["content"].get<std::string>(), "", "1");
      level->forEachPlayer([&](Player &player) {
        if (auto server_player = dynamic_cast<ServerPlayer *>(&player); server_player) {
          server_player->sendNetworkPacket(packet);
        }
        return true;
      });
      return true;
    }
    return false;
  });

  modloader_log_hook(+[](modloader_log_level level, const char *tag, const char *content) { Instance<RPC>().emit("core", json::object({{"level", (int)level}, {"tag", tag}, {"content", content}})); });

  std::thread rthread{[] {
    pthread_setname_np(pthread_self(), "APIs");
    Instance<RPC>().start();
    Instance<std::shared_ptr<epoll>>()->wait();
  }};
  rthread.detach();
});

TClasslessInstanceHook(void, _ZN20ServerNetworkHandler19_displayGameMessageERK6PlayerRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE, Player const &player, std::string const &text) {
  auto &ins = Instance<RPC>();
  ins.emit("chat.recv", json::object({{"sender", player.getNameTag()}, {"content", text}}));
  original(this, player, text);
}