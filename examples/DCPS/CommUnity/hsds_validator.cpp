// -*- C++ -*-
// HSDS Validator Implementation

#include "hsds_validator.h"
#include <ace/Log_Msg.h>
#include <regex>
#include <sstream>
#include <cstring>
#include <climits>
#include <cfloat>

using namespace std;

HsdsValidator::HsdsValidator() : strict_mode_(false) {
}

HsdsValidator::~HsdsValidator() {
}

bool HsdsValidator::validateOrganization(const HSDS::Organization& org) {
    clearErrors();
    bool valid = true;

    // Required fields
    if (!validateId(org.id)) valid = false;
    if (!validateRequiredString(org.name, "name")) valid = false;

    // Optional but important fields
    if (!validateOptionalString(org.alternate_name, "alternate_name", MAX_NAME_LENGTH)) valid = false;
    if (!validateOptionalString(org.description, "description", MAX_DESCRIPTION_LENGTH)) valid = false;
    if (!validateOptionalString(org.email, "email", MAX_EMAIL_LENGTH)) valid = false;
    if (!validateOptionalString(org.website, "website", MAX_URL_LENGTH)) valid = false;
    if (!validateOptionalString(org.logo, "logo", MAX_URL_LENGTH)) valid = false;
    if (!validateOptionalString(org.uri, "uri", MAX_URL_LENGTH)) valid = false;

    // Validate email format if provided
    if (org.email && strlen(org.email) > 0) {
        if (!validateEmail(org.email)) valid = false;
    }

    // Validate website URL if provided
    if (org.website && strlen(org.website) > 0) {
        if (!validateUrl(org.website)) valid = false;
    }

    // Validate year incorporated
    if (org.year_incorporated > 0) {
        if (!validateNumericField(org.year_incorporated, "year_incorporated", MIN_YEAR, MAX_YEAR)) {
            valid = false;
        }
    }

    // Validate tax status
    if (org.tax_status && strlen(org.tax_status) > 0) {
        if (!validateTaxStatus(org.tax_status)) valid = false;
    }

    // Validate legal status
    if (org.legal_status && strlen(org.legal_status) > 0) {
        if (!validateLegalStatus(org.legal_status)) valid = false;
    }

    // Validate relationships
    if (org.parent_organization_id && strlen(org.parent_organization_id) > 0) {
        if (!validateForeignKey(org.parent_organization_id, "parent_organization_id")) valid = false;
    }

    return valid;
}

bool HsdsValidator::validateProgram(const HSDS::Program& program) {
    clearErrors();
    bool valid = true;

    // Required fields
    if (!validateId(program.id)) valid = false;
    if (!validateRequiredString(program.organization_id, "organization_id")) valid = false;
    if (!validateRequiredString(program.name, "name")) valid = false;

    // Optional fields
    if (!validateOptionalString(program.alternate_name, "alternate_name", MAX_NAME_LENGTH)) valid = false;
    if (!validateOptionalString(program.description, "description", MAX_DESCRIPTION_LENGTH)) valid = false;

    // Validate foreign key
    if (!validateForeignKey(program.organization_id, "organization_id")) valid = false;

    return valid;
}

