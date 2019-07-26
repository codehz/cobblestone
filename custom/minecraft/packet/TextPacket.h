#pragma once

#include "Packet.h"

class TextPacket : public Packet {
  char filler[200 - sizeof(Packet)];

public:
  TextPacket();

  virtual ~TextPacket();
  virtual int getId() const override;
  virtual std::string getName() const override;
  virtual void write(BinaryStream &) const override;
  virtual void read(ReadOnlyBinaryStream &) override;
  virtual bool disallowBatching(void) const override;

  static TextPacket createTranslatedAnnouncement(std::string const &sender, std::string const &content, std::string const &uuid, std::string const &xuid);
};