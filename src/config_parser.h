#ifndef CONFIGPARSER_H_
#define CONFIGPARSER_H_

#include <memory>
#include <string>

#include "config_parser_base.h"
#include "config_parser_factory.h"


template<typename T>
class ConfigParser : public ConfigParserBase
{
public:
  bool loadConfigFile(const boost::filesystem::path& file_path) override = 0;
  bool saveConfigFile() override = 0;
  const std::string& getValueForKey(const std::string& key) const override = 0;
  bool setValueForKey(const std::string& key, const std::string& value) override = 0;
  bool registered() const noexcept override = 0;

  static inline std::shared_ptr<ConfigParserBase> create() noexcept {
    return T::create();
  }

  static inline std::string name() noexcept {
    return T::name();
  }

protected:
  static inline bool _registered = ConfigParserFactory::registerConfigParser(name(), create);
};


#endif // CONFIGPARSER_H_
