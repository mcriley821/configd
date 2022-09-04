#include "config_server.h"

#include <signal.h>

#include <functional>

#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>

#include "config_parser_factory.h"


using namespace boost::asio;
namespace fs = boost::filesystem;
using error_code = boost::system::error_code;
using endpoint = boost::asio::local::datagram_protocol::endpoint;
using socket_t = boost::asio::local::datagram_protocol::socket;


ConfigServer::ConfigServer(unsigned int threads)
  : _ioContext(std::make_unique<io_context>(threads))
  , _signalSet(_ioContext->get_executor())
  , _receive_buffer(65536, '0')
  , _threads(threads)
{
   for (int signum: {SIGABRT, SIGTERM, SIGINT, SIGHUP, SIGPIPE})
    _signalSet.add(signum);

  _signalSet.async_wait(
      std::bind(&ConfigServer::handleSignal, this,
                std::placeholders::_1, std::placeholders::_2)
  );
  // kick off a thread for signals between now and run()
  _threadPool.emplace_back([&](){ _ioContext->run(); });
}


void ConfigServer::setup(const std::vector<std::string>& config_files) 
{
  BOOST_LOG_TRIVIAL(info) << "Setting up configd server";
  for (const std::string& parser: ConfigParserFactory::registeredConfigParsers())
    BOOST_LOG_TRIVIAL(debug) << "Parser available: " << parser;

  BOOST_LOG_TRIVIAL(info) << "Creating config parsers for provided config files";
  for (const std::string& config_file: config_files) {
    std::string extension = fs::extension(config_file);
    if (!ConfigParserFactory::canCreateConfigParser(extension)) {
      BOOST_LOG_TRIVIAL(warning) << "Could not create config parser for file: " << config_file;
      continue;
    }
    BOOST_LOG_TRIVIAL(debug) << "Creating config parser for file: " << config_file;
    auto parser = ConfigParserFactory::createConfigParser(extension);
    if (!parser->loadConfigFile(config_file)) {
      BOOST_LOG_TRIVIAL(error) << "Could not load config file: " << config_file;
      continue;
    }
    _configParsers[config_file] = parser;
  }
}


void ConfigServer::handleSignal(const error_code& error, int signum)
{
  if (error && error != boost::asio::error::operation_aborted)
    BOOST_LOG_TRIVIAL(error) << "Signal handler error: " <<  error.message();
  if (signum) // 0 if cancelled
    BOOST_LOG_TRIVIAL(info) << "Signal received: " << strsignal(signum);
  stop();
}


void ConfigServer::run(const std::string& socket_file)
{
  BOOST_LOG_TRIVIAL(debug) << "Creating socket file: " << socket_file;
  endpoint _endpoint(socket_file);

  try {
    _socket = std::make_unique<socket_t>(*_ioContext, _endpoint);
  }
  catch (std::exception& exc) {
    std::ostringstream ss;
    ss << "Could not create server socket at " << _endpoint << ": " << exc.what();
    throw std::runtime_error(ss.str());
  }
  _strand = std::make_unique<strand<io_context::executor_type>>(_ioContext->get_executor());

  _socket->set_option(socket_base::broadcast(true));

  BOOST_LOG_TRIVIAL(info) << "Starting to serve...";
  receive();
  for (unsigned int i = 1; i < _threads - 1; i++)
    _threadPool.emplace_back([&](){ _ioContext->run(); });
  _ioContext->run();
}


void ConfigServer::stop() 
{
  BOOST_LOG_TRIVIAL(info) << "Exiting...";
  _ioContext->stop();
  for (std::thread& thread: _threadPool)
    if (thread.joinable())
      thread.join();

  if (_socket) {
    error_code error;
    _socket->close(error);
    if (error)
      BOOST_LOG_TRIVIAL(error) << "Couldn't close server socket: " << error.message();
  }
}


void ConfigServer::receive() 
{
  BOOST_LOG_TRIVIAL(trace) << "Posting async receive from. Ready to receive";
  _socket->async_receive(
      boost::asio::buffer(_receive_buffer.data(), 65536),
      bind_executor(*_strand,
        std::bind(&ConfigServer::onReceive, this,
                  std::placeholders::_1, std::placeholders::_2))
      );
}


void ConfigServer::onReceive(const error_code& error, size_t transferred) 
{
  BOOST_LOG_TRIVIAL(debug) << "Received " << transferred << " bytes of data";
  if (error) {
    BOOST_LOG_TRIVIAL(error) << "Receive error: " << error.message();
  }
  else {
    // parse, post, send
  }
  
  receive();
}
