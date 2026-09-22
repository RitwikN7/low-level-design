/*
    - Builder pattern is useful when you need to construct objects in a flexible manner.
    - Use the final build() function to check for logical correctness.
*/

#include <map>
#include <stdexcept>
#include <string>

class HttpRequest
{
public:
    const std::string& getUrl() const
    {
        return url;
    }
    const std::string& getMethod() const
    {
        return method;
    }
    const std::map<std::string, std::string>& getHeaders() const
    {
        return headers;
    }
    const std::string& getBody() const
    {
        return body;
    }

    class Builder
    {
    public:
        Builder& url(const std::string& value)
        {
            request.url = value;
            return *this;
        }

        Builder& method(const std::string& value)
        {
            request.method = value;
            return *this;
        }

        Builder& header(const std::string& key, const std::string& value)
        {
            request.headers[key] = value;
            return *this;
        }

        Builder& body(const std::string& value)
        {
            request.body = value;
            return *this;
        }

        HttpRequest build()
        {
            if (request.url.empty())
            {
                throw std::invalid_argument("URL is required");
            }
            return request;
        }

    private:
        HttpRequest request;
    };

private:
    std::string url;
    std::string method;
    std::map<std::string, std::string> headers;
    std::string body;
};

// Usage
// HttpRequest request = HttpRequest::Builder()
//   .url("https://api.example.com")
//   .method("POST")
//   .header("Content-Type", "application/json")
//   .body("{\"key\": \"value\"}")
//   .build();
