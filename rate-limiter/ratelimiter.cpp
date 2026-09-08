/*
Requirements:
1. Algorithm config per endpoint received from API gateway (loaded once)
2. Default for endpoints not specified.
3. System receives {clientID (unique), endpoint}
4. System checks clientID against the endpoint's rate limiting algo
5. Return structured result (allowed / denied, remaining requests, try again
    after n seconds)
6. In-memory, single threaded for now

Core Entities:
    1. Endpoint
        - algo
        - algo params

    2. Client
        - clientID
        - current state of limits

    3. Request
        - requestID
        - clientID
        - endpoint
        - needs a response
        - timestamp

    4. Response
        - responseID
        - requestID
        - result (allowed, denied)
        - remaining limit
        - cooldown period (only if denied)

    5. RateLimiter
        - Polymorphic types (leaky bucket, sliding window)
        - stores map of endpoints and algos
        - stores clientIDs and their current limits per endpoint
        - checkLimits(request) -> returns response
            - checks which algo to use, default to sliding window
            - checks clientID limit against algo
            - returns response

*/

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

using namespace std;
using ParamsMap = unordered_map<string, string>;
using Timestamp = std::chrono::system_clock::time_point;

struct EndpointConfigs
{
    string name_;
    string algo_;
    ParamsMap algo_configs_;
};

struct Request
{
    string requestID_;
    string clientID_;
    string endpoint_;
    Timestamp timestamp_;
};

struct Response
{
    string requestID_;
    string responseID_;
    ParamsMap details_;
    bool result_;
};

class IRateLimiter
{
public:
    virtual ~IRateLimiter() = default;
    virtual Response allowRequest(const Request& request) = 0;
};

using AlgoCreator = function<unique_ptr<IRateLimiter>(const ParamsMap&)>;

class RateLimiterFactory
{
public:
    static RateLimiterFactory& getInstance()
    {
        static RateLimiterFactory instance;
        return instance;
    }

    void registerAlgo(const string& algo, AlgoCreator algoCreator)
    {
        algoRegistry_.emplace(algo, std::move(algoCreator));
    }

    unique_ptr<IRateLimiter> create(const string& algo, const ParamsMap& params)
    {
        auto iter = algoRegistry_.find(algo);
        if (iter == algoRegistry_.end())
            return nullptr;

        return iter->second(params);
    }

private:
    unordered_map<string, AlgoCreator> algoRegistry_;
};

struct TokenBucketParams
{
    int capacity_{100};
    int refillRate_{10};
};

class TokenBucket : public IRateLimiter
{
    struct ClientState;

public:
    TokenBucket(const TokenBucketParams& params)
        : capacity_(params.capacity_),
          refillRate_(params.refillRate_)
    {
    }

    static void registerFactory()
    {
        auto& factory = RateLimiterFactory::getInstance();
        factory.registerAlgo("TokenBucket", [](const ParamsMap& params) {
            int capacity = stoi(params.at("capacity"));
            int refillRate = stoi(params.at("refillRate"));
            TokenBucketParams tbParams{.capacity_ = capacity, .refillRate_ = refillRate};
            return make_unique<TokenBucket>(tbParams);
        });
    }

    Response allowRequest(const Request& request) override
    {
        auto now = request.timestamp_;
        const auto& clientID = request.clientID_;

        auto clientIter = clientStateMap_.find(clientID);
        if (clientIter == clientStateMap_.end())
            clientStateMap_.insert({
                clientID, {.tokenCount_ = capacity_, .lastActivity_ = now}
            });

        auto& clientState = clientStateMap_.at(clientID);

        // calculate tokens to add
        int duration =
            chrono::duration_cast<chrono::seconds>(now - clientState.lastActivity_).count();

        int tokens2Add = duration * refillRate_;
        if (tokens2Add > 0)
            clientState.tokenCount_ = min(capacity_, clientState.tokenCount_ + tokens2Add);

        clientState.lastActivity_ = now;

        Response response{};
        response.requestID_ = request.requestID_;

        // check token availability
        if (clientState.tokenCount_ > 0)
        {
            clientState.tokenCount_--;
            response.result_ = true;
            response.details_.insert({"remaining", to_string(clientState.tokenCount_)});
        }
        else
        {
            response.result_ = false;
            response.details_.insert({"remaining", "0"});
            double time2Refill = 1.0 / refillRate_;
            response.details_.insert({"retry_after_seconds", to_string(time2Refill)});
        }

        return response;
    }

private:
    struct ClientState
    {
        int tokenCount_{};
        Timestamp lastActivity_;
    };

    unordered_map<string, ClientState> clientStateMap_;
    int capacity_{};
    int refillRate_{};
};