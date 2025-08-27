// -*- C++ -*-
// HSDS Publisher Manager Implementation

#include "hsds_publisher.h"
#include <dds/DCPS/Marked_Default_Qos.h>
#include <ace/Log_Msg.h>
#include <sstream>

using namespace std;

// Topic name constants
const char* HsdsPublisher::ORGANIZATION_TOPIC = "Organization";
const char* HsdsPublisher::PROGRAM_TOPIC = "Program";
const char* HsdsPublisher::SERVICE_TOPIC = "Service";
const char* HsdsPublisher::LOCATION_TOPIC = "Location";
const char* HsdsPublisher::PHONE_TOPIC = "Phone";
const char* HsdsPublisher::CONTACT_TOPIC = "Contact";
const char* HsdsPublisher::ADDRESS_TOPIC = "Address";
const char* HsdsPublisher::SERVICE_AT_LOCATION_TOPIC = "ServiceAtLocation";
const char* HsdsPublisher::SCHEDULE_TOPIC = "Schedule";
const char* HsdsPublisher::ATTRIBUTE_INFO_TOPIC = "AttributeInfo";
const char* HsdsPublisher::FUNDING_TOPIC = "Funding";
const char* HsdsPublisher::SERVICE_AREA_TOPIC = "ServiceArea";
const char* HsdsPublisher::REQUIRED_DOCUMENT_TOPIC = "RequiredDocument";
const char* HsdsPublisher::LANGUAGE_TOPIC = "Language";
const char* HsdsPublisher::ACCESSIBILITY_TOPIC = "Accessibility";
const char* HsdsPublisher::TAXONOMY_TERM_TOPIC = "TaxonomyTerm";
const char* HsdsPublisher::METADATA_TOPIC = "Metadata";
const char* HsdsPublisher::META_TABLE_DESCRIPTION_TOPIC = "MetaTableDescription";
const char* HsdsPublisher::COST_OPTION_TOPIC = "CostOption";
const char* HsdsPublisher::ORGANIZATION_IDENTIFIER_TOPIC = "OrganizationIdentifier";
const char* HsdsPublisher::TAXONOMY_TOPIC = "Taxonomy";
const char* HsdsPublisher::SERVICE_CAPACITY_TOPIC = "ServiceCapacity";
const char* HsdsPublisher::UNIT_TOPIC = "Unit";
const char* HsdsPublisher::URL_INFO_TOPIC = "UrlInfo";

HsdsPublisher::HsdsPublisher() 
    : initialized_(false), published_count_(0) {
}

HsdsPublisher::~HsdsPublisher() {
    shutdown();
}

bool HsdsPublisher::initialize(DDS::DomainParticipant_ptr participant, const Config& config) {
    if (initialized_) {
        ACE_DEBUG((LM_WARNING, ACE_TEXT("(%P|%t) HsdsPublisher already initialized\n")));
        return true;
    }

    participant_ = DDS::DomainParticipant::_duplicate(participant);
    config_ = config;
    dmp_id_ = config_.getDmpId();

    // Create publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT,
                                              DDS::PublisherListener::_nil(),
                                              ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil(publisher_.in())) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to create publisher\n")));
        return false;
    }

    // Setup topics and writers for all HSDS types
    if (!setupTopicsAndWriters()) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to setup topics and writers\n")));
        return false;
    }

    initialized_ = true;
    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) HsdsPublisher initialized with DMP ID: %C\n"), 
              dmp_id_.c_str()));
    return true;
}

void HsdsPublisher::shutdown() {
    if (!initialized_) {
        return;
    }

    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) Shutting down HsdsPublisher\n")));

    // Clean up writers and topics
    writers_.clear();
    topics_.clear();

    if (!CORBA::is_nil(publisher_.in()) && !CORBA::is_nil(participant_.in())) {
        participant_->delete_publisher(publisher_.in());
    }

    initialized_ = false;
    published_count_ = 0;
}

