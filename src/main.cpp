#include <sys/types.h>  // for pid_t
#include <unistd.h>     // for fork, setsid

#include <vector>
#include <iostream>
#include <cstring>
#include <cerrno>

#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>

#include "config_server.h"


namespace po = boost::program_options;
namespace log = boost::log;
namespace expr = boost::log::expressions;
namespace fs = boost::filesystem;


void print_usage(const po::options_description& options);
int hotload(const std::string& socket_file, const std::vector<std::string>& config_files);
int unload(const std::string& socket_file, const std::vector<std::string>& config_files);


int main(int argc, char** argv) noexcept
{
  std::vector<std::string> config_files;
  std::string log_directory;
  std::string socket_file;
  log::trivial::severity_level log_level;
  unsigned int thread_count;

  po::options_description visible_options("Options");
  visible_options.add_options()
    ("help,h",
        "print this help and exit")
    ("log-directory,o", 
        po::value<std::string>(&log_directory)->default_value("/var/log/configd"),
        "specify log directory")
    ("log-level,l", 
        po::value<log::trivial::severity_level>(&log_level)->default_value(log::trivial::info),
        "specify log level")
    ("socket-file,s",
        po::value<std::string>(&socket_file)->default_value("/var/run/configd.socket"),
        "specify name of the socket file")
    ("hot-load,H", "hot load configuration files to the running configd instance")
    ("unload,U", "unload configuration files from the running configd instance")
    ("threads,t", 
        po::value<unsigned int>(&thread_count)->default_value(8),
        "threads to use in thread pool")
  ;

  po::options_description hidden_options("Hidden options");
  hidden_options.add_options()
    ("config-file", 
        po::value<std::vector<std::string>>(&config_files)->composing(),
        "input a config file to serve")
  ;

  po::positional_options_description positional_options;
  positional_options.add("config-file", -1);

  po::options_description all_options;
  all_options.add(visible_options).add(hidden_options);

  po::variables_map options_map;
  po::command_line_parser parser(argc, argv);
  po::store(parser.options(all_options).positional(positional_options).run(), options_map);

  try {
    po::notify(options_map);
  }
  catch (po::error& error) {
    std::cout << error.what() << std::endl;
    print_usage(visible_options);
    return EXIT_FAILURE;
  }

  if (options_map.count("help")) {
    print_usage(visible_options);
    return EXIT_SUCCESS;
  }
  else if (config_files.size() == 0) {
    std::cerr << "Missing required positional arguments!" << std::endl;
    print_usage(visible_options);
    return EXIT_FAILURE;
  }
  else if (options_map.count("hot-load") && fs::exists(socket_file)) {
    return hotload(socket_file, config_files);
  }
  else if (options_map.count("unload") && fs::exists(socket_file)) {
    return unload(socket_file, config_files);
  }
  else if (fs::exists(socket_file)) {
    std::cerr << "An instance of configd is already running!\n"
              << "To hot-load or unload configuration files, see --hot-load and --unload"
              << std::endl;
    return EXIT_FAILURE;
  }
  else if (thread_count > std::thread::hardware_concurrency()) {
    std::cerr << "Too many threads... Must be less than or equal to " 
              << std::thread::hardware_concurrency()
              << std::endl;
    return EXIT_FAILURE;
  }

  // Create log directory
  if (!fs::exists(log_directory)) {
    boost::system::error_code error;
    if (!fs::create_directories(log_directory, error)) {
      std::cerr << "Could not create log directory: " << error.message() << std::endl;
      return EXIT_FAILURE;
    }
  }

  // fork off from parent
  pid_t pid = fork();
  if (pid < 0) {
    std::cerr << "Fork failed: " << std::strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }
  else if (pid > 0) 
    return EXIT_SUCCESS;
  
  // start a new session
  pid_t sid = setsid();
  if (sid == -1) {
    std::cerr << "setsid failed: " << std::strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }

  // close std fds
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  // Setup logger
  log::add_file_log(
      log::keywords::file_name = fs::path(log_directory) / "configd_%N.log",
      log::keywords::target = log_directory,
      log::keywords::target_file_name = "configd_%N.log",
      log::keywords::max_files = 10,
      log::keywords::rotation_size = 10 * 1024 * 1024,
      log::keywords::format = (
        expr::format("[%1%] %2% %3%")
          % expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
          % log::trivial::severity
          % expr::smessage
      ),
      log::keywords::auto_flush = true
  );
  log::core::get()->set_filter(log::trivial::severity >= log_level);
  log::add_common_attributes();

  // launch the server
  ConfigServer server(thread_count);
  server.setup(config_files);

  int retval = EXIT_SUCCESS;
  try {
    server.run(socket_file);
  }
  catch (std::exception& exc) {
    BOOST_LOG_TRIVIAL(fatal) << "Fatal exception occured: " << exc.what();
    retval = EXIT_FAILURE;
  }
  fs::remove(socket_file);
  return retval;
}


void print_usage(const po::options_description& options)
{
  std::cout << "Usage: configd [OPTION]... FILE...\n"
            << "\n"
            << options
            << std::endl;
}


int hotload(const std::string& /*socket_file*/, const std::vector<std::string>& /*config_files*/)
{
  throw std::logic_error("Not implemented");
  // Open a connection to socket_file, then send each file in config_files
}


int unload(const std::string& /*socket_file*/, const std::vector<std::string>& /*config_files*/)
{
  throw std::logic_error("Not implemented");
  // Open a connection to socket_file, then send each file in config_files
}
