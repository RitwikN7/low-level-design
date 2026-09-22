/*
    - Use the policy pattern as an alternative to polymorphism when behavior is fixed at
   instantiation
*/

#include <fstream>
#include <iostream>
#include <string>

// Policy 1: Output to Console
struct ConsoleOutput
{
    static void write(const std::string& msg)
    {
        std::cout << msg << std::endl;
    }
};

// Policy 2: Output to a File
struct FileOutput
{
    static void write(const std::string& msg)
    {
        std::ofstream file("app.log", std::ios::app);
        if (file.is_open())
        {
            file << msg << std::endl;
        }
    }
};

// Policy 1: Plain Text Formatting
struct PlainFormat
{
    static std::string format(const std::string& msg)
    {
        return msg;
    }
};

// Policy 2: Verbose/Tag Formatting
struct VerboseFormat
{
    static std::string format(const std::string& msg)
    {
        return "[LOG] " + msg;
    }
};

template <typename OutputPolicy, typename FormatPolicy>
class Logger
{
public:
    void log(const std::string& message)
    {
        // Combining behaviors derived from the template parameters
        std::string formatted = FormatPolicy::format(message);
        OutputPolicy::write(formatted);
    }
};

/*
int main() {
    // A logger that prints verbose text to the console
    Logger<ConsoleOutput, VerboseFormat> consoleLogger;
    consoleLogger.log("Application started"); // Outputs: [LOG] Application started

    // A logger that writes plain text to a file
    Logger<FileOutput, PlainFormat> fileLogger;
    fileLogger.log("Data processed successfully"); // Appends "Data processed successfully" to
app.log
}
*/
