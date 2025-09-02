#include "FileSyncApplication.h"
#include "ConfigurationManager.h"

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>

#include <ace/streams.h>
#include <ace/Get_Opt.h>

#include <iostream>
#include <memory>

namespace FileSync {

// Private implementation class (PIMPL pattern)
class FileSyncApplication::Impl {
public:
  explicit Impl(const ConfigurationManager& config)
    : config_manager_(config)
    , running_(false)
    , shutdown_requested_(false)
  {
  }

  ~Impl() {
    shutdown();
  }

  bool initialize(int argc, char* argv[]) {
    try {
      // Initialize DDS
      dpf_ = TheParticipantFactoryWithArgs(argc, argv);
      if (!dpf_) {
        std::cerr << "ERROR: Failed to initialize DDS participant factory" << std::endl;
        return false;
      }

      // Create DDS domain participant
      participant_ = dpf_->create_participant(
        config_manager_.get_domain_id(),
        PARTICIPANT_QOS_DEFAULT,
        0,
        OpenDDS::DCPS::DEFAULT_STATUS_MASK
      );

      if (!participant_) {
        std::cerr << "ERROR: Failed to create DDS participant for domain " 
                  << config_manager_.get_domain_id() << std::endl;
        return false;
      }

      std::cout << "INFO: DDS participant created successfully for domain " 
                << config_manager_.get_domain_id() << std::endl;

      // TODO: Initialize topics, publishers, subscribers
      // TODO: Initialize file system monitor
      // TODO: Initialize file manager

      return true;

    } catch (const std::exception& e) {
      std::cerr << "ERROR: Exception during initialization: " << e.what() << std::endl;
      return false;
    }
  }

  int run() {
    running_ = true;
    
    std::cout << "INFO: File_Sync application is running..." << std::endl;
    std::cout << "INFO: Monitoring: " << config_manager_.get_source_directory() << std::endl;
    std::cout << "INFO: Synchronizing to: " << config_manager_.get_destination_directory() << std::endl;

    // Main application loop
    while (running_ && !shutdown_requested_) {
      // TODO: Process DDS events
      // TODO: Process file system events
      // TODO: Handle sync operations
      
      // For now, just sleep and check for shutdown
      ACE_OS::sleep(ACE_Time_Value(1, 0)); // Sleep 1 second
    }

    std::cout << "INFO: File_Sync application main loop exited" << std::endl;
    return 0;
  }

  void shutdown() {
    if (running_) {
      std::cout << "INFO: Shutting down File_Sync application..." << std::endl;
      running_ = false;
    }

    // TODO: Shutdown file system monitor
    // TODO: Shutdown DDS publishers/subscribers
    
    // Cleanup DDS resources
    if (participant_) {
      if (dpf_) {
        dpf_->delete_participant(participant_);
      }
      participant_ = nullptr;
    }

    if (dpf_) {
      TheServiceParticipant->shutdown();
      dpf_ = nullptr;
    }

    std::cout << "INFO: File_Sync application shutdown complete" << std::endl;
  }

  bool is_running() const {
    return running_;
  }

  void request_shutdown() {
    std::cout << "INFO: Shutdown requested" << std::endl;
    shutdown_requested_ = true;
  }

private:
  const ConfigurationManager& config_manager_;
  bool running_;
  bool shutdown_requested_;

  // DDS objects
  DDS::DomainParticipantFactory_var dpf_;
  DDS::DomainParticipant_var participant_;
};

// FileSyncApplication public interface implementation
FileSyncApplication::FileSyncApplication(const ConfigurationManager& config_manager)
  : pimpl_(std::make_unique<Impl>(config_manager))
{
}

FileSyncApplication::~FileSyncApplication() = default;

bool FileSyncApplication::initialize(int argc, char* argv[]) {
  return pimpl_->initialize(argc, argv);
}

int FileSyncApplication::run() {
  return pimpl_->run();
}

void FileSyncApplication::shutdown() {
  pimpl_->shutdown();
}

bool FileSyncApplication::is_running() const {
  return pimpl_->is_running();
}

void FileSyncApplication::request_shutdown() {
  pimpl_->request_shutdown();
}

} // namespace FileSync