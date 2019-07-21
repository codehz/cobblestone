#pragma once

class CommandOrigin;
class CommandOutput;

class Command {
  char filler[32];

public:
  Command();
  virtual ~Command();
  virtual void execute(CommandOrigin const &, CommandOutput &) = 0;
};