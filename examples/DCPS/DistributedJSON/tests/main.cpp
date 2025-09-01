#include "jsondiff.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>

// Helper function to read an entire file's content into a string.
// Throws a std::runtime_error if the file cannot be opened.
std::string readFile(const std::string& filename) {
    std::ifstream file_stream(filename);
    if (!file_stream.is_open()) {
        throw std::runtime_error("Error: Could not open file '" + filename + "'");
    }
    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    return buffer.str();
}

// This program demonstrates how to use the CppJsInterop library.
// It takes two command-line arguments: a source filename and a target filename.
// Each file should contain a JSON string.
int main(int argc, char* argv[]) {
    // 1. Check if the correct number of arguments are provided.
    // The program expects the executable name, followed by two filenames.
    if (argc != 3) {
        std::cerr << "Error: Invalid number of arguments." << std::endl;
        std::cerr << "Usage: " << argv[0] << " <source_file.json> <target_file.json>" << std::endl;
        std::cerr << "--------------------------------------------------------" << std::endl;
        std::cerr << "Example:" << std::endl;
        std::cerr << argv[0] << " source.json target.json" << std::endl;
        return 1; // Indicate incorrect usage
    }

    // 2. Assign command-line arguments to string variables for clarity.
    std::string source_filename = argv[1];
    std::string target_filename = argv[2];

    try {
        // 3. Read the JSON content from the specified files.
        std::string source_json = readFile(source_filename);
        std::string target_json = readFile(target_filename);

        std::cout << "C++ Program: Using the CppJsInterop library..." << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "Source File: " << source_filename << std::endl;
        std::cout << "Target File: " << target_filename << std::endl;
        std::cout << "------------------------------------------" << std::endl;

        // 4. Call the library function with the file contents.
        std::string diff_result = CppJsInterop::diff(source_json, target_json);

        // 5. Print the successful result to standard output.
        std::cout << "SUCCESS: Received diff from library:" << std::endl;
        std::cout << diff_result << std::endl;

    } catch (const std::runtime_error& e) {
        // 6. Catch and handle any errors (e.g., from file reading or process execution).
        std::cerr << "ERROR: Failed to get JSON diff." << std::endl;
        std::cerr << e.what() << std::endl;
        return 1; // Indicate failure
    }

    return 0; // Indicate success
}

