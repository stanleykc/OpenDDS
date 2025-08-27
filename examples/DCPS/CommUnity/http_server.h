// -*- C++ -*-
// HTTP API Server - RESTful API for submitting HSDS data
// Based on the CommUnity Application Design Document

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "hsds_publisher.h"
#include "config.h"
#include "hsds_validator.h"
#include <ace/Task.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>
#include <string>
#include <map>
#include <memory>

// Forward declarations
class HttpRequest;
class HttpResponse;

class HttpServer : public ACE_Task_Base {
public:
    HttpServer(HsdsPublisher& publisher, const Config& config);
    ~HttpServer();

    // Server control
    bool start();
    void stop();
    bool isRunning() const { return running_; }

    // Task interface
    virtual int svc() override;

private:
    // HTTP handling
    void handleClient(ACE_SOCK_Stream& client_stream);
    HttpRequest parseRequest(const std::string& request_data);
    HttpResponse handleRequest(const HttpRequest& request);
    std::string buildResponse(const HttpResponse& response);

    // API endpoint handlers
    HttpResponse handleHealthCheck();
    HttpResponse handleStatus();
    HttpResponse handleSubmitData(const std::string& table_name, const std::string& json_data);
    HttpResponse handleUpdateData(const std::string& table_name, const std::string& id, const std::string& json_data);
    HttpResponse handleDeleteData(const std::string& table_name, const std::string& id);

    // JSON conversion methods
    bool jsonToOrganization(const std::string& json, HSDS::Organization& org);
    bool jsonToProgram(const std::string& json, HSDS::Program& program);
    bool jsonToService(const std::string& json, HSDS::Service& service);
    bool jsonToLocation(const std::string& json, HSDS::Location& location);
    bool jsonToPhone(const std::string& json, HSDS::Phone& phone);
    bool jsonToContact(const std::string& json, HSDS::Contact& contact);
    bool jsonToAddress(const std::string& json, HSDS::Address& address);
    bool jsonToServiceAtLocation(const std::string& json, HSDS::ServiceAtLocation& sal);
    bool jsonToSchedule(const std::string& json, HSDS::Schedule& schedule);
    bool jsonToAttribute(const std::string& json, HSDS::AttributeInfo& attribute);
    bool jsonToFunding(const std::string& json, HSDS::Funding& funding);
    bool jsonToServiceArea(const std::string& json, HSDS::ServiceArea& service_area);
    bool jsonToRequiredDocument(const std::string& json, HSDS::RequiredDocument& doc);
    bool jsonToLanguage(const std::string& json, HSDS::Language& language);
    bool jsonToAccessibility(const std::string& json, HSDS::Accessibility& accessibility);
    bool jsonToTaxonomyTerm(const std::string& json, HSDS::TaxonomyTerm& taxonomy_term);
    bool jsonToMetadata(const std::string& json, HSDS::Metadata& metadata);
    bool jsonToMetaTableDescription(const std::string& json, HSDS::MetaTableDescription& meta_table);
    bool jsonToCostOption(const std::string& json, HSDS::CostOption& cost_option);
    bool jsonToOrganizationIdentifier(const std::string& json, HSDS::OrganizationIdentifier& org_id);
    bool jsonToTaxonomy(const std::string& json, HSDS::Taxonomy& taxonomy);
    bool jsonToServiceCapacity(const std::string& json, HSDS::ServiceCapacity& capacity);
    bool jsonToUnit(const std::string& json, HSDS::Unit& unit);
    bool jsonToUrl(const std::string& json, HSDS::UrlInfo& url);

    // Utility methods
    bool validateAuthToken(const std::string& token);
    std::string extractJsonValue(const std::string& json, const std::string& key);
    std::string generateId();
    void logRequest(const HttpRequest& request);
    
    // Member variables
    HsdsPublisher& publisher_;
    Config config_;
    HsdsValidator validator_;
    ACE_SOCK_Acceptor acceptor_;
    bool running_;
    size_t request_count_;
    std::string auth_token_;
};

// HTTP request representation
class HttpRequest {
public:
    enum Method { GET, POST, PUT, DELETE, UNKNOWN };

    HttpRequest() : method_(UNKNOWN) {}

    Method getMethod() const { return method_; }
    void setMethod(Method method) { method_ = method; }

    const std::string& getPath() const { return path_; }
    void setPath(const std::string& path) { path_ = path; }

    const std::string& getBody() const { return body_; }
    void setBody(const std::string& body) { body_ = body; }

    std::string getHeader(const std::string& name) const;
    void setHeader(const std::string& name, const std::string& value);

    // Parse path components for REST API
    std::vector<std::string> getPathComponents() const;
    std::map<std::string, std::string> getQueryParameters() const;

private:
    Method method_;
    std::string path_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};

// HTTP response representation
class HttpResponse {
public:
    HttpResponse(int status = 200, const std::string& content_type = "application/json") 
        : status_(status), content_type_(content_type) {}

    int getStatus() const { return status_; }
    void setStatus(int status) { status_ = status; }

    const std::string& getContentType() const { return content_type_; }
    void setContentType(const std::string& content_type) { content_type_ = content_type; }

    const std::string& getBody() const { return body_; }
    void setBody(const std::string& body) { body_ = body; }

    void setHeader(const std::string& name, const std::string& value);
    const std::map<std::string, std::string>& getHeaders() const { return headers_; }

    // Convenience methods
    void setJsonBody(const std::string& json);
    void setErrorResponse(int status, const std::string& message);

private:
    int status_;
    std::string content_type_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};

#endif // HTTP_SERVER_H