bool HsdsValidator::validateService(const HSDS::Service& service) {
    clearErrors();
    bool valid = true;

    // Required fields
    if (!validateId(service.id)) valid = false;
    if (!validateRequiredString(service.organization_id, "organization_id")) valid = false;
    if (!validateRequiredString(service.name, "name")) valid = false;

    // Optional fields with validation
    if (!validateOptionalString(service.alternate_name, "alternate_name", MAX_NAME_LENGTH)) valid = false;
    if (!validateOptionalString(service.description, "description", MAX_DESCRIPTION_LENGTH)) valid = false;
    if (!validateOptionalString(service.url, "url", MAX_URL_LENGTH)) valid = false;
    if (!validateOptionalString(service.email, "email", MAX_EMAIL_LENGTH)) valid = false;

    // Validate email format if provided
    if (service.email && strlen(service.email) > 0) {
        if (!validateEmail(service.email)) valid = false;
    }

    // Validate URL if provided
    if (service.url && strlen(service.url) > 0) {
        if (!validateUrl(service.url)) valid = false;
    }

    // Validate service status
    if (service.status && strlen(service.status) > 0) {
        if (!validateServiceStatus(service.status)) valid = false;
    }

    // Validate numeric age ranges
    if (service.minimum_age >= 0 && service.maximum_age >= 0) {
        if (service.minimum_age > service.maximum_age) {
            addFieldError("age_range", "minimum_age cannot be greater than maximum_age");
            valid = false;
        }
    }

    // Validate foreign keys
    if (!validateForeignKey(service.organization_id, "organization_id")) valid = false;
    if (service.program_id && strlen(service.program_id) > 0) {
        if (!validateForeignKey(service.program_id, "program_id")) valid = false;
    }

    return valid;
}

bool HsdsValidator::validateLocation(const HSDS::Location& location) {
    clearErrors();
    bool valid = true;

    // Required fields
    if (!validateId(location.id)) valid = false;

    // Optional but important fields
    if (!validateOptionalString(location.organization_id, "organization_id", MAX_ID_LENGTH)) valid = false;
    if (!validateOptionalString(location.name, "name", MAX_NAME_LENGTH)) valid = false;
    if (!validateOptionalString(location.alternate_name, "alternate_name", MAX_NAME_LENGTH)) valid = false;
    if (!validateOptionalString(location.description, "description", MAX_DESCRIPTION_LENGTH)) valid = false;
    if (!validateOptionalString(location.transportation, "transportation", 500)) valid = false;

    // Validate latitude and longitude ranges
    if (location.latitude != 0.0) {
        if (!validateDoubleField(location.latitude, "latitude", -90.0, 90.0)) valid = false;
    }
    if (location.longitude != 0.0) {
        if (!validateDoubleField(location.longitude, "longitude", -180.0, 180.0)) valid = false;
    }

    // Validate foreign key
    if (location.organization_id && strlen(location.organization_id) > 0) {
        if (!validateForeignKey(location.organization_id, "organization_id")) valid = false;
    }

    return valid;
}

bool HsdsValidator::validatePhone(const HSDS::Phone& phone) {
    clearErrors();
    bool valid = true;

    // Required fields
    if (!validateId(phone.id)) valid = false;
    if (!validateRequiredString(phone.number, "number")) valid = false;

    // Validate phone number format
    if (!validatePhoneNumber(phone.number)) valid = false;

    // Validate phone type
    if (phone.type && strlen(phone.type) > 0) {
        if (!validatePhoneType(phone.type)) valid = false;
    }

    // Validate extension
    if (phone.extension < 0 || phone.extension > 99999) {
        addFieldError("extension", "extension must be between 0 and 99999");
        valid = false;
    }

    // Optional fields
    if (!validateOptionalString(phone.description, "description", 500)) valid = false;

    // Validate foreign key references (at least one must be provided)
    bool has_reference = false;
    if (phone.location_id && strlen(phone.location_id) > 0) {
        has_reference = true;
        if (!validateForeignKey(phone.location_id, "location_id")) valid = false;
    }
    if (phone.service_id && strlen(phone.service_id) > 0) {
        has_reference = true;
        if (!validateForeignKey(phone.service_id, "service_id")) valid = false;
    }
    if (phone.organization_id && strlen(phone.organization_id) > 0) {
        has_reference = true;
        if (!validateForeignKey(phone.organization_id, "organization_id")) valid = false;
    }
    if (phone.contact_id && strlen(phone.contact_id) > 0) {
        has_reference = true;
        if (!validateForeignKey(phone.contact_id, "contact_id")) valid = false;
    }
    if (phone.service_at_location_id && strlen(phone.service_at_location_id) > 0) {
        has_reference = true;
        if (!validateForeignKey(phone.service_at_location_id, "service_at_location_id")) valid = false;
    }

    if (!has_reference && strict_mode_) {
        addError("At least one reference (location_id, service_id, organization_id, contact_id, or service_at_location_id) must be provided");
        valid = false;
    }

    return valid;
}