bool HsdsPublisher::setupTopicsAndWriters() {
    struct TopicInfo {
        const char* name;
        const char* type_name;
    };

    // Define all topics with their type names
    TopicInfo topics[] = {
        {ORGANIZATION_TOPIC, "HSDS::Organization"},
        {PROGRAM_TOPIC, "HSDS::Program"},
        {SERVICE_TOPIC, "HSDS::Service"},
        {LOCATION_TOPIC, "HSDS::Location"},
        {PHONE_TOPIC, "HSDS::Phone"},
        {CONTACT_TOPIC, "HSDS::Contact"},
        {ADDRESS_TOPIC, "HSDS::Address"},
        {SERVICE_AT_LOCATION_TOPIC, "HSDS::ServiceAtLocation"},
        {SCHEDULE_TOPIC, "HSDS::Schedule"},
        {ATTRIBUTE_INFO_TOPIC, "HSDS::AttributeInfo"},
        {FUNDING_TOPIC, "HSDS::Funding"},
        {SERVICE_AREA_TOPIC, "HSDS::ServiceArea"},
        {REQUIRED_DOCUMENT_TOPIC, "HSDS::RequiredDocument"},
        {LANGUAGE_TOPIC, "HSDS::Language"},
        {ACCESSIBILITY_TOPIC, "HSDS::Accessibility"},
        {TAXONOMY_TERM_TOPIC, "HSDS::TaxonomyTerm"},
        {METADATA_TOPIC, "HSDS::Metadata"},
        {META_TABLE_DESCRIPTION_TOPIC, "HSDS::MetaTableDescription"},
        {COST_OPTION_TOPIC, "HSDS::CostOption"},
        {ORGANIZATION_IDENTIFIER_TOPIC, "HSDS::OrganizationIdentifier"},
        {TAXONOMY_TOPIC, "HSDS::Taxonomy"},
        {SERVICE_CAPACITY_TOPIC, "HSDS::ServiceCapacity"},
        {UNIT_TOPIC, "HSDS::Unit"},
        {URL_INFO_TOPIC, "HSDS::UrlInfo"}
    };

    // Register all type supports
    HSDS::OrganizationTypeSupport_var org_ts = new HSDS::OrganizationTypeSupportImpl();
    if (DDS::RETCODE_OK != org_ts->register_type(participant_.in(), "HSDS::Organization")) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to register Organization type\n")));
        return false;
    }

    HSDS::ProgramTypeSupport_var prog_ts = new HSDS::ProgramTypeSupportImpl();
    if (DDS::RETCODE_OK != prog_ts->register_type(participant_.in(), "HSDS::Program")) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to register Program type\n")));
        return false;
    }

    HSDS::ServiceTypeSupport_var svc_ts = new HSDS::ServiceTypeSupportImpl();
    if (DDS::RETCODE_OK != svc_ts->register_type(participant_.in(), "HSDS::Service")) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to register Service type\n")));
        return false;
    }

    HSDS::LocationTypeSupport_var loc_ts = new HSDS::LocationTypeSupportImpl();
    if (DDS::RETCODE_OK != loc_ts->register_type(participant_.in(), "HSDS::Location")) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to register Location type\n")));
        return false;
    }

    // Register remaining type supports
    HSDS::PhoneTypeSupport_var phone_ts = new HSDS::PhoneTypeSupportImpl();
    phone_ts->register_type(participant_.in(), "HSDS::Phone");

    HSDS::ContactTypeSupport_var contact_ts = new HSDS::ContactTypeSupportImpl();
    contact_ts->register_type(participant_.in(), "HSDS::Contact");

    HSDS::AddressTypeSupport_var addr_ts = new HSDS::AddressTypeSupportImpl();
    addr_ts->register_type(participant_.in(), "HSDS::Address");

    HSDS::ServiceAtLocationTypeSupport_var sal_ts = new HSDS::ServiceAtLocationTypeSupportImpl();
    sal_ts->register_type(participant_.in(), "HSDS::ServiceAtLocation");

    HSDS::ScheduleTypeSupport_var sched_ts = new HSDS::ScheduleTypeSupportImpl();
    sched_ts->register_type(participant_.in(), "HSDS::Schedule");

    HSDS::AttributeInfoTypeSupport_var attr_ts = new HSDS::AttributeInfoTypeSupportImpl();
    attr_ts->register_type(participant_.in(), "HSDS::AttributeInfo");

    HSDS::FundingTypeSupport_var fund_ts = new HSDS::FundingTypeSupportImpl();
    fund_ts->register_type(participant_.in(), "HSDS::Funding");

    HSDS::ServiceAreaTypeSupport_var sa_ts = new HSDS::ServiceAreaTypeSupportImpl();
    sa_ts->register_type(participant_.in(), "HSDS::ServiceArea");

    HSDS::RequiredDocumentTypeSupport_var doc_ts = new HSDS::RequiredDocumentTypeSupportImpl();
    doc_ts->register_type(participant_.in(), "HSDS::RequiredDocument");

    HSDS::LanguageTypeSupport_var lang_ts = new HSDS::LanguageTypeSupportImpl();
    lang_ts->register_type(participant_.in(), "HSDS::Language");

    HSDS::AccessibilityTypeSupport_var acc_ts = new HSDS::AccessibilityTypeSupportImpl();
    acc_ts->register_type(participant_.in(), "HSDS::Accessibility");

    HSDS::TaxonomyTermTypeSupport_var tt_ts = new HSDS::TaxonomyTermTypeSupportImpl();
    tt_ts->register_type(participant_.in(), "HSDS::TaxonomyTerm");

    HSDS::MetadataTypeSupport_var meta_ts = new HSDS::MetadataTypeSupportImpl();
    meta_ts->register_type(participant_.in(), "HSDS::Metadata");

    HSDS::MetaTableDescriptionTypeSupport_var mtd_ts = new HSDS::MetaTableDescriptionTypeSupportImpl();
    mtd_ts->register_type(participant_.in(), "HSDS::MetaTableDescription");

    HSDS::CostOptionTypeSupport_var cost_ts = new HSDS::CostOptionTypeSupportImpl();
    cost_ts->register_type(participant_.in(), "HSDS::CostOption");

    HSDS::OrganizationIdentifierTypeSupport_var oid_ts = new HSDS::OrganizationIdentifierTypeSupportImpl();
    oid_ts->register_type(participant_.in(), "HSDS::OrganizationIdentifier");

    HSDS::TaxonomyTypeSupport_var tax_ts = new HSDS::TaxonomyTypeSupportImpl();
    tax_ts->register_type(participant_.in(), "HSDS::Taxonomy");

    HSDS::ServiceCapacityTypeSupport_var cap_ts = new HSDS::ServiceCapacityTypeSupportImpl();
    cap_ts->register_type(participant_.in(), "HSDS::ServiceCapacity");

    HSDS::UnitTypeSupport_var unit_ts = new HSDS::UnitTypeSupportImpl();
    unit_ts->register_type(participant_.in(), "HSDS::Unit");

    HSDS::UrlInfoTypeSupport_var url_ts = new HSDS::UrlInfoTypeSupportImpl();
    url_ts->register_type(participant_.in(), "HSDS::UrlInfo");

    // Get default topic QoS
    DDS::TopicQos topic_qos;
    participant_->get_default_topic_qos(topic_qos);

    // Get default datawriter QoS
    DDS::DataWriterQos dw_qos;
    publisher_->get_default_datawriter_qos(dw_qos);

    // Create topics and writers for each HSDS type
    size_t num_topics = sizeof(topics) / sizeof(topics[0]);
    for (size_t i = 0; i < num_topics; ++i) {
        // Create topic
        DDS::Topic_var topic = participant_->create_topic(topics[i].name,
                                                         topics[i].type_name,
                                                         topic_qos,
                                                         DDS::TopicListener::_nil(),
                                                         ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (CORBA::is_nil(topic.in())) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to create topic: %C\n"), 
                      topics[i].name));
            return false;
        }

        // Create data writer
        DDS::DataWriter_var writer = publisher_->create_datawriter(topic.in(),
                                                                  dw_qos,
                                                                  DDS::DataWriterListener::_nil(),
                                                                  ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (CORBA::is_nil(writer.in())) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to create writer for topic: %C\n"), 
                      topics[i].name));
            return false;
        }

        // Store topic and writer
        topics_[topics[i].name] = topic;
        writers_[topics[i].name] = writer;

        ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Created topic and writer: %C\n"), 
                  topics[i].name));
    }

    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) Successfully created %d topics and writers\n"), 
              num_topics));
    return true;
}

