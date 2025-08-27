// -*- C++ -*-
// HTTP API Server Implementation

#include "http_server.h"
#include <ace/Log_Msg.h>
#include <ace/OS_NS_string.h>
#include <ace/OS_NS_stdio.h>
#include <sstream>
#include <algorithm>
#include <cstring>

using namespace std;

HttpServer::HttpServer(HsdsPublisher& publisher, const Config& config) 
    : publisher_(publisher), config_(config), validator_(), running_(false), request_count_(0) {
    auth_token_ = config_.getAuthToken();
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    if (running_) {
        ACE_DEBUG((LM_WARNING, ACE_TEXT("(%P|%t) HTTP server already running\n")));
        return true;
    }

    ACE_INET_Addr server_addr(config_.getApiPort(), config_.getApiHost().c_str());
    
    if (acceptor_.open(server_addr, 1) == -1) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to open HTTP server on %C:%d\n"), 
                  config_.getApiHost().c_str(), config_.getApiPort()));
        return false;
    }

    running_ = true;

    // Start the server thread
    if (activate(THR_NEW_LWP | THR_JOINABLE, 1) == -1) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Failed to activate HTTP server thread\n")));
        running_ = false;
        acceptor_.close();
        return false;
    }

    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) HTTP server started on %C:%d\n"), 
              config_.getApiHost().c_str(), config_.getApiPort()));
    return true;
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    acceptor_.close();
    
    // Wait for server thread to finish
    wait();
    
    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) HTTP server stopped\n")));
}

int HttpServer::svc() {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) HTTP server thread started\n")));
    
    while (running_) {
        ACE_SOCK_Stream client_stream;
        ACE_INET_Addr client_addr;
        
        ACE_Time_Value timeout(1, 0);  // 1 second timeout
        
        if (acceptor_.accept(client_stream, &client_addr, &timeout) == -1) {
            if (ACE_OS::last_error() == ETIME) {
                continue;  // Timeout, check if we should still be running
            }
            if (running_) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Error accepting client connection\n")));
            }
            continue;
        }
        
        if (!running_) {
            client_stream.close();
            break;
        }
        
        handleClient(client_stream);
        client_stream.close();
    }
    
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) HTTP server thread finished\n")));
    return 0;
}

void HttpServer::handleClient(ACE_SOCK_Stream& client_stream) {
    const size_t BUFFER_SIZE = 8192;
    char buffer[BUFFER_SIZE];
    
    // Read the HTTP request
    ssize_t bytes_received = client_stream.recv(buffer, BUFFER_SIZE - 1);
    if (bytes_received <= 0) {
        ACE_DEBUG((LM_WARNING, ACE_TEXT("(%P|%t) No data received from client\n")));
        return;
    }
    
    buffer[bytes_received] = '\0';
    string request_data(buffer);
    
    // Parse and handle the request
    HttpRequest request = parseRequest(request_data);
    logRequest(request);
    
    HttpResponse response = handleRequest(request);
    
    // Send the response
    string response_data = buildResponse(response);
    client_stream.send_n(response_data.c_str(), response_data.length());
    
    request_count_++;
}

HttpRequest HttpServer::parseRequest(const string& request_data) {
    HttpRequest request;
    
    istringstream iss(request_data);
    string line;
    
    // Parse request line
    if (getline(iss, line)) {
        istringstream request_line(line);
        string method_str, path, version;
        
        if (request_line >> method_str >> path >> version) {
            // Set method
            if (method_str == "GET") request.setMethod(HttpRequest::GET);
            else if (method_str == "POST") request.setMethod(HttpRequest::POST);
            else if (method_str == "PUT") request.setMethod(HttpRequest::PUT);
            else if (method_str == "DELETE") request.setMethod(HttpRequest::DELETE);
            
            request.setPath(path);
        }
    }
    
    // Parse headers
    while (getline(iss, line) && !line.empty() && line != "\r") {
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) {
            string name = line.substr(0, colon_pos);
            string value = line.substr(colon_pos + 1);
            
            // Trim whitespace
            name.erase(name.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
            request.setHeader(name, value);
        }
    }
    
    // Parse body (remaining content)
    ostringstream body_stream;
    while (getline(iss, line)) {
        body_stream << line << "\n";
    }
    request.setBody(body_stream.str());
    
    return request;
}

