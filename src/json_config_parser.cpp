#include "json_config_parser.h"

#include <boost/log/trivial.hpp>

namespace fs = boost::filesystem;
namespace ptree = boost::property_tree;
using namespace std;


bool JsonConfigParser::loadConfigFile(const fs::path& config_file)
{
  BOOST_LOG_TRIVIAL(info) << "Loading JSON config file: " << config_file;
  _file = config_file;
  ptree::json_parser::read_json(config_file.string(), _ptree);
  return true;
}


bool JsonConfigParser::saveConfigFile()
{
  BOOST_LOG_TRIVIAL(info) << "Saving JSON config file: " << _file;
  return true;
}


const string& JsonConfigParser::getValueForKey(const string& key) const
{
  BOOST_LOG_TRIVIAL(debug) << "JsonConfigParser: Retrieving value for key: " << key;
  static string ret = "";
  return ret;
}


bool JsonConfigParser::setValueForKey(const string& key, const string& value)
{
  BOOST_LOG_TRIVIAL(debug) << "JsonConfigParser: Setting value for key: "
                           << "'" << key << "' = '" << value << "'";
  return false;
}
