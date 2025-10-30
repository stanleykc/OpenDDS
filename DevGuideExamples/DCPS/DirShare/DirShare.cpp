#include "DirShareTypeSupportImpl.h"
#include "FileMonitor.h"
#include "FileUtils.h"
#include "Checksum.h"

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include <dds/DCPS/StaticIncludes.h>

#include <ace/Log_Msg.h>
#include <ace/OS_NS_unistd.h>
#include <ace/Get_Opt.h>

#if OPENDDS_DO_MANUAL_STATIC_INCLUDES
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include <string>
#include <iostream>

// Configuration constants
const int DEFAULT_DOMAIN_ID = 42;
const int POLL_INTERVAL_SEC = 2; // 2 second polling interval

// Global shared directory path
std::string g_shared_directory;

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  int return_code = 0;

  try {
    // Parse command-line arguments
    ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("h"));
    int option;
    while ((option = get_opts()) != EOF) {
      switch (option) {
      case 'h':
      default:
        ACE_ERROR_RETURN((LM_ERROR,
                         ACE_TEXT("Usage: %C [DDS options] <shared_directory>\n")
                         ACE_TEXT("Options:\n")
                         ACE_TEXT("  -h                  Show this help message\n")
                         ACE_TEXT("  -DCPSConfigFile <file> Specify DDS configuration file (e.g., rtps.ini)\n")
                         ACE_TEXT("  -DCPSInfoRepo <ior>    Specify DCPSInfoRepo IOR (InfoRepo mode)\n")
                         ACE_TEXT("\n")
                         ACE_TEXT("Example (InfoRepo mode):\n")
                         ACE_TEXT("  %C -DCPSInfoRepo file://repo.ior /path/to/shared_dir\n")
                         ACE_TEXT("\n")
                         ACE_TEXT("Example (RTPS mode):\n")
                         ACE_TEXT("  %C -DCPSConfigFile rtps.ini /path/to/shared_dir\n"),
                         argv[0], argv[0], argv[0]),
                        1);
      }
    }

    // Initialize DDS DomainParticipantFactory
    DDS::DomainParticipantFactory_var dpf =
      TheParticipantFactoryWithArgs(argc, argv);

    // Get shared directory path from remaining arguments
    if (get_opts.opt_ind() >= argc) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: Missing required <shared_directory> argument\n")
                       ACE_TEXT("Usage: %C [DDS options] <shared_directory>\n"),
                       argv[0]),
                      1);
    }

    g_shared_directory = ACE_TEXT_ALWAYS_CHAR(argv[get_opts.opt_ind()]);

    // Validate shared directory exists and is a directory
    if (!DirShare::is_directory(g_shared_directory)) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: Specified path is not a directory: %C\n"),
                       g_shared_directory.c_str()),
                      1);
    }

    ACE_DEBUG((LM_INFO,
               ACE_TEXT("(%P|%t) DirShare starting...\n")
               ACE_TEXT("  Monitoring directory: %C\n")
               ACE_TEXT("  Poll interval: %d seconds\n"),
               g_shared_directory.c_str(),
               POLL_INTERVAL_SEC));

    // Create DomainParticipant
    DDS::DomainParticipant_var participant =
      dpf->create_participant(DEFAULT_DOMAIN_ID,
                             PARTICIPANT_QOS_DEFAULT,
                             0,
                             OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!participant) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: create_participant failed!\n")),
                      1);
    }

    // Register TypeSupport for FileEvent
    DirShare::FileEventTypeSupport_var ts_event =
      new DirShare::FileEventTypeSupportImpl;

    if (ts_event->register_type(participant, "") != DDS::RETCODE_OK) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: register_type FileEvent failed!\n")),
                      1);
    }

    // Register TypeSupport for FileContent
    DirShare::FileContentTypeSupport_var ts_content =
      new DirShare::FileContentTypeSupportImpl;

    if (ts_content->register_type(participant, "") != DDS::RETCODE_OK) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: register_type FileContent failed!\n")),
                      1);
    }

    // Register TypeSupport for FileChunk
    DirShare::FileChunkTypeSupport_var ts_chunk =
      new DirShare::FileChunkTypeSupportImpl;

    if (ts_chunk->register_type(participant, "") != DDS::RETCODE_OK) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: register_type FileChunk failed!\n")),
                      1);
    }

    // Register TypeSupport for DirectorySnapshot
    DirShare::DirectorySnapshotTypeSupport_var ts_snapshot =
      new DirShare::DirectorySnapshotTypeSupportImpl;

    if (ts_snapshot->register_type(participant, "") != DDS::RETCODE_OK) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: register_type DirectorySnapshot failed!\n")),
                      1);
    }

    // Get type names
    CORBA::String_var type_name_event = ts_event->get_type_name();
    CORBA::String_var type_name_content = ts_content->get_type_name();
    CORBA::String_var type_name_chunk = ts_chunk->get_type_name();
    CORBA::String_var type_name_snapshot = ts_snapshot->get_type_name();

    // Set QoS for RELIABLE and TRANSIENT_LOCAL for FileEvents topic
    DDS::TopicQos topic_qos_events;
    participant->get_default_topic_qos(topic_qos_events);
    topic_qos_events.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    topic_qos_events.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
    topic_qos_events.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
    topic_qos_events.history.depth = 100;

    // Create FileEvents Topic
    DDS::Topic_var topic_events =
      participant->create_topic("DirShare_FileEvents",
                               type_name_event,
                               topic_qos_events,
                               0,
                               OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!topic_events) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: create_topic FileEvents failed!\n")),
                      1);
    }

    // Set QoS for RELIABLE and VOLATILE for FileContent topic
    DDS::TopicQos topic_qos_content;
    participant->get_default_topic_qos(topic_qos_content);
    topic_qos_content.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    topic_qos_content.durability.kind = DDS::VOLATILE_DURABILITY_QOS;
    topic_qos_content.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
    topic_qos_content.history.depth = 1;

    // Create FileContent Topic
    DDS::Topic_var topic_content =
      participant->create_topic("DirShare_FileContent",
                               type_name_content,
                               topic_qos_content,
                               0,
                               OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!topic_content) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: create_topic FileContent failed!\n")),
                      1);
    }

    // Set QoS for RELIABLE and VOLATILE with KEEP_ALL for FileChunks topic
    DDS::TopicQos topic_qos_chunks;
    participant->get_default_topic_qos(topic_qos_chunks);
    topic_qos_chunks.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    topic_qos_chunks.durability.kind = DDS::VOLATILE_DURABILITY_QOS;
    topic_qos_chunks.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
    topic_qos_chunks.resource_limits.max_samples = 1000;
    topic_qos_chunks.resource_limits.max_instances = 100;
    topic_qos_chunks.resource_limits.max_samples_per_instance = 1000;

    // Create FileChunks Topic
    DDS::Topic_var topic_chunks =
      participant->create_topic("DirShare_FileChunks",
                               type_name_chunk,
                               topic_qos_chunks,
                               0,
                               OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!topic_chunks) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: create_topic FileChunks failed!\n")),
                      1);
    }

    // Set QoS for RELIABLE and TRANSIENT_LOCAL for DirectorySnapshot topic
    DDS::TopicQos topic_qos_snapshot;
    participant->get_default_topic_qos(topic_qos_snapshot);
    topic_qos_snapshot.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    topic_qos_snapshot.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
    topic_qos_snapshot.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
    topic_qos_snapshot.history.depth = 1;

    // Create DirectorySnapshot Topic
    DDS::Topic_var topic_snapshot =
      participant->create_topic("DirShare_DirectorySnapshot",
                               type_name_snapshot,
                               topic_qos_snapshot,
                               0,
                               OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!topic_snapshot) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: create_topic DirectorySnapshot failed!\n")),
                      1);
    }

    // Create Publisher
    DDS::Publisher_var publisher =
      participant->create_publisher(PUBLISHER_QOS_DEFAULT,
                                   0,
                                   OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!publisher) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: create_publisher failed!\n")),
                      1);
    }

    // Create Subscriber
    DDS::Subscriber_var subscriber =
      participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT,
                                    0,
                                    OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!subscriber) {
      ACE_ERROR_RETURN((LM_ERROR,
                       ACE_TEXT("ERROR: %N:%l: create_subscriber failed!\n")),
                      1);
    }

    // Create WaitSet for synchronization
    DDS::WaitSet_var ws = new DDS::WaitSet;

    // Attach StatusCondition for Publisher to detect publication matched
    DDS::StatusCondition_var condition = publisher->get_statuscondition();
    condition->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);
    ws->attach_condition(condition);

    ACE_DEBUG((LM_INFO,
               ACE_TEXT("(%P|%t) DDS infrastructure initialized successfully\n")
               ACE_TEXT("  Domain ID: %d\n")
               ACE_TEXT("  Topics created: FileEvents, FileContent, FileChunks, DirectorySnapshot\n"),
               DEFAULT_DOMAIN_ID));

    // TODO: Phase 3+ implementation will add:
    // - FileMonitor instantiation and periodic scanning
    // - DataWriter/DataReader creation for each topic
    // - Listener implementations for receiving remote changes
    // - Initial directory snapshot publishing
    // - Main event loop for file monitoring

    ACE_DEBUG((LM_INFO,
               ACE_TEXT("(%P|%t) DirShare running. Press Ctrl+C to exit.\n")));

    // Temporary: Keep application running until Ctrl+C
    // In later phases, this will be replaced with file monitoring loop
    while (true) {
      ACE_OS::sleep(POLL_INTERVAL_SEC);
      // File monitoring will happen here in Phase 3+
    }

    // Cleanup (will be reached via signal handler or when loop exits)
    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) Shutting down DirShare...\n")));

    ws->detach_condition(condition);

    participant->delete_contained_entities();
    dpf->delete_participant(participant);

    TheServiceParticipant->shutdown();

  } catch (const CORBA::Exception& e) {
    e._tao_print_exception("Exception caught in main():");
    return_code = 1;
  }

  return return_code;
}
