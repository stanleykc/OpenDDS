// -*- C++ -*-
// HSDS Publisher Manager - Handles DDS DataWriters for all HSDS topics
// Based on the CommUnity Application Design Document

#ifndef HSDS_PUBLISHER_H
#define HSDS_PUBLISHER_H

#include "hsdsC.h"
#include "hsdsTypeSupportImpl.h"
#include "config.h"
#include <dds/DdsDcpsPublicationC.h>
#include <dds/DCPS/Service_Participant.h>
#include <string>
#include <map>
#include <memory>

class HsdsPublisher {
public:
    HsdsPublisher();
    ~HsdsPublisher();

    // Initialize the publisher with DDS participant and configuration
    bool initialize(DDS::DomainParticipant_ptr participant, const Config& config);
    
    // Shutdown and cleanup resources
    void shutdown();

    // Publishing methods for each HSDS topic type
    bool publishOrganization(const HSDS::Organization& org);
    bool publishProgram(const HSDS::Program& program);
    bool publishService(const HSDS::Service& service);
    bool publishLocation(const HSDS::Location& location);
    bool publishPhone(const HSDS::Phone& phone);
    bool publishContact(const HSDS::Contact& contact);
    bool publishAddress(const HSDS::Address& address);
    bool publishServiceAtLocation(const HSDS::ServiceAtLocation& sal);
    bool publishSchedule(const HSDS::Schedule& schedule);
    bool publishAttribute(const HSDS::AttributeInfo& attribute);
    bool publishFunding(const HSDS::Funding& funding);
    bool publishServiceArea(const HSDS::ServiceArea& service_area);
    bool publishRequiredDocument(const HSDS::RequiredDocument& doc);
    bool publishLanguage(const HSDS::Language& language);
    bool publishAccessibility(const HSDS::Accessibility& accessibility);
    bool publishTaxonomyTerm(const HSDS::TaxonomyTerm& taxonomy_term);
    bool publishMetadata(const HSDS::Metadata& metadata);
    bool publishMetaTableDescription(const HSDS::MetaTableDescription& meta_table);
    bool publishCostOption(const HSDS::CostOption& cost_option);
    bool publishOrganizationIdentifier(const HSDS::OrganizationIdentifier& org_id);
    bool publishTaxonomy(const HSDS::Taxonomy& taxonomy);
    bool publishServiceCapacity(const HSDS::ServiceCapacity& capacity);
    bool publishUnit(const HSDS::Unit& unit);
    bool publishUrl(const HSDS::UrlInfo& url);

    // Status and health check methods
    bool isInitialized() const { return initialized_; }
    std::string getStatus() const;
    size_t getPublishedMessageCount() const { return published_count_; }

private:
    // Topic names (matching IDL definitions)
    static const char* ORGANIZATION_TOPIC;
    static const char* PROGRAM_TOPIC;
    static const char* SERVICE_TOPIC;
    static const char* LOCATION_TOPIC;
    static const char* PHONE_TOPIC;
    static const char* CONTACT_TOPIC;
    static const char* ADDRESS_TOPIC;
    static const char* SERVICE_AT_LOCATION_TOPIC;
    static const char* SCHEDULE_TOPIC;
    static const char* ATTRIBUTE_INFO_TOPIC;
    static const char* FUNDING_TOPIC;
    static const char* SERVICE_AREA_TOPIC;
    static const char* REQUIRED_DOCUMENT_TOPIC;
    static const char* LANGUAGE_TOPIC;
    static const char* ACCESSIBILITY_TOPIC;
    static const char* TAXONOMY_TERM_TOPIC;
    static const char* METADATA_TOPIC;
    static const char* META_TABLE_DESCRIPTION_TOPIC;
    static const char* COST_OPTION_TOPIC;
    static const char* ORGANIZATION_IDENTIFIER_TOPIC;
    static const char* TAXONOMY_TOPIC;
    static const char* SERVICE_CAPACITY_TOPIC;
    static const char* UNIT_TOPIC;
    static const char* URL_INFO_TOPIC;

    // Template method for publishing data with type safety
    template<typename T>
    bool publishData(const std::string& topic_name, const T& data);

    // Setup methods
    bool setupTopicsAndWriters();
    bool setupSecurity();
    
    // Helper methods
    void injectNonRepudiation(HSDS::Organization& org);
    void injectNonRepudiation(HSDS::Program& program);
    void injectNonRepudiation(HSDS::Service& service);
    void injectNonRepudiation(HSDS::Location& location);
    void injectNonRepudiation(HSDS::Phone& phone);
    void injectNonRepudiation(HSDS::Contact& contact);
    void injectNonRepudiation(HSDS::Address& address);
    void injectNonRepudiation(HSDS::ServiceAtLocation& sal);
    void injectNonRepudiation(HSDS::Schedule& schedule);
    void injectNonRepudiation(HSDS::AttributeInfo& attribute);
    void injectNonRepudiation(HSDS::Funding& funding);
    void injectNonRepudiation(HSDS::ServiceArea& service_area);
    void injectNonRepudiation(HSDS::RequiredDocument& doc);
    void injectNonRepudiation(HSDS::Language& language);
    void injectNonRepudiation(HSDS::Accessibility& accessibility);
    void injectNonRepudiation(HSDS::TaxonomyTerm& taxonomy_term);
    void injectNonRepudiation(HSDS::Metadata& metadata);
    void injectNonRepudiation(HSDS::MetaTableDescription& meta_table);
    void injectNonRepudiation(HSDS::CostOption& cost_option);
    void injectNonRepudiation(HSDS::OrganizationIdentifier& org_id);
    void injectNonRepudiation(HSDS::Taxonomy& taxonomy);
    void injectNonRepudiation(HSDS::ServiceCapacity& capacity);
    void injectNonRepudiation(HSDS::Unit& unit);
    void injectNonRepudiation(HSDS::UrlInfo& url);

    // Member variables
    DDS::DomainParticipant_var participant_;
    DDS::Publisher_var publisher_;
    Config config_;
    std::string dmp_id_;
    
    // Topics and DataWriters storage
    std::map<std::string, DDS::Topic_var> topics_;
    std::map<std::string, DDS::DataWriter_var> writers_;
    
    // State tracking
    bool initialized_;
    size_t published_count_;
};

#endif // HSDS_PUBLISHER_H