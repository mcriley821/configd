#ifndef JSONCONFIGPARSER_H_
#define JSONCONFIGPARSER_H_


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "config_parser.h"


class JsonConfigParser : public ConfigParser<JsonConfigParser>
{
public:
  JsonConfigParser() = default;
  virtual ~JsonConfigParser() = default;

  bool loadConfigFile(const boost::filesystem::path& file_path) override;
  bool saveConfigFile() override;
  const std::string& getValueForKey(const std::string& key) const override;
  bool setValueForKey(const std::string& key, const std::string& value) override;

  bool registered() const noexcept override {
    return _registered;
  }

  static inline std::shared_ptr<ConfigParserBase> create() noexcept {
    return std::make_shared<JsonConfigParser>();
  }

  static inline std::string name() noexcept { return ".json"; }

private:
  boost::property_tree::ptree _ptree;
  boost::filesystem::path _file;
};


#endif // JSONCONFIGPARSER_H_