HttpResponse HttpServer::handleRequest(const HttpRequest& request) {
    vector<string> path_components = request.getPathComponents();
    
    // Health check endpoint
    if (request.getMethod() == HttpRequest::GET && 
        path_components.size() >= 3 && path_components[1] == "api" && 
        path_components[2] == "v1" && path_components.size() == 4 && 
        path_components[3] == "health") {
        return handleHealthCheck();
    }
    
    // Status endpoint
    if (request.getMethod() == HttpRequest::GET && 
        path_components.size() >= 3 && path_components[1] == "api" && 
        path_components[2] == "v1" && path_components.size() == 4 && 
        path_components[3] == "status") {
        return handleStatus();
    }
    
    // HSDS data endpoints
    if (path_components.size() >= 4 && path_components[1] == "api" && 
        path_components[2] == "v1" && path_components[3] == "hsds") {
        
        // Check authentication
        string auth_header = request.getHeader("Authorization");
        string expected_auth = "Bearer " + auth_token_;
        if (auth_header != expected_auth) {
            HttpResponse response;
            response.setErrorResponse(401, "Unauthorized");
            return response;
        }
        
        if (path_components.size() == 5) {
            // POST /api/v1/hsds/{table}
            if (request.getMethod() == HttpRequest::POST) {
                return handleSubmitData(path_components[4], request.getBody());
            }
        } else if (path_components.size() == 6) {
            // PUT /api/v1/hsds/{table}/{id}
            // DELETE /api/v1/hsds/{table}/{id}
            if (request.getMethod() == HttpRequest::PUT) {
                return handleUpdateData(path_components[4], path_components[5], request.getBody());
            } else if (request.getMethod() == HttpRequest::DELETE) {
                return handleDeleteData(path_components[4], path_components[5]);
            }
        }
    }
    
    // Not found
    HttpResponse response;
    response.setErrorResponse(404, "Not Found");
    return response;
}

string HttpServer::buildResponse(const HttpResponse& response) {
    ostringstream oss;
    
    // Status line
    oss << "HTTP/1.1 " << response.getStatus();
    switch (response.getStatus()) {
        case 200: oss << " OK"; break;
        case 201: oss << " Created"; break;
        case 400: oss << " Bad Request"; break;
        case 401: oss << " Unauthorized"; break;
        case 404: oss << " Not Found"; break;
        case 500: oss << " Internal Server Error"; break;
        default: oss << " Unknown"; break;
    }
    oss << "\r\n";
    
    // Headers
    oss << "Content-Type: " << response.getContentType() << "\r\n";
    oss << "Content-Length: " << response.getBody().length() << "\r\n";
    oss << "Connection: close\r\n";
    
    // Additional headers
    for (const auto& header : response.getHeaders()) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "\r\n";
    oss << response.getBody();
    
    return oss.str();
}

HttpResponse HttpServer::handleHealthCheck() {
    HttpResponse response;
    response.setJsonBody("{\"status\":\"healthy\",\"timestamp\":\"" + 
                        to_string(time(nullptr)) + "\"}");
    return response;
}

HttpResponse HttpServer::handleStatus() {
    HttpResponse response;
    
    ostringstream json;
    json << "{"
         << "\"publisher_status\":\"" << (publisher_.isInitialized() ? "initialized" : "not_initialized") << "\","
         << "\"published_messages\":" << publisher_.getPublishedMessageCount() << ","
         << "\"api_requests\":" << request_count_ << ","
         << "\"dmp_id\":\"" << config_.getDmpId() << "\""
         << "}";
    
    response.setJsonBody(json.str());
    return response;
}

HttpResponse HttpServer::handleSubmitData(const string& table_name, const string& json_data) {
    HttpResponse response;
    
    try {
        bool success = false;
        
        // Route to appropriate handler based on table name
        if (table_name == "organization") {
            HSDS::Organization org;
            if (jsonToOrganization(json_data, org) && validator_.validateOrganization(org)) {
                success = publisher_.publishOrganization(org);
            }
        } else if (table_name == "program") {
            HSDS::Program program;
            if (jsonToProgram(json_data, program) && validator_.validateProgram(program)) {
                success = publisher_.publishProgram(program);
            }
        } else if (table_name == "service") {
            HSDS::Service service;
            if (jsonToService(json_data, service) && validator_.validateService(service)) {
                success = publisher_.publishService(service);
            }
        } else if (table_name == "location") {
            HSDS::Location location;
            if (jsonToLocation(json_data, location) && validator_.validateLocation(location)) {
                success = publisher_.publishLocation(location);
            }
        }
        // Add more table handlers as needed...
        else {
            response.setErrorResponse(400, "Unknown table: " + table_name);
            return response;
        }
        
        if (success) {
            response.setStatus(201);
            response.setJsonBody("{\"status\":\"created\",\"message\":\"Data published successfully\"}");
        } else {
            response.setErrorResponse(500, "Failed to publish data");
        }
        
    } catch (const exception& e) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Exception handling submit request: %C\n"), e.what()));
        response.setErrorResponse(500, "Internal server error");
    }
    
    return response;
}