bool HsdsValidator::validateContact(const HSDS::Contact& contact) {
    clearErrors();
    bool valid = true;

    // Required fields
    if (!validateId(contact.id)) valid = false;

    // Optional but important fields
    if (!validateOptionalString(contact.name, "name", MAX_NAME_LENGTH)) valid = false;
    if (!validateOptionalString(contact.title, "title", 100)) valid = false;
    if (!validateOptionalString(contact.department, "department", 100)) valid = false;
    if (!validateOptionalString(contact.email, "email", MAX_EMAIL_LENGTH)) valid = false;

    // Validate email format if provided
    if (contact.email && strlen(contact.email) > 0) {
        if (!validateEmail(contact.email)) valid = false;
    }

    // Validate foreign key references
    if (contact.organization_id && strlen(contact.organization_id) > 0) {
        if (!validateForeignKey(contact.organization_id, "organization_id")) valid = false;
    }
    if (contact.service_id && strlen(contact.service_id) > 0) {
        if (!validateForeignKey(contact.service_id, "service_id")) valid = false;
    }

    return valid;
}

bool HsdsValidator::validateAddress(const HSDS::Address& address) {
    clearErrors();
    bool valid = true;

    // Required fields
    if (!validateId(address.id)) valid = false;
    if (!validateRequiredString(address.location_id, "location_id")) valid = false;

    // Optional but important address fields
    if (!validateOptionalString(address.address_1, "address_1", 200)) valid = false;
    if (!validateOptionalString(address.city, "city", 100)) valid = false;
    if (!validateOptionalString(address.state_province, "state_province", 100)) valid = false;
    if (!validateOptionalString(address.postal_code, "postal_code", 20)) valid = false;
    if (!validateOptionalString(address.country, "country", 100)) valid = false;

    // Validate address type
    if (address.address_type && strlen(address.address_type) > 0) {
        if (!validateAddressType(address.address_type)) valid = false;
    }

    // Validate foreign key
    if (!validateForeignKey(address.location_id, "location_id")) valid = false;

    return valid;
}

bool HsdsValidator::validateServiceAtLocation(const HSDS::ServiceAtLocation& sal) {
    clearErrors();
    bool valid = true;

    // Required fields
    if (!validateId(sal.id)) valid = false;
    if (!validateRequiredString(sal.service_id, "service_id")) valid = false;
    if (!validateRequiredString(sal.location_id, "location_id")) valid = false;

    // Optional fields
    if (!validateOptionalString(sal.description, "description", MAX_DESCRIPTION_LENGTH)) valid = false;

    // Validate foreign keys
    if (!validateForeignKey(sal.service_id, "service_id")) valid = false;
    if (!validateForeignKey(sal.location_id, "location_id")) valid = false;

    return valid;
}

bool HsdsValidator::validateSchedule(const HSDS::Schedule& schedule) {
    clearErrors();
    bool valid = true;

    // Required fields
    if (!validateId(schedule.id)) valid = false;

    // Optional fields
    if (!validateOptionalString(schedule.description, "description", MAX_DESCRIPTION_LENGTH)) valid = false;
    
    // Validate schedule frequency
    if (schedule.freq && strlen(schedule.freq) > 0) {
        if (!validateScheduleFrequency(schedule.freq)) valid = false;
    }

    // Validate time fields (basic format validation)
    if (schedule.opens_at && strlen(schedule.opens_at) > 0) {
        // Basic time format validation could be added here
        if (!validateOptionalString(schedule.opens_at, "opens_at", 20)) valid = false;
    }
    if (schedule.closes_at && strlen(schedule.closes_at) > 0) {
        if (!validateOptionalString(schedule.closes_at, "closes_at", 20)) valid = false;
    }

    return valid;
}

