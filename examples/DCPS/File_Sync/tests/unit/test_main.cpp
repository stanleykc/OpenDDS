#include <iostream>
#include <vector>
#include <string>
#include <functional>

/**
 * Simple unit test framework for File_Sync application.
 * This provides basic test registration and execution without external dependencies.
 */

struct TestCase {
  std::string name;
  std::function<bool()> test_function;
};

class TestRunner {
public:
  static TestRunner& instance() {
    static TestRunner instance;
    return instance;
  }

  void register_test(const std::string& name, std::function<bool()> test_func) {
    tests_.emplace_back(TestCase{name, test_func});
  }

  int run_all_tests() {
    int passed = 0;
    int failed = 0;

    std::cout << "Running " << tests_.size() << " test(s)...\n" << std::endl;

    for (const auto& test : tests_) {
      std::cout << "Running test: " << test.name << "... ";
      
      try {
        if (test.test_function()) {
          std::cout << "PASSED" << std::endl;
          ++passed;
        } else {
          std::cout << "FAILED" << std::endl;
          ++failed;
        }
      } catch (const std::exception& e) {
        std::cout << "FAILED (exception: " << e.what() << ")" << std::endl;
        ++failed;
      } catch (...) {
        std::cout << "FAILED (unknown exception)" << std::endl;
        ++failed;
      }
    }

    std::cout << "\nTest Results:" << std::endl;
    std::cout << "  Passed: " << passed << std::endl;
    std::cout << "  Failed: " << failed << std::endl;
    std::cout << "  Total:  " << (passed + failed) << std::endl;

    return (failed == 0) ? 0 : 1;
  }

private:
  std::vector<TestCase> tests_;
};

// Macros for easier test registration
#define TEST(test_name) \
  bool test_##test_name(); \
  static struct test_##test_name##_registrar { \
    test_##test_name##_registrar() { \
      TestRunner::instance().register_test(#test_name, test_##test_name); \
    } \
  } test_##test_name##_registrar_instance; \
  bool test_##test_name()

#define ASSERT_TRUE(condition) \
  do { \
    if (!(condition)) { \
      std::cerr << "ASSERTION FAILED: " << #condition \
                << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
      return false; \
    } \
  } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ(expected, actual) \
  do { \
    if ((expected) != (actual)) { \
      std::cerr << "ASSERTION FAILED: Expected " << (expected) \
                << ", got " << (actual) \
                << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
      return false; \
    } \
  } while (0)

#define ASSERT_NE(unexpected, actual) \
  do { \
    if ((unexpected) == (actual)) { \
      std::cerr << "ASSERTION FAILED: Expected not " << (unexpected) \
                << ", but got " << (actual) \
                << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
      return false; \
    } \
  } while (0)

// Forward declarations for test functions
bool test_ConfigurationManager();
bool test_FileManager();

int main(int argc, char* argv[]) {
  std::cout << "File_Sync Unit Tests" << std::endl;
  std::cout << "===================" << std::endl;

  return TestRunner::instance().run_all_tests();
}