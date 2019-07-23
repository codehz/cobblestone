#include <cstring>
#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/refs.hpp>
#include <modloader/utils.hpp>
#include <netdb.h>
#include <string>
#include <sys/socket.h>

#include <mods-ip/bind.hpp>

bool bypass_force_bind = false;

struct PropertiesSettings {
  std::string const &getCustomProperty(std::string const &) const;

  std::string const &getServerIP() const { return getCustomProperty("server-ip"); }
  std::string const &getServerPortExtra() const { return getCustomProperty("server-port-extra"); }

  std::string const &getServerIPv6() const { return getCustomProperty("server-ipv6"); }
  std::string const &getServerPortv6Extra() const { return getCustomProperty("server-portv6-extra"); }
};

THook(int, bind, int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  if (bypass_force_bind)
    return original(sockfd, addr, addrlen);
  auto &settings = refs<PropertiesSettings>;
  if (addr->sa_family == AF_INET) {
    addrinfo hints = {
        .ai_flags = AI_PASSIVE,
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    addrinfo *list;
    if (getaddrinfo(settings->getServerIP().c_str(), settings->getServerPortExtra().data(), &hints, &list) == 0) {
      if (((sockaddr_in *)addr)->sin_port != 0)
        ((sockaddr_in *)list->ai_addr)->sin_port = ((sockaddr_in *)addr)->sin_port;
      auto ret = original(sockfd, list->ai_addr, list->ai_addrlen);
      freeaddrinfo(list);
      return ret;
    } else if (((sockaddr_in *)addr)->sin_port == 0) {
      if (auto port = atoi(settings->getServerPortExtra().data()); port) {
        sockaddr_in temp;
        memcpy(&temp, addr, sizeof(sockaddr_in));
        temp.sin_port = ntohs(port);
        return original(sockfd, (sockaddr const *)&temp, addrlen);
      }
    }
  }
  if (addr->sa_family == AF_INET6) {
    addrinfo hints = {
        .ai_flags = AI_PASSIVE,
        .ai_family = AF_INET6,
        .ai_socktype = SOCK_DGRAM,
    };
    addrinfo *list;
    if (getaddrinfo(settings->getServerIPv6().c_str(), settings->getServerPortv6Extra().data(), &hints, &list) == 0) {
      if (((sockaddr_in6 *)addr)->sin6_port != 0)
        ((sockaddr_in6 *)list->ai_addr)->sin6_port = ((sockaddr_in6 *)addr)->sin6_port;
      auto ret = original(sockfd, list->ai_addr, list->ai_addrlen);
      freeaddrinfo(list);
      return ret;
    } else if (((sockaddr_in6 *)addr)->sin6_port == 0) {
      if (auto port = atoi(settings->getServerPortv6Extra().data()); port) {
        sockaddr_in6 temp;
        memcpy(&temp, addr, sizeof(sockaddr_in6));
        temp.sin6_port = ntohs(port);
        return original(sockfd, (sockaddr const *)&temp, addrlen);
      }
    }
  }
  return original(sockfd, addr, addrlen);
}