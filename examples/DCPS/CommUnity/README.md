# CommUnity Publisher

A federated data sharing application built on OpenDDS that enables organizations to securely share and consume Human Services Data Specification (HSDS) directories.

## Overview

The CommUnity Publisher is the writer component of the CommUnity federated architecture. It provides:

- **HTTP REST API** for submitting HSDS data
- **OpenDDS Publishing** to federate data across organizations
- **Data Validation** ensuring HSDS compliance
- **Security Integration** with DDS Security plugins
- **Non-repudiation** through DMP source identification

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Shared Infrastructure                      │
│  ┌──────────────────┐            ┌──────────────────┐      │
│  │  DDS Permissions │            │   RtpsRelay      │      │
│  │  Manager (DPM)   │            │   Service        │      │
│  └──────────────────┘            └──────────────────┘      │
└────────────┬──────────────────────────┬────────────────────┘
             │                          │
   ┌─────────┴─────────┐      ┌────────┴─────────┐
   │  Organization A   │      │  Organization B   │
   │                   │      │                   │
   │  ┌─────────────┐  │      │  ┌─────────────┐ │
   │  │   Writer    │  │      │  │   Writer    │ │
   │  │ Application │  │      │  │ Application │ │
   │  └─────────────┘  │      │  └─────────────┘ │
   └───────────────────┘      └───────────────────┘
```

## Building

### Prerequisites

- OpenDDS (with DDS Security support if using security features)
- ACE/TAO
- C++ compiler (GCC, Clang, or MSVC)
- Perl (for build system)

### Build Steps

1. **Generate Type Support**:
   ```bash
   # Generate IDL type support files
   opendds_idl hsds.idl
   ```

2. **Generate Build Files**:
   ```bash
   # Generate makefiles using MPC
   mwc.pl -type gnuace CommUnityPublisher.mpc
   ```

3. **Build**:
   ```bash
   make
   ```

## Configuration

### Basic Configuration

Create a YAML configuration file (default: `community_publisher_config.yaml`):

```yaml
writer:
  dmp_id: "your-organization-id"
  
  api:
    host: "0.0.0.0"
    port: 8080
    auth_token: "your-secure-token"
    
  dds:
    domain_id: 0
    rtps_relay:
      host: "relay.community.org"
      port: 4444
      
logging:
  level: INFO
  console:
    enabled: true
```

### Security Configuration

For production deployments with DDS Security:

```yaml
writer:
  security:
    enabled: true
    identity_ca: "/path/to/identity_ca.pem"
    identity_cert: "/path/to/your_identity_cert.pem"
    identity_key: "/path/to/your_identity_key.pem"
    permissions_ca: "/path/to/permissions_ca.pem"
    permissions_file: "/path/to/permissions.xml"
    governance_file: "/path/to/governance.xml"
```

## Usage

### Starting the Publisher

```bash
# Start with default configuration
./community_publisher

# Start with custom configuration
./community_publisher -c /path/to/config.yaml

# Start with custom DMP ID
./community_publisher -d "my-org-publisher"

# Show help
./community_publisher -h
```

### API Endpoints

#### Health Check
```bash
curl http://localhost:8080/api/v1/health
```

#### Status Information
```bash
curl http://localhost:8080/api/v1/status
```

#### Submit Organization Data
```bash
curl -X POST http://localhost:8080/api/v1/hsds/organization \
  -H "Authorization: Bearer your-auth-token" \
  -H "Content-Type: application/json" \
  -d '{
    "id": "org-123",
    "name": "Community Services",
    "description": "Local community support services",
    "email": "info@communityservices.org",
    "website": "https://www.communityservices.org"
  }'
```

#### Submit Service Data
```bash
curl -X POST http://localhost:8080/api/v1/hsds/service \
  -H "Authorization: Bearer your-auth-token" \
  -H "Content-Type: application/json" \
  -d '{
    "id": "service-456",
    "organization_id": "org-123",
    "name": "Food Assistance",
    "description": "Emergency food assistance program",
    "email": "food@communityservices.org"
  }'
```

#### Submit Program Data
```bash
curl -X POST http://localhost:8080/api/v1/hsds/program \
  -H "Authorization: Bearer your-auth-token" \
  -H "Content-Type: application/json" \
  -d '{
    "id": "program-789",
    "organization_id": "org-123",
    "name": "Emergency Services",
    "description": "Emergency assistance programs"
  }'
```

#### Submit Location Data
```bash
curl -X POST http://localhost:8080/api/v1/hsds/location \
  -H "Authorization: Bearer your-auth-token" \
  -H "Content-Type: application/json" \
  -d '{
    "id": "location-101",
    "organization_id": "org-123",
    "name": "Main Office",
    "description": "Primary service location",
    "latitude": 40.7128,
    "longitude": -74.0060
  }'
