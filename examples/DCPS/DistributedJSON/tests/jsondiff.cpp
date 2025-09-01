#include "jsondiff.h"

#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <sstream>

// --- Platform-specific includes for popen/pclose ---
#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif
// ---

namespace { // Use an anonymous namespace to keep these functions private to this file.

    /**
     * @brief Executes a command and captures its stdout.
     */
    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        // The '2>&1' part redirects stderr to stdout, so we can capture errors from the script.
        std::string cmd_with_redirect = std::string(cmd) + " 2>&1";
        std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(cmd_with_redirect.c_str(), "r"), PCLOSE);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    /**
     * @brief Escapes a string to be safely used as a command-line argument.
     */
    std::string escapeArgument(const std::string& s) {
        std::string escaped_s = "\"";
        for (char c : s) {
            if (c == '"' || c == '\\') {
                escaped_s += '\\';
            }
            escaped_s += c;
        }
        escaped_s += "\"";
        return escaped_s;
    }

} // end anonymous namespace

namespace CppJsInterop {

    std::string diff(const std::string& jsonA, const std::string& jsonB, const std::string& nodeScriptPath) {
        std::stringstream command_stream;
        command_stream << "node " << nodeScriptPath << " "
                       << escapeArgument(jsonA) << " "
                       << escapeArgument(jsonB);

        std::string result = exec(command_stream.str().c_str());

        // Check if the script output an error message
        if (result.rfind("Error:", 0) == 0) { // Check if the result string starts with "Error:"
            throw std::runtime_error("JavaScript Error: " + result);
        }

        return result;
    }

} // namespace CppJsInterop