// Validation helper implementations
bool HsdsValidator::validateId(const char* id, const string& field_name) {
    if (!id || strlen(id) == 0) {
        addFieldError(field_name, "is required and cannot be empty");
        return false;
    }

    if (strlen(id) > MAX_ID_LENGTH) {
        addFieldError(field_name, "exceeds maximum length of " + to_string(MAX_ID_LENGTH));
        return false;
    }

    // Check for invalid characters in ID
    regex id_pattern("^[a-zA-Z0-9_.-]+$");
    if (!regex_match(id, id_pattern)) {
        addFieldError(field_name, "contains invalid characters (only alphanumeric, underscore, period, and hyphen allowed)");
        return false;
    }

    return true;
}

bool HsdsValidator::validateRequiredString(const char* value, const string& field_name) {
    if (!value || strlen(value) == 0) {
        addFieldError(field_name, "is required and cannot be empty");
        return false;
    }
    return true;
}

bool HsdsValidator::validateOptionalString(const char* value, const string& field_name, size_t max_length) {
    if (!value) return true;  // Null is acceptable for optional fields

    if (max_length > 0 && strlen(value) > max_length) {
        addFieldError(field_name, "exceeds maximum length of " + to_string(max_length));
        return false;
    }

    return true;
}

bool HsdsValidator::validateEmail(const char* email) {
    if (!email || strlen(email) == 0) return true;  // Empty is acceptable

    // Basic email regex pattern
    regex email_pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!regex_match(email, email_pattern)) {
        addFieldError("email", "has invalid format");
        return false;
    }

    return true;
}

bool HsdsValidator::validateUrl(const char* url_str) {
    if (!url_str || strlen(url_str) == 0) return true;  // Empty is acceptable

    // Basic URL validation
    string url(url_str);
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        addFieldError("url", "must start with http:// or https://");
        return false;
    }

    return true;
}

bool HsdsValidator::validatePhoneNumber(const char* phone) {
    if (!phone || strlen(phone) == 0) return false;

    // Basic phone number validation (allows various formats)
    regex phone_pattern(R"(^[\+]?[1-9]?[\d\s\-\(\)\.]{7,15}$)");
    if (!regex_match(phone, phone_pattern)) {
        addFieldError("number", "has invalid phone number format");
        return false;
    }

    return true;
}

bool HsdsValidator::validateNumericField(long value, const string& field_name, long min_val, long max_val) {
    if (value < min_val || value > max_val) {
        addFieldError(field_name, "must be between " + to_string(min_val) + " and " + to_string(max_val));
        return false;
    }
    return true;
}

bool HsdsValidator::validateDoubleField(double value, const string& field_name, double min_val, double max_val) {
    if (value < min_val || value > max_val) {
        addFieldError(field_name, "must be between " + to_string(min_val) + " and " + to_string(max_val));
        return false;
    }
    return true;
}

bool HsdsValidator::validateForeignKey(const char* foreign_key, const string& field_name) {
    return validateId(foreign_key, field_name);
}

// Specific field validators
bool HsdsValidator::validateTaxStatus(const char* tax_status) {
    if (!tax_status) return true;

    vector<string> valid_statuses = {"501(c)(3)", "501(c)(4)", "501(c)(6)", "government", "for-profit", "other"};
    string status(tax_status);
    
    for (const auto& valid : valid_statuses) {
        if (status == valid) return true;
    }

    addFieldError("tax_status", "must be one of: 501(c)(3), 501(c)(4), 501(c)(6), government, for-profit, other");
    return false;
}

bool HsdsValidator::validateLegalStatus(const char* legal_status) {
    if (!legal_status) return true;

    vector<string> valid_statuses = {"corporation", "nonprofit", "government", "partnership", "sole-proprietorship"};
    string status(legal_status);
    
    for (const auto& valid : valid_statuses) {
        if (status == valid) return true;
    }

    addFieldError("legal_status", "must be one of: corporation, nonprofit, government, partnership, sole-proprietorship");
    return false;
}

