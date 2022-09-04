#ifndef CONFIGPARSERBASE_H_
#define CONFIGPARSERBASE_H_


#include <string>
#include <memory>

#include <boost/filesystem.hpp>


class ConfigParserBase
{
public:
  virtual bool loadConfigFile(const boost::filesystem::path& file_path) = 0;
  virtual bool saveConfigFile() = 0;
  virtual const std::string& getValueForKey(const std::string& key) const = 0;
  virtual bool setValueForKey(const std::string& key, const std::string& value) = 0;
  virtual bool registered() const noexcept = 0;
};


#endif // CONFIGPARSERBASE_H_
