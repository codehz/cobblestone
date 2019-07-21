#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/utils.hpp>
#include <sys/mman.h>

class NetworkIdentifier;

class ClientToServerHandshakePacket;

struct ServerNetworkHandler {
  void handle(NetworkIdentifier const &, ClientToServerHandshakePacket const &);
};

TInstanceHook(void, _ZN20ServerNetworkHandler6handleERK17NetworkIdentifierRK11LoginPacket, ServerNetworkHandler, NetworkIdentifier const &netId,
              void *packet) {
  original(this, netId, packet);
  ClientToServerHandshakePacket *handshakePacket = nullptr;
  this->handle(netId, *handshakePacket);
}

THook(void, _ZN20EncryptedNetworkPeer16enableEncryptionERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE) {}

LAZY(fix, {
  auto sym = union_cast<unsigned char *>(dlsym(RTLD_DEFAULT, "_ZN20ServerNetworkHandler6handleERK17NetworkIdentifierRK11LoginPacket"));
  Log::debug("NoEncryption", "Patch %p: %02X %02X %02X %02X %02X", &sym[0x1f48], sym[0x1f48], sym[0x1f49], sym[0x1f4A], sym[0x1f4B], sym[0x1f4C]);
  auto start = ROUND_PAGE_DOWN(union_cast<size_t>(sym) + 0x1f48);
  auto end   = ROUND_PAGE_UP(union_cast<size_t>(sym) + 0x1f48 + 5);
  mprotect(union_cast<void *>(start), end - start, PROT_READ | PROT_WRITE | PROT_EXEC);
  sym[0x1f48] = 0x90;
  sym[0x1f49] = 0x90;
  sym[0x1f4A] = 0x90;
  sym[0x1f4B] = 0x90;
  sym[0x1f4C] = 0x90;
  mprotect(union_cast<void *>(start), end - start, PROT_READ | PROT_EXEC);
});