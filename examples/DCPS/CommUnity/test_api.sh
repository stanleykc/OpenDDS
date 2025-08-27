#!/bin/bash
# Test script for CommUnity Publisher API
# Demonstrates basic usage of the HTTP API

set -e

# Configuration
API_HOST="localhost"
API_PORT="8080"
AUTH_TOKEN="secure_token_change_me_in_production"
BASE_URL="http://${API_HOST}:${API_PORT}/api/v1"

echo "Testing CommUnity Publisher API at ${BASE_URL}"
echo "================================================"

# Test health check
echo "1. Testing health check..."
curl -s "${BASE_URL}/health" | jq .
echo

# Test status
echo "2. Testing status..."
curl -s "${BASE_URL}/status" | jq .
echo

# Test organization submission
echo "3. Submitting organization data..."
ORG_DATA='{
  "id": "org-test-123",
  "name": "Test Community Services",
  "description": "A test organization for demonstrating the CommUnity Publisher",
  "email": "info@testcommunity.org",
  "website": "https://www.testcommunity.org",
  "legal_status": "nonprofit"
}'

RESPONSE=$(curl -s -X POST "${BASE_URL}/hsds/organization" \
  -H "Authorization: Bearer ${AUTH_TOKEN}" \
  -H "Content-Type: application/json" \
  -d "${ORG_DATA}")

echo "Organization submission response: ${RESPONSE}"
echo

# Test program submission
echo "4. Submitting program data..."
PROGRAM_DATA='{
  "id": "program-test-456",
  "organization_id": "org-test-123",
  "name": "Test Emergency Services",
  "description": "Emergency assistance programs for testing"
}'

RESPONSE=$(curl -s -X POST "${BASE_URL}/hsds/program" \
  -H "Authorization: Bearer ${AUTH_TOKEN}" \
  -H "Content-Type: application/json" \
  -d "${PROGRAM_DATA}")

echo "Program submission response: ${RESPONSE}"
echo

# Test service submission
echo "5. Submitting service data..."
SERVICE_DATA='{
  "id": "service-test-789",
  "organization_id": "org-test-123",
  "program_id": "program-test-456",
  "name": "Test Food Assistance",
  "description": "Emergency food assistance program for testing",
  "email": "food@testcommunity.org",
  "status": "active"
}'

RESPONSE=$(curl -s -X POST "${BASE_URL}/hsds/service" \
  -H "Authorization: Bearer ${AUTH_TOKEN}" \
  -H "Content-Type: application/json" \
  -d "${SERVICE_DATA}")

echo "Service submission response: ${RESPONSE}"
echo

# Test location submission
echo "6. Submitting location data..."
LOCATION_DATA='{
  "id": "location-test-101",
  "organization_id": "org-test-123",
  "name": "Test Main Office",
  "description": "Primary service location for testing",
  "latitude": 40.7128,
  "longitude": -74.0060,
  "transportation": "Accessible by public transit"
}'

RESPONSE=$(curl -s -X POST "${BASE_URL}/hsds/location" \
  -H "Authorization: Bearer ${AUTH_TOKEN}" \
  -H "Content-Type: application/json" \
  -d "${LOCATION_DATA}")

echo "Location submission response: ${RESPONSE}"
echo

# Test unauthorized request
echo "7. Testing unauthorized request (should fail)..."
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "${BASE_URL}/hsds/organization" \
  -H "Content-Type: application/json" \
  -d "${ORG_DATA}")

echo "Unauthorized response: ${RESPONSE}"
echo

# Test invalid data (should fail validation)
echo "8. Testing invalid data submission (should fail validation)..."
INVALID_DATA='{
  "name": "Missing ID Organization",
  "email": "invalid-email"
}'

RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "${BASE_URL}/hsds/organization" \
  -H "Authorization: Bearer ${AUTH_TOKEN}" \
  -H "Content-Type: application/json" \
  -d "${INVALID_DATA}")

echo "Invalid data response: ${RESPONSE}"
echo

# Final status check
echo "9. Final status check..."
curl -s "${BASE_URL}/status" | jq .
echo

echo "================================================"
echo "API testing complete!"
echo "Check the publisher logs for detailed information about the published data."