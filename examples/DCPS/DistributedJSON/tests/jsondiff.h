#ifndef JSONDIFF_HPP
#define JSONDIFF_HPP

#include <string>
#include <stdexcept>

/**
 * @brief A C++ wrapper for calling the jsondiffpatch Node.js library.
 */
namespace CppJsInterop {

    /**
     * @brief Computes the difference between two JSON strings.
     *
     * This function invokes an external Node.js script ('diff_wrapper.js')
     * to perform the diff operation using the 'jsondiffpatch' library.
     *
     * @param jsonA The first JSON string (the 'left' side).
     * @param jsonB The second JSON string (the 'right' side).
     * @param nodeScriptPath The relative path to the Node.js wrapper script.
     * Defaults to "diff_wrapper.js".
     * @return A string containing the JSON diff delta.
     * @throws std::runtime_error if the Node.js script fails to execute,
     * if it returns an error, or if JSON parsing fails.
     */
    std::string diff(
        const std::string& jsonA,
        const std::string& jsonB,
        const std::string& nodeScriptPath = "diff_wrapper.js"
    );

} // namespace CppJsInterop

#endif // JSONDIFF_HPP
