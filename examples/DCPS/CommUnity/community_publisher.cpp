// -*- C++ -*-
// CommunityPublisher - Main application for publishing HSDS data via OpenDDS
// Based on the CommUnity Application Design Document

#include "hsds_publisher.h"
#include "config.h"
#include "http_server.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <ace/streams.h>
#include <ace/Get_Opt.h>
#include <ace/OS_NS_unistd.h>
#include <ace/Log_Msg.h>

using namespace std;

// Default configuration
const char* DEFAULT_CONFIG_FILE = "community_publisher_config.yaml";
const char* DEFAULT_DMP_ID = "community-publisher-default";

void print_usage(const char* prog_name) {
    cout << "Usage: " << prog_name << " [options]\n"
         << "Options:\n"
         << "  -c <config_file>  Configuration file (default: " << DEFAULT_CONFIG_FILE << ")\n"
         << "  -d <dmp_id>       DMP source identifier (default: " << DEFAULT_DMP_ID << ")\n" 
         << "  -h                Show this help\n"
         << endl;
}

int ACE_TMAIN(int argc, ACE_TCHAR *argv[]) {
    string config_file = DEFAULT_CONFIG_FILE;
    string dmp_id = DEFAULT_DMP_ID;
    
    // Parse command line options
    ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("c:d:h"));
    int c;
    while ((c = get_opts()) != -1) {
        switch (c) {
            case 'c':
                config_file = get_opts.opt_arg();
                break;
            case 'd':
                dmp_id = get_opts.opt_arg();
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    try {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) Starting CommunityPublisher\n")));
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) Config file: %C\n"), config_file.c_str()));
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) DMP ID: %C\n"), dmp_id.c_str()));

        // Load configuration
        Config config;
        if (!config.load(config_file)) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to load configuration from %C\n"), 
                      config_file.c_str()));
            return 1;
        }

        // Override DMP ID if provided
        config.setDmpId(dmp_id);

        // Initialize DDS
        DDS::DomainParticipantFactory_var dpf = 
            TheParticipantFactoryWithArgs(argc, argv);

        if (CORBA::is_nil(dpf.in())) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to initialize DomainParticipantFactory\n")));
            return 1;
        }

        // Create DDS participant
        DDS::DomainParticipant_var participant = 
            dpf->create_participant(config.getDomainId(),
                                   PARTICIPANT_QOS_DEFAULT,
                                   DDS::DomainParticipantListener::_nil(),
                                   ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (CORBA::is_nil(participant.in())) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to create DomainParticipant\n")));
            return 1;
        }

        // Initialize HSDS Publisher
        HsdsPublisher hsds_publisher;
        if (!hsds_publisher.initialize(participant.in(), config)) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to initialize HSDS Publisher\n")));
            return 1;
        }

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) HSDS Publisher initialized successfully\n")));

        // Start HTTP API Server
        HttpServer http_server(hsds_publisher, config);
        if (!http_server.start()) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to start HTTP server\n")));
            return 1;
        }

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) HTTP server started on %C:%d\n"), 
                  config.getApiHost().c_str(), config.getApiPort()));

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) CommunityPublisher is running. Press Ctrl+C to stop.\n")));

        // Main loop - wait for shutdown signal
        while (true) {
            ACE_Time_Value sleep_time(1, 0);  // 1 second
            ACE_OS::sleep(sleep_time);
            
            // Check if server should shutdown
            if (!http_server.isRunning()) {
                ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) HTTP server stopped, shutting down...\n")));
                break;
            }
        }

        // Cleanup
        http_server.stop();
        hsds_publisher.shutdown();
        
        participant->delete_contained_entities();
        dpf->delete_participant(participant.in());

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) CommunityPublisher shutdown complete\n")));

    } catch (const CORBA::Exception& e) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) CORBA Exception in main(): %C\n"), 
                  e._info().c_str()));
        return 1;
    } catch (const std::exception& e) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Standard Exception in main(): %C\n"), 
                  e.what()));
        return 1;
    } catch (...) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Unknown Exception in main()\n")));
        return 1;
    }

    TheServiceParticipant->shutdown();
    return 0;
}