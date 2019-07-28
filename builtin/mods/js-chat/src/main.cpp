#include <mods-quickjs/quickjs.hpp>

#include <minecraft/actor/Player.h>
#include <minecraft/level/Level.h>
#include <minecraft/packet/TextPacket.h>

static JSValue sendText(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 2 && argc != 3 && argc != 4)
    return JS_ThrowTypeError(ctx, "Require 2 or 3 arguments");
  Actor *actor;
  std::string content, sender;
  int flag = 8;
  if (!scriptengine->helpGetActor(argv[0], &actor) || !scriptengine->getValue(argv[1], content))
    return JS_ThrowTypeError(ctx, "Require (actor, string)");
  if (argc >= 3 && !scriptengine->getValue(argv[2], flag))
    return JS_ThrowTypeError(ctx, "Require (actor, string, number)");
  if (argc >= 4 && !scriptengine->getValue(argv[3], sender))
    return JS_ThrowTypeError(ctx, "Require (actor, string, number, string)");
  if (ServerPlayer *player = dynamic_cast<ServerPlayer *>(actor); player) {
    auto packet = TextPacket::createTranslatedAnnouncement("", content, "", "1");
    packet.type() = flag;
    if (sender.size())
      packet.sender() = sender;
    player->sendNetworkPacket(packet);
    return JS_UNDEFINED;
  } else
    return JS_ThrowTypeError(ctx, "Require (player, string)");
}

static JSValue broadcastText(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 1)
    return JS_ThrowTypeError(ctx, "Require 1 arguments");
  std::string content;
  if (!scriptengine->getValue(argv[0], content))
    return JS_ThrowTypeError(ctx, "Require (string)");
  auto packet = TextPacket::createTranslatedAnnouncement("", content, "", "1");
  scriptengine->getScriptServerContext().level->forEachPlayer([&](Player &player) {
    if (auto server_player = dynamic_cast<ServerPlayer *>(&player); server_player) {
      server_player->sendNetworkPacket(packet);
    }
    return true;
  });
  return JS_UNDEFINED;
}

static JSCFunctionListEntry list[] = {
    JS_CFUNC_DEF("sendText", 2, &sendText),
    JS_CFUNC_DEF("broadcastText", 2, &broadcastText),
};

static void entry(JSValue const &system) { JS_SetPropertyFunctionList(js_context, system, list, countof(list)); }

LAZY(register, quickjs_proto_extras.emplace_back(entry));
