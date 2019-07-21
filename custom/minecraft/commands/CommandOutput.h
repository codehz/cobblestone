#pragma once

#include <string>
#include <vector>

class CommandOutputParameter {};

class CommandOutput {
public:
  void success();
  void success(std::string const &, std::vector<CommandOutputParameter> const &params = {});
  void error(std::string const &, std::vector<CommandOutputParameter> const &params = {});
  template <typename T> void set(char const *, T);
};