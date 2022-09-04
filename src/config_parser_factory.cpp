#include "config_parser_factory.h"

#include <algorithm>

bool ConfigParserFactory::registerConfigParser(
      const std::string& name, ConfigParserCreateMethod createMethod, bool overwrite
    ) noexcept
{
  if (overwrite) {
    _createMethods.insert_or_assign(name, createMethod);
    return true;
  }
  else {
    bool can_create = canCreateConfigParser(name);
    if (!can_create)
      _createMethods[name] = createMethod;
    return !can_create;
  }
}


std::shared_ptr<ConfigParserBase> ConfigParserFactory::createConfigParser(
      const std::string& name
    ) noexcept
{
  if (canCreateConfigParser(name))
    return _createMethods.at(name)();
  return nullptr;
}


bool ConfigParserFactory::canCreateConfigParser(const std::string& name) noexcept
{
  auto it = _createMethods.find(name);
  return it != _createMethods.end();
}


std::vector<std::string> ConfigParserFactory::registeredConfigParsers() noexcept
{
  static constexpr auto key_selector = [](auto pair){ return pair.first; };
  std::vector<std::string> keys(_createMethods.size());
  std::transform(_createMethods.begin(), _createMethods.end(), keys.begin(), key_selector);
  return keys;
        
}
