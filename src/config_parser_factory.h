#ifndef CONFIGPARSERFACTORY_H_
#define CONFIGPARSERFACTORY_H_


#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include "config_parser_base.h"


class ConfigParserFactory
{
  using SharedConfigParser = std::shared_ptr<ConfigParserBase>;
  using ConfigParserCreateMethod = SharedConfigParser(*)();

public:
  static bool registerConfigParser(
        const std::string& name, ConfigParserCreateMethod createMethod, bool overwrite = false
  ) noexcept;

  static SharedConfigParser createConfigParser(const std::string& name) noexcept;
  static bool canCreateConfigParser(const std::string& name) noexcept;

  static std::vector<std::string> registeredConfigParsers() noexcept;

private:
  static inline std::unordered_map<std::string, ConfigParserCreateMethod> _createMethods{};
};


#endif // CONFIGPARSERFACTORY_H_
