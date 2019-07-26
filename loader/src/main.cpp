#include <climits>
#include <condition_variable>
#include <csignal>
#include <cstring>
#include <ctime>
#include <minecraft/core/core.h>
#include <modloader/hook.hpp>
#include <modloader/loader.hpp>
#include <modloader/refs.hpp>
#include <mutex>
#include <unistd.h>

struct DedicatedServer {
  char buffer[0x28];
  bool stopped;
  void stop();
};

MakeStaticReference(DedicatedServer);

static std::mutex stop_mtx;
static std::condition_variable stop_cv;

TInstanceHook(void, _ZN15DedicatedServer4stopEv, DedicatedServer) {
  if (!stopped) {
    {
      std::unique_lock lk{stop_mtx};
      stopped = true;
    }
    stop_cv.notify_all();
  }
}

TClasslessInstanceHook(void, _ZN18ConsoleInputReaderC2Ev) {}
TClasslessInstanceHook(void, _ZN18ConsoleInputReaderD2Ev) {}
TClasslessInstanceHook(bool, _ZN18ConsoleInputReader7getLineERNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE, std::string &str) {
  if (getenv("UPSTART_JOB"))
    kill(getpid(), SIGSTOP);

  std::unique_lock lk{stop_mtx};
  stop_cv.wait(lk);
  return false;
}

THook(long, ftell, FILE *stream) {
  auto ret = original(stream);
  if (ret == LONG_MAX)
    return -1;
  return ret;
}

TInstanceHook(void, _ZN15DedicatedServer17initializeLoggingEv, DedicatedServer) {
  refs<DedicatedServer> = this;
  original(this);
}

THook(std::string, _ZN6Common22getServerVersionStringB5cxx11Ev) { return original() + " modded (cobblestone)"; }

THook(std::string, _ZN6Common23getGameVersionStringNetB5cxx11Ev) { return original() + ".42"; }

static struct Init {
  Init() {
    auto handler = +[](int) {
      refs<DedicatedServer>->stop();
      close(STDIN_FILENO);
    };
    signal(SIGINT, handler);
    signal(SIGHUP, handler);
    ModLoader::addLibSearchDir("builtin");
    ModLoader::addLibSearchDir("data/mods");
    ModLoader::loadModsFromDirectory("builtin");
    ModLoader::loadModsFromDirectory("data/mods");
    new (dlsym(NULL, "_ZN4Util12EMPTY_STRINGB5cxx11E")) std::string();
  }
} init;