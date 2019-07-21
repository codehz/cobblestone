#pragma once

#include <modloader/utils.hpp>

#include <set>
#include <string>
#include <vector>

class Actor;

namespace ScriptApi {
class ScriptObjectHandle;
}

class MyQuerySystem {
  struct QueryInfo {
    std::string name = "minecraft:position", x = "x", y = "y", z = "z";
    std::set<std::string> filter;

    INLINE QueryInfo() {}

    INLINE QueryInfo(std::string const &name, std::string const &x, std::string const &y, std::string const &z)
        : name(name)
        , x(x)
        , y(y)
        , z(z) {}
  };
  std::vector<QueryInfo> queries;

public:
  template <typename... PS> INLINE int registerQuery(PS... args) {
    queries.emplace_back(std::forward<PS>(args)...);
    return queries.size() - 1;
  }
  INLINE bool addFilter(int identifier, std::string filter) {
    if (identifier < (int)queries.size()) {
      queries[identifier].filter.insert(filter);
      return true;
    }
    return false;
  }
  ScriptApi::ScriptObjectHandle queryEntities(int identifier);
  ScriptApi::ScriptObjectHandle queryEntities(int identifier, double x1, double x2, double y1, double y2, double z1, double z2);
};