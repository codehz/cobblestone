#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>

#include <modloader/hook.hpp>

#include <minecraft/core/NBT.h>
#include <minecraft/json.h>

// * ByteTag
std::string ByteTag::toString() const {
  char buffer[5] = {};
  sprintf(buffer, "<%02x>", value);
  return { buffer, 4 };
}
// * ShortTag
std::string ShortTag::toString() const { return std::to_string(value) + "i16"; }
// * IntTag
std::string IntTag::toString() const { return std::to_string(value) + "i32"; }
// * Int64Tag
std::string Int64Tag::toString() const { return std::to_string(value) + "i64"; }
// * FloatTag
std::string FloatTag::toString() const { return std::to_string(value) + "f32"; }
// * DoubleTag
std::string DoubleTag::toString() const { return std::to_string(value) + "f64"; }
// * ByteArrayTag
std::string ByteArrayTag::toString() const {
  std::ostringstream oss;
  oss << std::hex << std::setfill('0') << "byte[ ";
  for (size_t i = 0; i < value.size(); i++) oss << std::setw(2) << (int)value.data()[i] << " ";
  oss << "]";
  return oss.str();
}
// * IntArrayTag
std::string IntArrayTag::toString() const {
  std::ostringstream oss;
  oss << std::hex << std::setfill('0') << "int[ ";
  for (size_t i = 0; i < value.size() / 4; i++) oss << std::setw(8) << ((unsigned *)value.data())[i] << " ";
  oss << "]";
  return oss.str();
}
// * StringTag
std::string StringTag::toString() const {
  Json::FastWriter writer;
  Json::Value temp = value;
  return writer.write(temp);
}
// * ListTag
std::string ListTag::toString() const {
  std::ostringstream oss;
  oss << "list(" << value.size() << ")[ " << std::hex;
  for (auto &item : value) oss << item->toString() << " ";
  oss << "]";
  return oss.str();
}
// * CompoundTag
std::string CompoundTag::toString() const {
  std::ostringstream oss;
  oss << std::hex << "{ ";
  for (auto &[key, value] : value) oss << key << "->" << value->toString() << " ";
  oss << "}";
  return oss.str();
}
// * EndTag
std::string EndTag::toString() const { return "<nil>"; }

// ! ONLY explicit patch ! //
// * Just enable symbol patch * //
extern "C" {
bool DO_PATCH(char const *name) { return false; }
}