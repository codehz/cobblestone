#pragma once

class EntityRegistry;

class IEntityRegistryOwner {
public:
  virtual ~IEntityRegistryOwner();
  virtual EntityRegistry &getEntityRegistry() = 0;
};

class MinecraftCommands;
class IMinecraftEventing;
class NetworkHandler;
class NetworkStatistics;
class RakNetServerLocator;
class StructureManager;
class Timer;
class Level;

class Minecraft : public IEntityRegistryOwner {
public:
  MinecraftCommands *getCommands();
  IMinecraftEventing *getEventing() const;
  NetworkHandler *getNetworkHandler();
  NetworkHandler *getNetworkHandler() const;
  NetworkStatistics *getNetworkStatistics();
  RakNetServerLocator *getServerLocator();
  RakNetServerLocator *getServerLocator() const;
  NetworkHandler *getServerNetworkHandler();
  StructureManager *getStructureManager();
  Timer *getTimer();
  Level *getLevel() const;
  virtual ~Minecraft();
  virtual EntityRegistry &getEntityRegistry();
};