bool HsdsValidator::validateServiceStatus(const char* status) {
    if (!status) return true;

    vector<string> valid_statuses = {"active", "inactive", "defunct", "temporarily-closed"};
    string stat(status);
    
    for (const auto& valid : valid_statuses) {
        if (stat == valid) return true;
    }

    addFieldError("status", "must be one of: active, inactive, defunct, temporarily-closed");
    return false;
}

bool HsdsValidator::validatePhoneType(const char* type) {
    if (!type) return true;

    vector<string> valid_types = {"voice", "fax", "tty", "sms", "other"};
    string phone_type(type);
    
    for (const auto& valid : valid_types) {
        if (phone_type == valid) return true;
    }

    addFieldError("type", "must be one of: voice, fax, tty, sms, other");
    return false;
}

bool HsdsValidator::validateAddressType(const char* address_type) {
    if (!address_type) return true;

    vector<string> valid_types = {"physical", "postal", "mailing", "other"};
    string addr_type(address_type);
    
    for (const auto& valid : valid_types) {
        if (addr_type == valid) return true;
    }

    addFieldError("address_type", "must be one of: physical, postal, mailing, other");
    return false;
}

bool HsdsValidator::validateScheduleFrequency(const char* freq) {
    if (!freq) return true;

    vector<string> valid_freqs = {"WEEKLY", "MONTHLY", "YEARLY", "DAILY", "HOURLY"};
    string frequency(freq);
    
    for (const auto& valid : valid_freqs) {
        if (frequency == valid) return true;
    }

    addFieldError("freq", "must be one of: WEEKLY, MONTHLY, YEARLY, DAILY, HOURLY");
    return false;
}

void HsdsValidator::addError(const string& error) {
    last_errors_.push_back(error);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Validation error: %C\n"), error.c_str()));
}

void HsdsValidator::addFieldError(const string& field_name, const string& error) {
    string full_error = field_name + ": " + error;
    addError(full_error);
}

string HsdsValidator::getLastErrorsAsString() const {
    stringstream ss;
    for (size_t i = 0; i < last_errors_.size(); ++i) {
        if (i > 0) ss << "; ";
        ss << last_errors_[i];
    }
    return ss.str();
}

// Stub implementations for remaining validation methods
bool HsdsValidator::validateAttribute(const HSDS::AttributeInfo& attribute) { return true; }
bool HsdsValidator::validateFunding(const HSDS::Funding& funding) { return true; }
bool HsdsValidator::validateServiceArea(const HSDS::ServiceArea& service_area) { return true; }
bool HsdsValidator::validateRequiredDocument(const HSDS::RequiredDocument& doc) { return true; }
bool HsdsValidator::validateLanguage(const HSDS::Language& language) { return true; }
bool HsdsValidator::validateAccessibility(const HSDS::Accessibility& accessibility) { return true; }
bool HsdsValidator::validateTaxonomyTerm(const HSDS::TaxonomyTerm& taxonomy_term) { return true; }
bool HsdsValidator::validateMetadata(const HSDS::Metadata& metadata) { return true; }
bool HsdsValidator::validateMetaTableDescription(const HSDS::MetaTableDescription& meta_table) { return true; }
bool HsdsValidator::validateCostOption(const HSDS::CostOption& cost_option) { return true; }
bool HsdsValidator::validateOrganizationIdentifier(const HSDS::OrganizationIdentifier& org_id) { return true; }
bool HsdsValidator::validateTaxonomy(const HSDS::Taxonomy& taxonomy) { return true; }
bool HsdsValidator::validateServiceCapacity(const HSDS::ServiceCapacity& capacity) { return true; }
bool HsdsValidator::validateUnit(const HSDS::Unit& unit) { return true; }
bool HsdsValidator::validateUrl(const HSDS::UrlInfo& url) { return true; }