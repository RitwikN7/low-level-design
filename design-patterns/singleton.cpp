/*
    - Singleton classes are useful for global instances of config files, loggers etc.
    - Simplest implementation below (Meyer's Singleton)
*/

#include <string>

class DatabaseConnection
{
public:
    static DatabaseConnection& getInstance()
    {
        static DatabaseConnection instance;
        return instance;
    }

    void query(const std::string& sql)
    {
        // Database operations
    }

private:
    DatabaseConnection() = default;
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
};

// Usage
// auto& db = DatabaseConnection::getInstance();
// db.query("SELECT * FROM users");