HttpResponse HttpServer::handleUpdateData(const string& table_name, const string& id, const string& json_data) {
    // For now, treat updates the same as submissions
    // In a full implementation, this would handle UPDATE operations
    return handleSubmitData(table_name, json_data);
}

HttpResponse HttpServer::handleDeleteData(const string& table_name, const string& id) {
    HttpResponse response;
    // DELETE operations are not fully implemented in this example
    // Would require maintaining state of published records
    response.setErrorResponse(501, "DELETE operations not implemented");
    return response;
}

// Simple JSON parsing helpers (would use proper JSON library in production)
string HttpServer::extractJsonValue(const string& json, const string& key) {
    string search_pattern = "\"" + key + "\"";
    size_t key_pos = json.find(search_pattern);
    if (key_pos == string::npos) {
        return "";
    }
    
    size_t colon_pos = json.find(":", key_pos);
    if (colon_pos == string::npos) {
        return "";
    }
    
    size_t value_start = json.find("\"", colon_pos);
    if (value_start == string::npos) {
        return "";
    }
    value_start++;  // Skip opening quote
    
    size_t value_end = json.find("\"", value_start);
    if (value_end == string::npos) {
        return "";
    }
    
    return json.substr(value_start, value_end - value_start);
}

// Basic JSON to HSDS conversions (simplified for demo)
bool HttpServer::jsonToOrganization(const string& json, HSDS::Organization& org) {
    try {
        org.id = CORBA::string_dup(extractJsonValue(json, "id").c_str());
        org.name = CORBA::string_dup(extractJsonValue(json, "name").c_str());
        org.description = CORBA::string_dup(extractJsonValue(json, "description").c_str());
        org.email = CORBA::string_dup(extractJsonValue(json, "email").c_str());
        org.website = CORBA::string_dup(extractJsonValue(json, "website").c_str());
        
        // Set defaults for required fields if not provided
        if (strlen(org.id) == 0) {
            org.id = CORBA::string_dup(generateId().c_str());
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool HttpServer::jsonToProgram(const string& json, HSDS::Program& program) {
    try {
        program.id = CORBA::string_dup(extractJsonValue(json, "id").c_str());
        program.organization_id = CORBA::string_dup(extractJsonValue(json, "organization_id").c_str());
        program.name = CORBA::string_dup(extractJsonValue(json, "name").c_str());
        program.description = CORBA::string_dup(extractJsonValue(json, "description").c_str());
        
        if (strlen(program.id) == 0) {
            program.id = CORBA::string_dup(generateId().c_str());
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool HttpServer::jsonToService(const string& json, HSDS::Service& service) {
    try {
        service.id = CORBA::string_dup(extractJsonValue(json, "id").c_str());
        service.organization_id = CORBA::string_dup(extractJsonValue(json, "organization_id").c_str());
        service.name = CORBA::string_dup(extractJsonValue(json, "name").c_str());
        service.description = CORBA::string_dup(extractJsonValue(json, "description").c_str());
        service.url = CORBA::string_dup(extractJsonValue(json, "url").c_str());
        service.email = CORBA::string_dup(extractJsonValue(json, "email").c_str());
        
        if (strlen(service.id) == 0) {
            service.id = CORBA::string_dup(generateId().c_str());
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool HttpServer::jsonToLocation(const string& json, HSDS::Location& location) {
    try {
        location.id = CORBA::string_dup(extractJsonValue(json, "id").c_str());
        location.organization_id = CORBA::string_dup(extractJsonValue(json, "organization_id").c_str());
        location.name = CORBA::string_dup(extractJsonValue(json, "name").c_str());
        location.description = CORBA::string_dup(extractJsonValue(json, "description").c_str());
        
        if (strlen(location.id) == 0) {
            location.id = CORBA::string_dup(generateId().c_str());
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

// Stub implementations for other JSON conversion methods
bool HttpServer::jsonToPhone(const string& json, HSDS::Phone& phone) { return false; }
bool HttpServer::jsonToContact(const string& json, HSDS::Contact& contact) { return false; }
bool HttpServer::jsonToAddress(const string& json, HSDS::Address& address) { return false; }
bool HttpServer::jsonToServiceAtLocation(const string& json, HSDS::ServiceAtLocation& sal) { return false; }
bool HttpServer::jsonToSchedule(const string& json, HSDS::Schedule& schedule) { return false; }
bool HttpServer::jsonToAttribute(const string& json, HSDS::AttributeInfo& attribute) { return false; }
bool HttpServer::jsonToFunding(const string& json, HSDS::Funding& funding) { return false; }
bool HttpServer::jsonToServiceArea(const string& json, HSDS::ServiceArea& service_area) { return false; }
bool HttpServer::jsonToRequiredDocument(const string& json, HSDS::RequiredDocument& doc) { return false; }
bool HttpServer::jsonToLanguage(const string& json, HSDS::Language& language) { return false; }
bool HttpServer::jsonToAccessibility(const string& json, HSDS::Accessibility& accessibility) { return false; }
bool HttpServer::jsonToTaxonomyTerm(const string& json, HSDS::TaxonomyTerm& taxonomy_term) { return false; }
bool HttpServer::jsonToMetadata(const string& json, HSDS::Metadata& metadata) { return false; }
bool HttpServer::jsonToMetaTableDescription(const string& json, HSDS::MetaTableDescription& meta_table) { return false; }
bool HttpServer::jsonToCostOption(const string& json, HSDS::CostOption& cost_option) { return false; }
bool HttpServer::jsonToOrganizationIdentifier(const string& json, HSDS::OrganizationIdentifier& org_id) { return false; }
bool HttpServer::jsonToTaxonomy(const string& json, HSDS::Taxonomy& taxonomy) { return false; }
bool HttpServer::jsonToServiceCapacity(const string& json, HSDS::ServiceCapacity& capacity) { return false; }
bool HttpServer::jsonToUnit(const string& json, HSDS::Unit& unit) { return false; }
bool HttpServer::jsonToUrl(const string& json, HSDS::UrlInfo& url) { return false; }

string HttpServer::generateId() {
    // Simple ID generation - in production would use UUID
    static int counter = 0;
    return "auto_" + to_string(time(nullptr)) + "_" + to_string(++counter);
}

void HttpServer::logRequest(const HttpRequest& request) {
    string method_str;
    switch (request.getMethod()) {
        case HttpRequest::GET: method_str = "GET"; break;
        case HttpRequest::POST: method_str = "POST"; break;
        case HttpRequest::PUT: method_str = "PUT"; break;
        case HttpRequest::DELETE: method_str = "DELETE"; break;
        default: method_str = "UNKNOWN"; break;
    }
    
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) HTTP %C %C\n"), 
              method_str.c_str(), request.getPath().c_str()));
}

// HttpRequest implementation
string HttpRequest::getHeader(const string& name) const {
    auto it = headers_.find(name);
    return (it != headers_.end()) ? it->second : "";
}

void HttpRequest::setHeader(const string& name, const string& value) {
    headers_[name] = value;
}

vector<string> HttpRequest::getPathComponents() const {
    vector<string> components;
    istringstream iss(path_);
    string component;
    
    while (getline(iss, component, '/')) {
        if (!component.empty()) {
            components.push_back(component);
        }
    }
    
    return components;
}

map<string, string> HttpRequest::getQueryParameters() const {
    map<string, string> params;
    size_t query_pos = path_.find('?');
    
    if (query_pos != string::npos) {
        string query = path_.substr(query_pos + 1);
        istringstream iss(query);
        string param;
        
        while (getline(iss, param, '&')) {
            size_t eq_pos = param.find('=');
            if (eq_pos != string::npos) {
                string key = param.substr(0, eq_pos);
                string value = param.substr(eq_pos + 1);
                params[key] = value;
            }
        }
    }
    
    return params;
}

// HttpResponse implementation
void HttpResponse::setHeader(const string& name, const string& value) {
    headers_[name] = value;
}

void HttpResponse::setJsonBody(const string& json) {
    setContentType("application/json");
    setBody(json);
}

void HttpResponse::setErrorResponse(int status, const string& message) {
    setStatus(status);
    setJsonBody("{\"error\":\"" + message + "\"}");
}