// Publishing method implementations with non-repudiation injection
bool HsdsPublisher::publishOrganization(const HSDS::Organization& org) {
    HSDS::Organization local_org = org;
    injectNonRepudiation(local_org);
    return publishData(ORGANIZATION_TOPIC, local_org);
}

bool HsdsPublisher::publishProgram(const HSDS::Program& program) {
    HSDS::Program local_program = program;
    injectNonRepudiation(local_program);
    return publishData(PROGRAM_TOPIC, local_program);
}

bool HsdsPublisher::publishService(const HSDS::Service& service) {
    HSDS::Service local_service = service;
    injectNonRepudiation(local_service);
    return publishData(SERVICE_TOPIC, local_service);
}

bool HsdsPublisher::publishLocation(const HSDS::Location& location) {
    HSDS::Location local_location = location;
    injectNonRepudiation(local_location);
    return publishData(LOCATION_TOPIC, local_location);
}

bool HsdsPublisher::publishPhone(const HSDS::Phone& phone) {
    HSDS::Phone local_phone = phone;
    injectNonRepudiation(local_phone);
    return publishData(PHONE_TOPIC, local_phone);
}

bool HsdsPublisher::publishContact(const HSDS::Contact& contact) {
    HSDS::Contact local_contact = contact;
    injectNonRepudiation(local_contact);
    return publishData(CONTACT_TOPIC, local_contact);
}

