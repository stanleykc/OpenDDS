Calling Javascript (Node.js) from C++ ExampleThis project demonstrates how to invoke a JavaScript library (jsondiffpatch) from a native C++ application using Inter-Process Communication (IPC).The C++ program executes a Node.js script as a child process, passes two JSON strings as command-line arguments, and reads the resulting JSON diff from the script's standard output.RequirementsA C++ Compiler: g++ (on Linux/macOS) or MSVC (on Windows).Node.js and npm: Make sure you have Node.js installed. You can get it from nodejs.org.Step-by-Step Setup and ExecutionStep 1: Set Up the Node.js EnvironmentFirst, you need to install the JavaScript dependency specified in package.json.Open your terminal or command prompt in the project directory (where package.json is located) and run:npm install
This command will read package.json and install the jsondiffpatch library into a node_modules folder.Step 2: Compile the C++ ProgramNext, compile the main.cpp file to create an executable.On Linux or macOS:Use the g++ compiler:g++ -std=c++11 -o main main.cpp
On Windows (using MinGW/g++):The command is the same as for Linux:g++ -std=c++11 -o main.exe main.cpp
On Windows (using Visual Studio C++ Compiler):Open a "Developer Command Prompt for VS" and use the cl compiler:cl /EHsc /std:c++14 main.cpp
This will create an executable file named main (or main.exe on Windows).Step 3: Run the C++ ProgramNow you can execute the compiled program. The C++ application will, in turn, execute the diff_wrapper.js script.On Linux or macOS:./main
On Windows:.\main.exe
Expected OutputYou should see output from the C++ program showing the two initial JSON objects, the command being executed, and finally, the diff result it received from the Node.js script.C++ Program: Initiating JavaScript diff...

---

JSON A: {"name": "John", "age": 30, "tasks": ["eat", "sleep"]}
JSON B: {"name": "John", "age": 31, "tasks": ["eat", "code", "sleep"]}

---

Executing command: node diff_wrapper.js "{\"name\": \"John\", \"age\": 30, \"tasks\": [\"eat\", \"sleep\"]}" "{\"name\": \"John\", \"age\": 31, \"tasks\": [\"eat\", \"code\", \"sleep\"]}"

SUCCESS: Received diff from JavaScript:
{
"age": [
30,
31
],
"tasks": {
"\_t": "a",
"1": [
"code"
]
}
}
This output confirms that the C++ program successfully called the JavaScript script, passed the data, and received the correct diff as a result.
