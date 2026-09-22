/*
    - Use decorator pattern to modify behavior at runtime by chaining rather than changing
*/

#include <memory>
#include <string>
#include <utility>

class DataSource
{
public:
    virtual ~DataSource() = default;
    virtual void writeData(const std::string& data) = 0;
    virtual std::string readData() = 0;
};

class FileDataSource : public DataSource
{
public:
    explicit FileDataSource(std::string filename)
        : filename(std::move(filename))
    {
    }

    void writeData(const std::string& data) override
    {
        // Write to file
    }

    std::string readData() override
    {
        // Read from file
        return "data from file";
    }

private:
    std::string filename;
};

class EncryptionDecorator : public DataSource
{
public:
    explicit EncryptionDecorator(std::shared_ptr<DataSource> source)
        : wrapped(std::move(source))
    {
    }

    void writeData(const std::string& data) override
    {
        auto encrypted = encrypt(data);
        wrapped->writeData(encrypted);
    }

    std::string readData() override
    {
        auto data = wrapped->readData();
        return decrypt(data);
    }

private:
    std::string encrypt(const std::string& data)
    {
        return "encrypted:" + data;
    }
    std::string decrypt(const std::string& data)
    {
        return data.substr(10);
    }

    std::shared_ptr<DataSource> wrapped;
};

class CompressionDecorator : public DataSource
{
public:
    explicit CompressionDecorator(std::shared_ptr<DataSource> source)
        : wrapped(std::move(source))
    {
    }

    void writeData(const std::string& data) override
    {
        auto compressed = compress(data);
        wrapped->writeData(compressed);
    }

    std::string readData() override
    {
        auto data = wrapped->readData();
        return decompress(data);
    }

private:
    std::string compress(const std::string& data)
    {
        return "compressed:" + data;
    }
    std::string decompress(const std::string& data)
    {
        return data.substr(11);
    }

    std::shared_ptr<DataSource> wrapped;
};

// Usage
// std::shared_ptr<DataSource> source = std::make_shared<FileDataSource>("data.txt");
// source = std::make_shared<EncryptionDecorator>(source);
// source = std::make_shared<CompressionDecorator>(source);
// source->writeData("sensitive info");