```

### Supported HSDS Tables

The publisher supports all HSDS 3.0 tables:

- `organization` - Organizations providing services
- `program` - Programs offered by organizations  
- `service` - Individual services provided
- `location` - Physical locations where services are offered
- `phone` - Phone numbers for organizations, services, locations
- `contact` - Contact information for staff
- `address` - Physical addresses
- `service_at_location` - Services available at specific locations
- `schedule` - Operating schedules
- `attribute` - Additional attributes and taxonomy terms
- `funding` - Funding sources
- `service_area` - Geographic service areas
- `required_document` - Documents required to access services
- `language` - Language support information
- `accessibility` - Accessibility accommodations
- `taxonomy_term` - Taxonomy classifications
- `metadata` - Change tracking metadata
- `cost_option` - Cost information for services
- `organization_identifier` - External identifiers for organizations
- `taxonomy` - Taxonomy definitions
- `service_capacity` - Service capacity and availability
- `unit` - Units of measurement
- `url` - Web links and URLs

## Data Validation

The publisher includes comprehensive HSDS validation:

- **Required Field Validation** - Ensures all mandatory fields are present
- **Format Validation** - Validates email addresses, URLs, phone numbers
- **Range Validation** - Checks numeric ranges (coordinates, dates, etc.)
- **Relationship Validation** - Validates foreign key relationships
- **Enum Validation** - Checks enumerated field values
- **Length Validation** - Enforces maximum field lengths

## Security Features

### Authentication
- Token-based API authentication
- Configurable authentication tokens

### DDS Security Integration
- Identity authentication via X.509 certificates
- Access control via permissions files
- Data encryption and signing
- Non-repudiation through DMP source identification

### Non-Repudiation
Every published data sample is automatically tagged with:
- **DMP Source ID** - Identifies the publishing organization
- **Timestamp** - When the data was published
- **Signature** - Cryptographic signature (when security enabled)

## Monitoring and Observability

### Logging
- Configurable log levels (DEBUG, INFO, WARN, ERROR, FATAL)
- Multiple output targets (console, file, syslog)
- Structured logging for operational monitoring

### Metrics
- Published message counts
- API request counts
- Error rates and types
- Connection status

### Health Checks
- `/api/v1/health` - Basic health status
- `/api/v1/status` - Detailed operational status

## Error Handling

The publisher provides comprehensive error handling:

- **HTTP API Errors** - Standard HTTP status codes with JSON error responses
- **DDS Errors** - Automatic retry and connection recovery
- **Validation Errors** - Detailed validation failure descriptions
- **Configuration Errors** - Clear configuration validation messages

## Deployment

### Docker Deployment
```dockerfile
FROM ubuntu:22.04

# Install OpenDDS and dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake libssl-dev libboost-all-dev

COPY --from=opendds-builder /opt/OpenDDS /opt/OpenDDS
ENV DDS_ROOT=/opt/OpenDDS

# Copy application
COPY build/community_publisher /usr/local/bin/
COPY config/community_publisher_config.yaml /etc/community/

VOLUME ["/etc/community/certs"]
EXPOSE 8080

CMD ["community_publisher", "-c", "/etc/community/community_publisher_config.yaml"]
```

### Resource Requirements
- **CPU**: 2+ cores recommended
- **Memory**: 4GB+ recommended  
- **Disk**: 20GB+ for logs and data
- **Network**: 100Mbps+ for federation

## Troubleshooting

### Common Issues

#### API Authentication Failures
- Verify `auth_token` in configuration matches request headers
- Check `Authorization: Bearer <token>` header format

#### DDS Connection Issues
- Verify `domain_id` matches other participants
- Check `rtps_relay` configuration for federated deployment
- Verify firewall rules allow DDS communication

#### Security Configuration
- Ensure certificate files exist and are readable
- Verify certificate validity and trust chain
- Check permissions file syntax and content

### Debug Mode
Enable debug logging for detailed troubleshooting:
```yaml
logging:
  level: DEBUG
```

## Development

### Code Structure
- `community_publisher.cpp` - Main application entry point
- `hsds_publisher.h/cpp` - DDS publishing logic
- `http_server.h/cpp` - HTTP API server
- `hsds_validator.h/cpp` - HSDS data validation
- `config.h/cpp` - Configuration management
- `hsds.idl` - HSDS data type definitions

### Adding New HSDS Tables
1. Update `hsds.idl` with new struct definition
2. Add TypeSupport registration in `hsds_publisher.cpp`
3. Add validation method in `hsds_validator.cpp`
4. Add JSON conversion method in `http_server.cpp`
5. Add API endpoint handler

## License

This software is part of the OpenDDS project and follows the same licensing terms.

## Support

For support and questions:
- OpenDDS documentation: https://opendds.org/
- Community forums and mailing lists
- Issue tracking via project repository

## Contributing

Contributions are welcome! Please follow OpenDDS contribution guidelines and ensure:
- Code follows project style standards
- Tests are included for new features
- Documentation is updated for API changes
- Security considerations are addressed