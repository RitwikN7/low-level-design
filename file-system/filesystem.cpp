/*
Design an in-memory file system

Features:
- single root starting at "/"
- allow creating, deleting, moving and renaming files and directories
- allow subfolders, and viewing contents of a folder
- navigate and resolve absolute path of file / folder
- files store string content
- throw exceptions for invalid operations
- scale can be 10K files and folders

Core Entities:
- FileSystemNode
    - File (name, content : string)
    - Folder (name, list<FileSystemNode>)

- FileSystemManager
    - createFile
    - createFolder
    - move
    - delete
    - list
*/

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class Folder;

class FileSystemNode
{
public:
    FileSystemNode(std::string name, Folder* parent = nullptr)
        : name_(std::move(name)),
          parent_(parent)
    {
    }

    [[nodiscard]] const std::string& getName() const
    {
        return name_;
    }

    [[nodiscard]] Folder* getParent() const
    {
        return parent_;
    }

    void setName(const std::string& newName)
    {
        name_ = newName;
    }

    void setParent(Folder* newFolder)
    {
        parent_ = newFolder;
    }

    std::string resolvePath() const;

    [[nodiscard]] virtual bool isFolder() const = 0;
    virtual ~FileSystemNode() = default;

protected:
    std::string name_{};
    Folder* parent_{nullptr};
};

class File : public FileSystemNode
{
public:
    File(const std::string& name, std::string content = "", Folder* parent = nullptr)
        : FileSystemNode(name, parent),
          content_(std::move(content))
    {
    }

    void editContent(const std::string& newContent)
    {
        content_ = newContent;
    }

    void appendContent(const std::string& newContent)
    {
        content_ += newContent;
    }

    [[nodiscard]] bool isFolder() const override
    {
        return false;
    }

private:
    std::string content_;
};

class Folder : public FileSystemNode
{
public:
    Folder(const std::string& name, Folder* parent = nullptr)
        : FileSystemNode(name, parent)
    {
    }

    void addNode(std::unique_ptr<FileSystemNode> node)
    {
        const auto& name = node->getName();
        auto iter = children_.find(name);
        if (iter != children_.end())
            throw std::invalid_argument("File / Folder with name already exists");

        children_.insert({name, std::move(node)});
    }

    void removeNode(const std::string& name)
    {
        auto iter = children_.find(name);
        if (iter == children_.end())
            throw std::invalid_argument("File / Folder with name does not exist");

        children_.erase(iter);
    }

    void renameNode(const std::string& oldName, const std::string& newName)
    {
        auto iterOldName = children_.find(oldName);
        if (iterOldName == children_.end())
            throw std::invalid_argument("File / Folder with name does not exist");

        auto iterNewName = children_.find(newName);
        if (iterNewName != children_.end())
            throw std::invalid_argument("Cannot rename. File / Folder already exists");

        iterOldName->second->setName(newName);
        children_.insert({newName, std::move(iterOldName->second)});
        children_.erase(iterOldName);
    }

    FileSystemNode* getChild(const std::string& name)
    {
        auto iter = children_.find(name);
        if (iter == children_.end())
            return nullptr;

        return iter->second.get();
    }

    bool isFolder() const override
    {
        return true;
    }

    void listContents() const
    {
        std::stringstream contents{};
        for (const auto& iter : children_)
        {
            contents << iter.first << " ";
        }

        std::cout << contents.str();
    }

private:
    std::map<std::string, std::unique_ptr<FileSystemNode>> children_{};
};

std::string FileSystemNode::resolvePath() const
{
    std::vector<std::string> res{name_};
    Folder* curr = parent_;
    while (curr != nullptr)
    {
        res.push_back(curr->getName());
        curr = curr->getParent();
    }

    std::stringstream path{};
    for (auto iter = res.rbegin(); iter != res.rend(); ++iter)
    {
        path << *iter;
        if (*iter != "/")
            path << "/";
    }

    return path.str();
}

class FileSystemManager
{
public:
    FileSystemManager()
        : root_(std::make_unique<Folder>("/"))
    {
    }

    void addFile(const std::string& path, const std::string& content)
    {
        Folder* parent = resolveParent(path);
        if (!parent)
            throw std::invalid_argument("Invalid path");

        std::string filename = tokenizePath(path).back();
        std::unique_ptr<File> newFile = std::make_unique<File>(filename, content, parent);
        parent->addNode(std::move(newFile));
    }

    void addFolder(const std::string& path)
    {
        Folder* parent = resolveParent(path);
        if (!parent)
            throw std::invalid_argument("Invalid path");

        std::string folderName = tokenizePath(path).back();
        std::unique_ptr<Folder> newFolder = std::make_unique<Folder>(folderName, parent);
        parent->addNode(std::move(newFolder));
    }

    void listContents(const std::string& path)
    {
        FileSystemNode* node = resolveNode(path);
        if (!node || !node->isFolder())
            throw std::invalid_argument("Invalid path");

        auto* folder = static_cast<Folder*>(node);
        folder->listContents();
    }

private:
    static std::vector<std::string> tokenizePath(const std::string& path)
    {
        std::vector<std::string> tokens{};
        if (path.empty() || path[0] != '/')
            return tokens;

        tokens.emplace_back("/");

        std::string curr{};
        for (int i = 1; i <= path.size(); i++)
        {
            if (i == path.size() || path[i] == '/')
            {
                tokens.push_back(curr);
                curr.clear();
            }
            else
                curr.push_back(path[i]);
        }

        return tokens;
    }

    FileSystemNode* resolveNode(const std::string& path)
    {
        std::vector<std::string> nodes = tokenizePath(path);
        if (nodes.empty())
            return nullptr;

        Folder* curr = root_.get();
        for (int i = 1; i < nodes.size(); i++)
        {
            FileSystemNode* nextNode = curr->getChild(nodes[i]);

            if (!nextNode)
                return nullptr;

            if (i == nodes.size() - 1)
                return nextNode;

            if (!nextNode->isFolder())
                return nullptr;

            curr = static_cast<Folder*>(nextNode);
        }

        return nullptr;
    }

    Folder* resolveParent(const std::string& path)
    {
        std::vector<std::string> nodes = tokenizePath(path);
        if (nodes.size() < 2)
            return nullptr;

        Folder* curr = root_.get();
        for (int i = 1; i < nodes.size(); i++)
        {
            FileSystemNode* nextNode = curr->getChild(nodes[i]);

            if (!nextNode)
                return nullptr;

            if (i == nodes.size() - 2)
                return static_cast<Folder*>(nextNode);

            if (!nextNode->isFolder())
                return nullptr;

            curr = static_cast<Folder*>(nextNode);
        }

        return nullptr;
    }

    std::unique_ptr<Folder> root_{nullptr};
};