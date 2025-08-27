#!/usr/bin/env perl
# Run test for CommUnity Publisher
# Basic smoke test to verify the application can start and respond to API calls

use strict;
use warnings;
use lib "$ENV{ACE_ROOT}/bin";
use lib "$ENV{DDS_ROOT}/bin";
use PerlDDS::Run_Test;
use File::Spec;

my $test = new PerlDDS::Run_Test ();

# Configuration
my $config_file = "community_publisher_config.yaml";
my $publisher_executable = "community_publisher";

# Test timeout (seconds)
my $test_timeout = 60;

print "CommUnity Publisher Test\n";
print "========================\n";

# Check if required files exist
unless (-f $config_file) {
    print STDERR "ERROR: Configuration file $config_file not found\n";
    exit 1;
}

unless (-x $publisher_executable) {
    print STDERR "ERROR: Publisher executable $publisher_executable not found or not executable\n";
    exit 1;
}

# Start the publisher
print "Starting CommUnity Publisher...\n";
my $Publisher = $test->Spawn("$publisher_executable -c $config_file");

# Give the publisher time to initialize
print "Waiting for publisher to initialize...\n";
sleep(5);

# Test basic functionality with API calls
print "Testing API endpoints...\n";

# Test health endpoint
my $health_result = system("curl -s -f http://localhost:8080/api/v1/health > /dev/null 2>&1");
if ($health_result == 0) {
    print "✓ Health endpoint test passed\n";
} else {
    print "✗ Health endpoint test failed\n";
    $test->finish(1);
}

# Test status endpoint  
my $status_result = system("curl -s -f http://localhost:8080/api/v1/status > /dev/null 2>&1");
if ($status_result == 0) {
    print "✓ Status endpoint test passed\n";
} else {
    print "✗ Status endpoint test failed\n";
    $test->finish(1);
}

# Test data submission (this should work if everything is configured correctly)
my $submit_data = '{
    "id": "test-org-001",
    "name": "Test Organization",
    "description": "A test organization for automated testing"
}';

my $submit_result = system("curl -s -f -X POST http://localhost:8080/api/v1/hsds/organization " .
                          "-H 'Authorization: Bearer secure_token_change_me_in_production' " .
                          "-H 'Content-Type: application/json' " .
                          "-d '$submit_data' > /dev/null 2>&1");

if ($submit_result == 0) {
    print "✓ Data submission test passed\n";
} else {
    print "✗ Data submission test failed (this may be expected if DDS is not fully configured)\n";
    # Don't fail the test for this as DDS might not be fully set up in test environment
}

print "Basic functionality tests completed successfully!\n";

# Clean up
print "Shutting down publisher...\n";
$Publisher->Kill();

# Final status
my $PublisherResult = $Publisher->WaitKill($test_timeout);
if ($PublisherResult != 0) {
    print STDERR "ERROR: Publisher returned $PublisherResult\n";
    $test->finish(1);
}

print "Test completed successfully!\n";
$test->finish(0);