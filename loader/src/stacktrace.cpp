#include "../include/modloader/log.hpp"
#include "../include/modloader/utils.hpp"

#include <cxxabi.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void print_stacktrace(unsigned int max_frames) {
  void *addrlist[max_frames + 1];
  int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void *));

  if (addrlen == 0) {
    Log::debug("modloader/stacktrace", "<empty, possibly corrupt>");
    return;
  }

  char **symbollist = backtrace_symbols(addrlist, addrlen);

  size_t funcnamesize = 256;
  char *funcname      = (char *)malloc(funcnamesize);

  for (int i = 1; i < addrlen; i++) {
    char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

    for (char *p = symbollist[i]; *p; ++p) {
      if (*p == '(')
        begin_name = p;
      else if (*p == '+')
        begin_offset = p;
      else if (*p == ')' && begin_offset) {
        end_offset = p;
        break;
      }
    }

    if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
      *begin_name++   = '\0';
      *begin_offset++ = '\0';
      *end_offset     = '\0';

      int status;
      char *ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
      if (status == 0) {
        funcname = ret; // use possibly realloc()-ed string
        Log::debug("modloader/stacktrace", "  %s : %s+%s", symbollist[i], funcname, begin_offset);
      } else {
        // demangling failed. Output function name as a C function with
        // no arguments.
        Log::debug("modloader/stacktrace", "  %s : %s()+%s", symbollist[i], begin_name, begin_offset);
      }
    } else {
      Log::debug("modloader/stacktrace", "  %s", symbollist[i]);
    }
  }

  free(funcname);
  free(symbollist);
}
