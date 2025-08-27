// -*- C++ -*-
// HSDS Validator - Data validation for HSDS structures
// Based on the CommUnity Application Design Document

#ifndef HSDS_VALIDATOR_H
#define HSDS_VALIDATOR_H

#include "hsdsC.h"
#include <string>
#include <vector>

class HsdsValidator {
public:
    HsdsValidator();
    ~HsdsValidator();

    // Primary HSDS validation methods
    bool validateOrganization(const HSDS::Organization& org);
    bool validateProgram(const HSDS::Program& program);
    bool validateService(const HSDS::Service& service);
    bool validateLocation(const HSDS::Location& location);
    bool validatePhone(const HSDS::Phone& phone);
    bool validateContact(const HSDS::Contact& contact);
    bool validateAddress(const HSDS::Address& address);
    bool validateServiceAtLocation(const HSDS::ServiceAtLocation& sal);
    bool validateSchedule(const HSDS::Schedule& schedule);
    bool validateAttribute(const HSDS::AttributeInfo& attribute);
    bool validateFunding(const HSDS::Funding& funding);
    bool validateServiceArea(const HSDS::ServiceArea& service_area);
    bool validateRequiredDocument(const HSDS::RequiredDocument& doc);
    bool validateLanguage(const HSDS::Language& language);
    bool validateAccessibility(const HSDS::Accessibility& accessibility);
    bool validateTaxonomyTerm(const HSDS::TaxonomyTerm& taxonomy_term);
    bool validateMetadata(const HSDS::Metadata& metadata);
    bool validateMetaTableDescription(const HSDS::MetaTableDescription& meta_table);
    bool validateCostOption(const HSDS::CostOption& cost_option);
    bool validateOrganizationIdentifier(const HSDS::OrganizationIdentifier& org_id);
    bool validateTaxonomy(const HSDS::Taxonomy& taxonomy);
    bool validateServiceCapacity(const HSDS::ServiceCapacity& capacity);
    bool validateUnit(const HSDS::Unit& unit);
    bool validateUrl(const HSDS::UrlInfo& url);

    // Error reporting
    const std::vector<std::string>& getLastErrors() const { return last_errors_; }
    std::string getLastErrorsAsString() const;
    void clearErrors() { last_errors_.clear(); }

    // Configuration
    void setStrictMode(bool strict) { strict_mode_ = strict; }
    bool isStrictMode() const { return strict_mode_; }

private:
    // Common validation helpers
    bool validateId(const char* id, const std::string& field_name = "id");
    bool validateRequiredString(const char* value, const std::string& field_name);
    bool validateOptionalString(const char* value, const std::string& field_name, size_t max_length = 0);
    bool validateEmail(const char* email);
    bool validateUrl(const char* url_str);
    bool validatePhoneNumber(const char* phone);
    bool validateNumericField(long value, const std::string& field_name, long min_val = LONG_MIN, long max_val = LONG_MAX);
    bool validateDoubleField(double value, const std::string& field_name, double min_val = -DBL_MAX, double max_val = DBL_MAX);

    // Relationship validation helpers
    bool validateRelationship(const char* id, const std::string& field_name, const std::string& referenced_type);
    bool validateForeignKey(const char* foreign_key, const std::string& field_name);

    // Specific field validators
    bool validateTaxStatus(const char* tax_status);
    bool validateLegalStatus(const char* legal_status);
    bool validateServiceStatus(const char* status);
    bool validatePhoneType(const char* type);
    bool validateAddressType(const char* address_type);
    bool validateScheduleFrequency(const char* freq);
    bool validateLanguageCode(const char* code);

    // Error handling
    void addError(const std::string& error);
    void addFieldError(const std::string& field_name, const std::string& error);

    // Member variables
    std::vector<std::string> last_errors_;
    bool strict_mode_;  // If true, apply stricter validation rules

    // Constants for validation
    static const size_t MAX_ID_LENGTH = 100;
    static const size_t MAX_NAME_LENGTH = 255;
    static const size_t MAX_DESCRIPTION_LENGTH = 2000;
    static const size_t MAX_EMAIL_LENGTH = 320;
    static const size_t MAX_URL_LENGTH = 2000;
    static const size_t MAX_PHONE_LENGTH = 50;
    static const size_t MIN_YEAR = 1800;
    static const size_t MAX_YEAR = 2100;
};

#endif // HSDS_VALIDATOR_H