bool HsdsPublisher::publishAddress(const HSDS::Address& address) {
    HSDS::Address local_address = address;
    injectNonRepudiation(local_address);
    return publishData(ADDRESS_TOPIC, local_address);
}

bool HsdsPublisher::publishServiceAtLocation(const HSDS::ServiceAtLocation& sal) {
    HSDS::ServiceAtLocation local_sal = sal;
    injectNonRepudiation(local_sal);
    return publishData(SERVICE_AT_LOCATION_TOPIC, local_sal);
}

bool HsdsPublisher::publishSchedule(const HSDS::Schedule& schedule) {
    HSDS::Schedule local_schedule = schedule;
    injectNonRepudiation(local_schedule);
    return publishData(SCHEDULE_TOPIC, local_schedule);
}

bool HsdsPublisher::publishAttribute(const HSDS::AttributeInfo& attribute) {
    HSDS::AttributeInfo local_attribute = attribute;
    injectNonRepudiation(local_attribute);
    return publishData(ATTRIBUTE_INFO_TOPIC, local_attribute);
}

bool HsdsPublisher::publishFunding(const HSDS::Funding& funding) {
    HSDS::Funding local_funding = funding;
    injectNonRepudiation(local_funding);
    return publishData(FUNDING_TOPIC, local_funding);
}

bool HsdsPublisher::publishServiceArea(const HSDS::ServiceArea& service_area) {
    HSDS::ServiceArea local_sa = service_area;
    injectNonRepudiation(local_sa);
    return publishData(SERVICE_AREA_TOPIC, local_sa);
}

bool HsdsPublisher::publishRequiredDocument(const HSDS::RequiredDocument& doc) {
    HSDS::RequiredDocument local_doc = doc;
    injectNonRepudiation(local_doc);
    return publishData(REQUIRED_DOCUMENT_TOPIC, local_doc);
}

bool HsdsPublisher::publishLanguage(const HSDS::Language& language) {
    HSDS::Language local_language = language;
    injectNonRepudiation(local_language);
    return publishData(LANGUAGE_TOPIC, local_language);
}

bool HsdsPublisher::publishAccessibility(const HSDS::Accessibility& accessibility) {
    HSDS::Accessibility local_accessibility = accessibility;
    injectNonRepudiation(local_accessibility);
    return publishData(ACCESSIBILITY_TOPIC, local_accessibility);
}

bool HsdsPublisher::publishTaxonomyTerm(const HSDS::TaxonomyTerm& taxonomy_term) {
    HSDS::TaxonomyTerm local_tt = taxonomy_term;
    injectNonRepudiation(local_tt);
    return publishData(TAXONOMY_TERM_TOPIC, local_tt);
}

bool HsdsPublisher::publishMetadata(const HSDS::Metadata& metadata) {
    HSDS::Metadata local_metadata = metadata;
    injectNonRepudiation(local_metadata);
    return publishData(METADATA_TOPIC, local_metadata);
}

bool HsdsPublisher::publishMetaTableDescription(const HSDS::MetaTableDescription& meta_table) {
    HSDS::MetaTableDescription local_mtd = meta_table;
    injectNonRepudiation(local_mtd);
    return publishData(META_TABLE_DESCRIPTION_TOPIC, local_mtd);
}

bool HsdsPublisher::publishCostOption(const HSDS::CostOption& cost_option) {
    HSDS::CostOption local_cost = cost_option;
    injectNonRepudiation(local_cost);
    return publishData(COST_OPTION_TOPIC, local_cost);
}

