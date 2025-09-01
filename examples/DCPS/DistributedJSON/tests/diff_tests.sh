#!/bin/bash

APP="./main"

echo "Semantic Difference Tests for JSON Diff Application"
echo "---------------------------------------------------"

# Test 1: Identical files
echo '{"a":1,"b":2}' > test1_left.json
echo '{"a":1,"b":2}' > test1_right.json
echo "Test 1: Identical files"
$APP test1_left.json test1_right.json > identical.out

# Test 2: Value changed
echo '{"a":1,"b":2}' > test2_left.json
echo '{"a":1,"b":3}' > test2_right.json
echo "Test 2: Value changed"
$APP test2_left.json test2_right.json

# Test 3: Key added
echo '{"a":1}' > test3_left.json
echo '{"a":1,"b":2}' > test3_right.json
echo "Test 3: Key added"
$APP test3_left.json test3_right.json

# Test 4: Key removed
echo '{"a":1,"b":2}' > test4_left.json
echo '{"a":1}' > test4_right.json
echo "Test 4: Key removed"
$APP test4_left.json test4_right.json

# Test 5: Nested value changed
echo '{"a":{"b":1}}' > test5_left.json
echo '{"a":{"b":2}}' > test5_right.json
echo "Test 5: Nested value changed"
$APP test5_left.json test5_right.json

# Test 6: Array difference
echo '{"a":[1,2,3]}' > test6_left.json
echo '{"a":[1,2,4]}' > test6_right.json
echo "Test 6: Array difference"
$APP test6_left.json test6_right.json

# Cleanup
rm test*_left.json test*_right.json
echo "Tests completed and temporary files removed."
echo "---------------------------------------------------"
