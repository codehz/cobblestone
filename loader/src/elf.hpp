#pragma once
#include <vector>
#include <string>
#include <functional>

void symelf(std::string const &path, std::function<void(std::string const &, std::string const &)> const &f);
std::vector<std::string> depelf(std::string const &);