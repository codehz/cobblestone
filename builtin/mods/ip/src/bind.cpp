#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/refs.hpp>
#include <netdb.h>
#include <string>
#include <sys/socket.h>

#include <mods-ip/bind.hpp>

bool bypass_force_bind = false;

struct PropertiesSettings {
  std::string const &getCustomProperty(std::string const &) const;

  std::string const &getServerIP() const { return getCustomProperty("server-ip"); }

  std::string const &getServerIPv6() const { return getCustomProperty("server-ipv6"); }
};

THook(int, bind, int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  if (bypass_force_bind) return original(sockfd, addr, addrlen);
  if (addr->sa_family == AF_INET) {
    addrinfo hints = {
      .ai_flags    = AI_PASSIVE,
      .ai_family   = AF_INET,
      .ai_socktype = SOCK_DGRAM,
    };
    addrinfo *list;
    if (getaddrinfo(refs<PropertiesSettings>->getServerIP().c_str(), "1", &hints, &list) == 0) {
      ((sockaddr_in *)list->ai_addr)->sin_port = ((sockaddr_in *)addr)->sin_port;
      auto ret                                 = original(sockfd, list->ai_addr, list->ai_addrlen);
      freeaddrinfo(list);
      return ret;
    }
  }
  if (addr->sa_family == AF_INET6) {
    addrinfo hints = {
      .ai_flags    = AI_PASSIVE,
      .ai_family   = AF_INET6,
      .ai_socktype = SOCK_DGRAM,
    };
    addrinfo *list;
    if (getaddrinfo(refs<PropertiesSettings>->getServerIPv6().c_str(), "1", &hints, &list) == 0) {
      ((sockaddr_in6 *)list->ai_addr)->sin6_port = ((sockaddr_in6 *)addr)->sin6_port;
      auto ret                                   = original(sockfd, list->ai_addr, list->ai_addrlen);
      freeaddrinfo(list);
      return ret;
    }
  }
  return original(sockfd, addr, addrlen);
}