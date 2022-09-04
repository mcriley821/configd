#ifndef CONFIGSERVER_H_
#define CONFIGSERVER_H_


#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <thread>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "config_parser_base.h"


class ConfigServer
{
  using path = boost::filesystem::path;
  using error_code = boost::system::error_code;
public:
  ConfigServer(unsigned int thread_count);
  ~ConfigServer() = default;

  void setup(const std::vector<std::string>& config_files);
  void run(const std::string& socket_file);
  void stop();

private:
  void handleSignal(const error_code& error, int signum);

  boost::asio::awaitable<void> receive();
  
  std::unique_ptr<boost::asio::io_context> _ioContext;
  std::unique_ptr<boost::asio::local::datagram_protocol::socket> _socket = nullptr;
  std::unique_ptr<boost::asio::strand<boost::asio::io_context::executor_type>> _strand = nullptr;

  std::vector<std::thread> _threadPool;
  std::unordered_map<std::string, std::shared_ptr<ConfigParserBase>> _configParsers;

  boost::asio::signal_set _signalSet;

  unsigned int _threads;
};


#endif // CONFIGSERVER_H_
