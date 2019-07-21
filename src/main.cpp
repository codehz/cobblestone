#include <filesystem>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>

#if __cplusplus < 201703L
#error CPP17
#endif

namespace fs = std::filesystem;

void loadbds(fs::path path, std::string name, fs::path loader) {
  prctl(PR_SET_PDEATHSIG, SIGHUP);
  setenv("LD_PRELOAD", loader.c_str(), true);
  execl(path.c_str(), name.c_str(), nullptr);
  exit(-1);
}

int main() { loadbds("./game/bedrock_server", "Cobblestone", "./loader.so"); }