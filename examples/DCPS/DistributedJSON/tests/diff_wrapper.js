// This script acts as a bridge between the C++ application and the 'jsondiffpatch' library.
// It reads two JSON strings from the command-line arguments, computes the diff,
// and prints the resulting diff object as a JSON string to standard output.

// 1. Import the necessary library.
// Note: We use `require` as this is a standard Node.js script.
const jsondiffpatch = require("jsondiffpatch");

// 2. Create a jsondiffpatch instance.
// You can pass options here if needed, e.g., jsondiffpatch.create({ propertyFilter: ... });
const diffpatcher = jsondiffpatch.create();

// 3. Get the raw command-line arguments.
// process.argv is an array containing the command-line arguments.
// The first two elements are 'node' and the script path, so we slice them off.
const args = process.argv.slice(2);

if (args.length !== 2) {
  console.error("Error: Expected exactly two JSON string arguments.");
  process.exit(1); // Exit with a non-zero code to indicate an error
}

const [jsonStringA, jsonStringB] = args;

try {
  // 4. Parse the JSON strings into JavaScript objects.
  const objA = JSON.parse(jsonStringA);
  const objB = JSON.parse(jsonStringB);

  // 5. Compute the difference between the two objects.
  const delta = diffpatcher.diff(objA, objB);

  // 6. If a diff exists, print it to standard output as a formatted JSON string.
  // The C++ program will read this output.
  if (delta) {
    // Using JSON.stringify with indentation for readability
    console.log(JSON.stringify(delta, null, 2));
  } else {
    // If there's no difference, output an empty object or a specific message.
    console.log("{}");
  }
} catch (error) {
  // If JSON parsing fails or another error occurs, print it to standard error.
  console.error("Error processing JSON diff:", error.message);
  process.exit(1); // Exit with an error code
}
