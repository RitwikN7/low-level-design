#include <memory>
#include <stdexcept>
#include <string>

class Notification
{
public:
    virtual ~Notification() = default;
    virtual void send(const std::string& message) = 0;
};

class EmailNotification : public Notification
{
public:
    void send(const std::string& message) override
    {
        // Email sending logic
    }
};

class SMSNotification : public Notification
{
public:
    void send(const std::string& message) override
    {
        // SMS sending logic
    }
};

class NotificationFactory
{
public:
    static std::unique_ptr<Notification> create(const std::string& type)
    {
        if (type == "email")
        {
            return std::make_unique<EmailNotification>();
        }
        if (type == "sms")
        {
            return std::make_unique<SMSNotification>();
        }
        throw std::invalid_argument("Unknown type");
    }
};

// Usage
// auto notif = NotificationFactory::create("email");
// notif->send("Hello");