bool HsdsPublisher::publishOrganizationIdentifier(const HSDS::OrganizationIdentifier& org_id) {
    HSDS::OrganizationIdentifier local_oid = org_id;
    injectNonRepudiation(local_oid);
    return publishData(ORGANIZATION_IDENTIFIER_TOPIC, local_oid);
}

bool HsdsPublisher::publishTaxonomy(const HSDS::Taxonomy& taxonomy) {
    HSDS::Taxonomy local_taxonomy = taxonomy;
    injectNonRepudiation(local_taxonomy);
    return publishData(TAXONOMY_TOPIC, local_taxonomy);
}

bool HsdsPublisher::publishServiceCapacity(const HSDS::ServiceCapacity& capacity) {
    HSDS::ServiceCapacity local_capacity = capacity;
    injectNonRepudiation(local_capacity);
    return publishData(SERVICE_CAPACITY_TOPIC, local_capacity);
}

bool HsdsPublisher::publishUnit(const HSDS::Unit& unit) {
    HSDS::Unit local_unit = unit;
    injectNonRepudiation(local_unit);
    return publishData(UNIT_TOPIC, local_unit);
}

bool HsdsPublisher::publishUrl(const HSDS::UrlInfo& url) {
    HSDS::UrlInfo local_url = url;
    injectNonRepudiation(local_url);
    return publishData(URL_INFO_TOPIC, local_url);
}

// Template method for publishing with error handling
template<typename T>
bool HsdsPublisher::publishData(const string& topic_name, const T& data) {
    auto writer_it = writers_.find(topic_name);
    if (writer_it == writers_.end()) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) No writer found for topic: %C\n"), 
                  topic_name.c_str()));
        return false;
    }

    // This would need proper template specialization for each data type
    // For now, we'll use the basic DataWriter interface
    DDS::ReturnCode_t ret = DDS::RETCODE_ERROR;

    // The actual write would need to be specialized for each type
    // Example for Organization (other types would follow similar pattern):
    if (topic_name == ORGANIZATION_TOPIC) {
        HSDS::OrganizationDataWriter_var org_writer = 
            HSDS::OrganizationDataWriter::_narrow(writer_it->second.in());
        if (!CORBA::is_nil(org_writer.in())) {
            ret = org_writer->write(static_cast<const HSDS::Organization&>(data), DDS::HANDLE_NIL);
        }
    }
    // Add similar blocks for other data types...

    if (ret != DDS::RETCODE_OK) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to publish to topic %C, error: %d\n"), 
                  topic_name.c_str(), ret));
        return false;
    }

    published_count_++;
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Successfully published to topic: %C\n"), 
              topic_name.c_str()));
    return true;
}

string HsdsPublisher::getStatus() const {
    stringstream ss;
    ss << "HsdsPublisher Status:\n"
       << "  Initialized: " << (initialized_ ? "Yes" : "No") << "\n"
       << "  DMP ID: " << dmp_id_ << "\n"
       << "  Published Messages: " << published_count_ << "\n"
       << "  Topics: " << topics_.size() << "\n"
       << "  Writers: " << writers_.size();
    return ss.str();
}

// Non-repudiation injection methods
void HsdsPublisher::injectNonRepudiation(HSDS::Organization& org) {
    org.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Program& program) {
    program.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Service& service) {
    service.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Location& location) {
    location.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Phone& phone) {
    phone.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Contact& contact) {
    contact.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Address& address) {
    address.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::ServiceAtLocation& sal) {
    sal.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Schedule& schedule) {
    schedule.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::AttributeInfo& attribute) {
    attribute.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Funding& funding) {
    funding.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::ServiceArea& service_area) {
    service_area.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::RequiredDocument& doc) {
    doc.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Language& language) {
    language.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Accessibility& accessibility) {
    accessibility.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::TaxonomyTerm& taxonomy_term) {
    taxonomy_term.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Metadata& metadata) {
    metadata.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::MetaTableDescription& meta_table) {
    meta_table.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::CostOption& cost_option) {
    cost_option.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::OrganizationIdentifier& org_id) {
    org_id.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Taxonomy& taxonomy) {
    taxonomy.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::ServiceCapacity& capacity) {
    capacity.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::Unit& unit) {
    unit.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}

void HsdsPublisher::injectNonRepudiation(HSDS::UrlInfo& url) {
    url.dmp_source_id = CORBA::string_dup(dmp_id_.c_str());
}