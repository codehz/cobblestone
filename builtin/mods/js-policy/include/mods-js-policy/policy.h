#pragma once

#include <mods-quickjs/quickjs.hpp>

#include <set>

inline namespace Policy {
class BasePolicy {
public:
  bool result;
  BasePolicy(bool result);
  virtual ~BasePolicy();
  virtual std::string const &getName() const = 0;
  virtual JSValue build()                    = 0;
};
class ScriptPolicy : public BasePolicy {
  std::string name;
  JSValue value;

public:
  ScriptPolicy(std::string const &name, JSValue value, bool def);
  virtual ~ScriptPolicy();
  virtual std::string const &getName() const;
  virtual JSValue build();
};
class PolicyManager {
  std::multimap<std::string, JSValue> handlers;
  std::set<std::string> valid_policy;

public:
  void dispatch(BasePolicy &ptr);
  bool addListener(std::string const &name, JSValue callback);
  void registerPolicy(std::string const &);
  bool hasPolicy(std::string const &);
};
} // namespace Policy