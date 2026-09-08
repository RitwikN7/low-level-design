/*
Requirements:
1. Five severity levels: DEBUG < INFO < WARN < ERROR < FATAL.
2. Each record carries timestamp, level, message, emitting thread name.
3. Logger writes each record to one or more destinations, set at startup.
4. Each destination has its own min-level threshold and its own format.
   Format and destination type vary independently.
5. Concurrent calls are safe. A record's bytes never interleave with
   another record's bytes on the same destination.

Entities:
1. LogLevel enum
2. LogFormatter interface
3. LogAppender interface
    - MinLevel
    - Formatter
4. LogMessage
5. Logger
    - List<LogAppender>
    - createLogAppender()
    - debug()
    - info()
    - warn()
    - error()
    - fatal()
*/

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <source_location>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

constexpr std::string_view getLogLevelString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARN:
        return "WARN";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::FATAL:
        return "FATAL";
    }
    return "UNKNOWN";
}

struct LogMessage
{
    std::string message_{};
    std::string source_{};
    std::chrono::time_point<std::chrono::system_clock> timestamp_{};
    LogLevel level_{};
    std::thread::id thread_id_{};
};

class LogFormatter
{
public:
    virtual ~LogFormatter() = default;
    virtual std::string getFormatted(const LogMessage& message) const = 0;
};

class PrintFormatter : public LogFormatter
{
public:
    std::string getFormatted(const LogMessage& message) const override
    {
        std::ostringstream threadStream;
        threadStream << message.thread_id_;

        return std::format("[{}] [{}] [Thread-{}] {}: {}\n", message.timestamp_,
                           getLogLevelString(message.level_), threadStream.str(), message.source_,
                           message.message_);
    }
};

class JSONFormatter : public LogFormatter
{
public:
    std::string getFormatted(const LogMessage& message) const override
    {
        std::ostringstream threadStream;
        threadStream << message.thread_id_;

        return std::format("{{\"timestamp\":\"{}\", \"level\":\"{}\", \"thread\":\"{}\", "
                           "\"source\":\"{}\", \"message\":\"{}\"}}\n",
                           message.timestamp_, getLogLevelString(message.level_),
                           threadStream.str(), message.source_, message.message_);
    }
};

class LogAppender
{
public:
    LogAppender(std::unique_ptr<LogFormatter> formatter, LogLevel minLevel)
        : formatter_(std::move(formatter)),
          minLevel_(minLevel)
    {
    }

    virtual ~LogAppender() = default;

    void append(const LogMessage& message)
    {
        if (message.level_ < minLevel_)
            return;

        std::string formattedStr = formatter_->getFormatted(message);

        std::lock_guard<std::mutex> lock(mtx_);
        write(formattedStr);
    }

protected:
    virtual void write(const std::string& formattedString) = 0;

private:
    std::unique_ptr<LogFormatter> formatter_;
    LogLevel minLevel_;
    std::mutex mtx_;
};

class ConsoleAppender : public LogAppender
{
public:
    ConsoleAppender(std::unique_ptr<LogFormatter> formatter, LogLevel minLevel)
        : LogAppender(std::move(formatter), minLevel)
    {
    }

protected:
    void write(const std::string& formattedString) override { std::cout << formattedString; }
};

class FileAppender : public LogAppender
{
public:
    FileAppender(std::unique_ptr<LogFormatter> formatter, LogLevel minLevel,
                 const std::string& fileName)
        : LogAppender(std::move(formatter), minLevel),
          fileName_(fileName),
          file_(fileName, std::ios::app)
    {
    }

    ~FileAppender()
    {
        if (file_.is_open())
            file_.close();
    }

protected:
    void write(const std::string& formattedString) override
    {
        file_ << formattedString;
        file_.flush();
    }

private:
    std::string fileName_{};
    std::ofstream file_;
};

class Logger
{
public:
    void addAppender(std::unique_ptr<LogAppender> appender)
    {
        appenders_.push_back(std::move(appender));
    }

    void debug(const std::string& message,
               std::source_location loc = std::source_location::current())
    {
        log(LogLevel::DEBUG, message, loc);
    }

    void info(const std::string& message,
              std::source_location loc = std::source_location::current())
    {
        log(LogLevel::INFO, message, loc);
    }

    void warn(const std::string& message,
              std::source_location loc = std::source_location::current())
    {
        log(LogLevel::WARN, message, loc);
    }

    void error(const std::string& message,
               std::source_location loc = std::source_location::current())
    {
        log(LogLevel::ERROR, message, loc);
    }

    void fatal(const std::string& message,
               std::source_location loc = std::source_location::current())
    {
        log(LogLevel::FATAL, message, loc);
    }

private:
    std::vector<std::unique_ptr<LogAppender>> appenders_{};

    void log(LogLevel level, const std::string& message, const std::source_location& location)
    {
        LogMessage logMessage{message, location.function_name(), std::chrono::system_clock::now(),
                              level, std::this_thread::get_id()};

        for (const auto& appender : appenders_)
            appender->append(logMessage);
    }
};

int main()
{
    Logger logger;

    // 1. Setup Appenders (Demonstrating Dependency Injection & Open-Closed
    // Principle)

    // Console gets ALL messages from INFO and above, formatted as plain text
    logger.addAppender(
        std::make_unique<ConsoleAppender>(std::make_unique<PrintFormatter>(), LogLevel::INFO));

    // File gets ONLY ERROR and FATAL messages, formatted as JSON
    logger.addAppender(std::make_unique<FileAppender>(std::make_unique<JSONFormatter>(),
                                                      LogLevel::ERROR, "errors.json"));

    std::cout << "--- Starting Sequential Tests ---\n";

    // 2. Test Level Filtering
    logger.debug("This is a debug message. You should NOT see this anywhere.");
    logger.info("Application started. You should see this on the console ONLY.");
    logger.error("Failed to load config. You should see this on BOTH console and file.");

    std::cout << "\n--- Starting Concurrency Tests ---\n";

    // 3. Test Thread Safety (Demonstrating fine-grained locking)
    auto workerTask = [&logger](int workerId) {
        for (int i = 0; i < 3; ++i)
        {
            // Using std::format to dynamically create the message
            logger.warn(std::format("Worker {} processing task {}", workerId, i));

            // This will write to both the console and the JSON file
            // concurrently
            logger.fatal(std::format("Worker {} hit a fatal error on task {}", workerId, i));
        }
    };

    // Launch 4 concurrent threads
    std::vector<std::thread> threads;
    for (int i = 1; i <= 4; ++i)
    {
        threads.emplace_back(workerTask, i);
    }

    // Wait for all threads to finish
    for (auto& t : threads)
    {
        t.join();
    }

    std::cout << "\nExecution complete. Please check 'errors.json' for the "
                 "file output.\n";

    return 0;
}