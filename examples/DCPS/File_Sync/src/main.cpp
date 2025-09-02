#include "FileSyncApplication.h"
#include "ConfigurationManager.h"

#include <ace/Get_Opt.h>
#include <ace/OS_NS_unistd.h>
#include <ace/OS_NS_stdlib.h>
#include <ace/streams.h>

#include <iostream>
#include <string>

void print_usage(const char* program_name) {
  std::cout << "Usage: " << program_name << " [OPTIONS]\n"
            << "\nOptions:\n"
            << "  -c, --config FILE     Configuration file path (default: file_sync.conf)\n"
            << "  -s, --source DIR      Source directory to monitor\n"
            << "  -d, --dest DIR        Destination directory for synchronized files\n"
            << "  -D, --domain ID       DDS domain ID (default: 42)\n"
            << "  --daemon              Run as daemon/service\n"
            << "  -v, --verbose         Enable verbose logging\n"
            << "  -h, --help            Show this help message\n"
            << std::endl;
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[]) {
  int status = 0;

  try {
    // Parse command line arguments
    std::string config_file = "file_sync.conf";
    std::string source_dir;
    std::string dest_dir;
    int domain_id = 42;
    bool daemon_mode = false;
    bool verbose = false;

    // Parse command line options using ACE_Get_Opt
    ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("c:s:d:D:vh"), 1, 0, 
                         ACE_Get_Opt::LONG_ONLY);
    get_opts.long_option(ACE_TEXT("config"), 'c', ACE_Get_Opt::ARG_REQUIRED);
    get_opts.long_option(ACE_TEXT("source"), 's', ACE_Get_Opt::ARG_REQUIRED);
    get_opts.long_option(ACE_TEXT("dest"), 'd', ACE_Get_Opt::ARG_REQUIRED);
    get_opts.long_option(ACE_TEXT("domain"), 'D', ACE_Get_Opt::ARG_REQUIRED);
    get_opts.long_option(ACE_TEXT("daemon"), ACE_Get_Opt::NO_ARG);
    get_opts.long_option(ACE_TEXT("verbose"), 'v', ACE_Get_Opt::NO_ARG);
    get_opts.long_option(ACE_TEXT("help"), 'h', ACE_Get_Opt::NO_ARG);

    int option;
    while ((option = get_opts()) != EOF) {
      switch (option) {
        case 'c':
          config_file = ACE_TEXT_ALWAYS_CHAR(get_opts.opt_arg());
          break;
        case 's':
          source_dir = ACE_TEXT_ALWAYS_CHAR(get_opts.opt_arg());
          break;
        case 'd':
          dest_dir = ACE_TEXT_ALWAYS_CHAR(get_opts.opt_arg());
          break;
        case 'D':
          domain_id = ACE_OS::atoi(get_opts.opt_arg());
          break;
        case 1: // --daemon
          daemon_mode = true;
          break;
        case 'v':
          verbose = true;
          break;
        case 'h':
          print_usage(ACE_TEXT_ALWAYS_CHAR(argv[0]));
          return 0;
        case '?':
        default:
          print_usage(ACE_TEXT_ALWAYS_CHAR(argv[0]));
          return 1;
      }
    }

    // Create and initialize configuration manager
    FileSync::ConfigurationManager config_manager;
    if (!config_manager.load_configuration(config_file)) {
      std::cerr << "Failed to load configuration from: " << config_file << std::endl;
      return 1;
    }

    // Override configuration with command line arguments
    if (!source_dir.empty()) {
      config_manager.set_source_directory(source_dir);
    }
    if (!dest_dir.empty()) {
      config_manager.set_destination_directory(dest_dir);
    }
    config_manager.set_domain_id(domain_id);
    config_manager.set_verbose_logging(verbose);
    config_manager.set_daemon_mode(daemon_mode);

    // Validate required configuration
    if (!config_manager.validate_configuration()) {
      std::cerr << "Configuration validation failed" << std::endl;
      return 1;
    }

    if (verbose) {
      std::cout << "File_Sync starting with configuration:" << std::endl;
      std::cout << "  Source directory: " << config_manager.get_source_directory() << std::endl;
      std::cout << "  Destination directory: " << config_manager.get_destination_directory() << std::endl;
      std::cout << "  DDS domain ID: " << config_manager.get_domain_id() << std::endl;
    }

    // Create and run the File_Sync application
    FileSync::FileSyncApplication app(config_manager);
    
    if (!app.initialize(argc, argv)) {
      std::cerr << "Failed to initialize File_Sync application" << std::endl;
      return 1;
    }

    if (verbose) {
      std::cout << "File_Sync application initialized successfully" << std::endl;
    }

    // Run the application (this will block until shutdown)
    status = app.run();

    if (verbose) {
      std::cout << "File_Sync application shutting down" << std::endl;
    }

    app.shutdown();

  } catch (const std::exception& e) {
    std::cerr << "File_Sync application error: " << e.what() << std::endl;
    status = 1;
  } catch (...) {
    std::cerr << "File_Sync application unknown error occurred" << std::endl;
    status = 1;
  }

  return status;
}