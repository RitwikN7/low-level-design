#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/*
    Entities:
    - library (driver class)
    - book (metadata)
    - book_copy (physical book)
    - racks (1 to n)
    - user
*/

class User
{
public:
    User(std::string name, std::string userID)
        : name_(std::move(name)),
          userID_(std::move(userID))
    {
        std::cout << "Created user with name [" << name_ << "] and ID [" << userID_ << "]\n";
    }

    const std::map<std::string, std::string>& getBorrowed() const
    {
        return borrowed_;
    }

    void addBorrowedBookCopy(const std::string& bookCopyID, const std::string& dueDate)
    {
        borrowed_.insert({bookCopyID, dueDate});
    }

    void removeBorrowedBookCopy(const std::string& bookCopyID)
    {
        borrowed_.erase(bookCopyID);
    }

private:
    std::string name_;
    std::string userID_;
    std::map<std::string, std::string> borrowed_;
};

using HUser = std::shared_ptr<User>;

class Book
{
public:
    Book(std::string title, std::string bookID, const std::unordered_set<std::string>& authors,
         const std::unordered_set<std::string>& publishers)
        : title_(std::move(title)),
          bookID_(std::move(bookID)),
          authors_(authors),
          publishers_(publishers)
    {
    }

    const std::string& getTitle() const
    {
        return title_;
    }

    const std::string& getID() const
    {
        return bookID_;
    }

    const std::unordered_set<std::string>& getAuthors() const
    {
        return authors_;
    }

    const std::unordered_set<std::string>& getPublishers() const
    {
        return publishers_;
    }

    const std::set<std::string>& getBookCopies() const
    {
        return bookCopies_;
    }

    void addBookCopy(const std::string& bookCopyID)
    {
        bookCopies_.insert(bookCopyID);
    }

    void removeBookCopy(const std::string& bookCopyID)
    {
        bookCopies_.erase(bookCopyID);
    }

private:
    std::string title_{};
    std::string bookID_{};
    std::unordered_set<std::string> authors_{};
    std::unordered_set<std::string> publishers_{};
    std::set<std::string> bookCopies_{};
};

using HBook = std::shared_ptr<Book>;

class BookCopy
{
public:
    BookCopy(const std::string& bookCopyID, HBook& book, int assignedRack)
        : bookCopyID_(bookCopyID),
          book_(book),
          assignedRack_(assignedRack)
    {
    }

    const int getAssignedRack() const
    {
        return assignedRack_;
    }

    void registerReturn(int rack)
    {
        assignedRack_ = rack;
        borrowerID_ = "";
    }

    void registerBorrowed(const std::string& userID)
    {
        assignedRack_ = -1;
        borrowerID_ = userID;
    }

    const std::string& getBorrowerID() const
    {
        return borrowerID_;
    }

    HBook& getBook()
    {
        return book_;
    }

private:
    std::string bookCopyID_{};
    std::string borrowerID_{};
    HBook book_{nullptr};
    int assignedRack_{-1};
};

using HBookCopy = std::shared_ptr<BookCopy>;

class BookSearchStrategy
{
public:
    virtual ~BookSearchStrategy() = default;

    virtual std::vector<HBook>
    matches(const std::unordered_map<std::string, HBook>& books) const = 0;
};

class BookSearchByID : public BookSearchStrategy
{
public:
    explicit BookSearchByID(const std::string& bookID)
        : bookID_(bookID)
    {
    }

    std::vector<HBook> matches(const std::unordered_map<std::string, HBook>& books) const override
    {
        std::vector<HBook> result{};
        auto iter = books.find(bookID_);
        if (iter != books.end())
            result.push_back(iter->second);

        return result;
    }

private:
    std::string bookID_{};
};

class BookSearchByAuthor : public BookSearchStrategy
{
public:
    explicit BookSearchByAuthor(const std::string& author)
        : author_(author)
    {
    }

    std::vector<HBook> matches(const std::unordered_map<std::string, HBook>& books) const override
    {
        std::vector<HBook> result{};
        for (const auto& entry : books)
        {
            const auto& book = entry.second;
            if (book->getAuthors().find(author_) != book->getAuthors().end())
                result.push_back(book);
        }

        return result;
    }

private:
    std::string author_{};
};

class BookSearchByPublisher : public BookSearchStrategy
{
public:
    explicit BookSearchByPublisher(const std::string& publisher)
        : publisher_(publisher)
    {
    }

    std::vector<HBook> matches(const std::unordered_map<std::string, HBook>& books) const override
    {
        std::vector<HBook> result{};
        for (const auto& entry : books)
        {
            const auto& book = entry.second;
            if (book->getPublishers().find(publisher_) != book->getPublishers().end())
                result.push_back(book);
        }

        return result;
    }

private:
    std::string publisher_{};
};

using HBookSearchStrategy = std::unique_ptr<BookSearchStrategy>;

class LibraryManager
{
public:
    LibraryManager(const int racks, const int maxBorrow)
        : racks_(racks),
          maxBorrow_(maxBorrow)
    {
        racksToBooks_.resize(racks_);
        std::cout << "Created library with " << racks_ << "racks\n";
    }

    void addUser(const std::string& name, const std::string userID)
    {
        std::lock_guard<std::mutex> lock(mtx);

        if (users_.find(userID) != users_.end())
        {
            std::cout << "User with ID [" << userID << "] already present\n";
            return;
        }

        users_.insert({userID, std::make_shared<User>(name, userID)});
    }

    void addBook(const std::string& title, const std::string& bookID,
                 const std::unordered_set<std::string>& authors,
                 const std::unordered_set<std::string>& publishers,
                 std::vector<std::string>& bookCopies)
    {
        std::lock_guard<std::mutex> lock(mtx);

        HBook book{nullptr};
        auto bookIter = books_.find(bookID);
        int currentCopies{};
        if (bookIter != books_.end())
        {
            book = bookIter->second;
            currentCopies = book->getBookCopies().size();
        }

        int requiredRacks = bookCopies.size() - currentCopies;
        if (requiredRacks > racks_)
        {
            std::cout << "Rack not available\n";
            return;
        }

        if (!book)
        {
            book = std::make_shared<Book>(title, bookID, authors, publishers);
            books_.insert({bookID, book});
        }

        std::stringstream ssRacksUsed{};
        int count{};
        for (int i = 1; i <= racks_; i++)
        {
            auto& rack = racksToBooks_[i - 1];
            auto rackIter = rack.find(bookID);
            if (rackIter != rack.end())
                continue;

            std::string bookCopyID = bookCopies.back();
            bookCopies_.insert({bookCopyID, std::make_shared<BookCopy>(bookCopyID, book, i)});
            bookCopies.pop_back();
            book->addBookCopy(bookCopyID);
            rack.insert({bookID, bookCopyID});
            if (count > 0)
                ssRacksUsed << ",";
            ssRacksUsed << i;
            count++;
        }

        std::cout << "Added Book to racks: " << ssRacksUsed.str() << '\n';
    }

    void removeBookCopy(const std::string& bookCopyID)
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto bookCopyIter = bookCopies_.find(bookCopyID);
        if (bookCopyIter == bookCopies_.end() || bookCopyIter->second->getAssignedRack() == -1)
        {
            std::cout << "Invalid Book Copy ID\n";
            return;
        }

        auto& bookCopy = bookCopyIter->second;
        int rack = bookCopy->getAssignedRack();
        auto& book = bookCopy->getBook();

        book->removeBookCopy(bookCopyID);
        bookCopies_.erase(bookCopyIter);
        racksToBooks_[rack - 1].erase(book->getID());
        std::cout << "Removed book copy: " << bookCopyID << " from rack: " << rack << '\n';
    }

    void borrowBook(const std::string& bookID, const std::string& dueDate,
                    const std::string& userID)
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto userIter = users_.find(userID);
        if (userIter == users_.end())
        {
            std::cout << "Invalid User ID\n";
        }
        auto& user = userIter->second;

        int borrowedCount = user->getBorrowed().size();
        if (borrowedCount >= maxBorrow_)
        {
            std::cout << "Overlimit\n";
            return;
        }

        auto bookIter = books_.find(bookID);
        if (bookIter == books_.end())
        {
            std::cout << "Invalid Book ID\n";
            return;
        }

        for (int i = 1; i <= racks_; i++)
        {
            auto& rack = racksToBooks_[i - 1];
            auto rackIter = rack.find(bookID);
            if (rackIter == rack.end())
                continue;

            const auto& bookCopyID = rackIter->second;
            auto& bookCopy = bookCopies_[bookCopyID];
            bookCopy->registerBorrowed(userID);
            user->addBorrowedBookCopy(bookCopyID, dueDate);
            rack.erase(bookID);
            std::cout << "Borrowed Book from rack: " << i << '\n';
            return;
        }

        std::cout << "Not available\n";
    }

    void borrowBookCopy(const std::string& bookCopyID, const std::string& dueDate,
                        const std::string& userID)
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto userIter = users_.find(userID);
        if (userIter == users_.end())
        {
            std::cout << "Invalid User ID\n";
            return;
        }
        auto& user = userIter->second;

        int borrowedCount = user->getBorrowed().size();
        if (borrowedCount >= maxBorrow_)
        {
            std::cout << "Overlimit\n";
            return;
        }

        auto bookCopyIter = bookCopies_.find(bookCopyID);
        if (bookCopyIter == bookCopies_.end() || bookCopyIter->second->getAssignedRack() == -1)
        {
            std::cout << "Invalid Book Copy ID\n";
            return;
        }

        auto& bookCopy = bookCopyIter->second;
        const std::string& bookID = bookCopy->getBook()->getID();
        int rack = bookCopy->getAssignedRack();
        bookCopy->registerBorrowed(userID);
        user->addBorrowedBookCopy(bookCopyID, dueDate);
        racksToBooks_[rack - 1].erase(bookID);
        std::cout << "Borrowed Book Copy from rack: " << rack << '\n';
    }

    void returnBookCopy(const std::string& bookCopyID)
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto bookCopyIter = bookCopies_.find(bookCopyID);
        if (bookCopyIter == bookCopies_.end())
        {
            std::cout << "Invalid Book Copy ID\n";
            return;
        }

        auto& bookCopy = bookCopyIter->second;
        const std::string& borrowerID = bookCopy->getBorrowerID();
        const std::string& bookID = bookCopy->getBook()->getID();

        for (int i = 1; i <= racks_; i++)
        {
            auto& rack = racksToBooks_[i - 1];
            auto rackIter = rack.find(bookID);
            if (rackIter != rack.end())
                continue;

            auto& user = users_[borrowerID];
            user->removeBorrowedBookCopy(bookCopyID);
            rack.insert({bookID, bookCopyID});
            bookCopy->registerReturn(i);
            std::cout << "Returned book copy ID: " << bookCopyID << " and returned to rack: " << i
                      << '\n';
            return;
        }

        std::cout << "No space on racks to return book\n";
    }

    void printBorrowed(const std::string& userID)
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto userIter = users_.find(userID);
        if (userIter == users_.end())
        {
            std::cout << "Invalid User ID\n";
        }
        auto& user = userIter->second;

        const auto& borrowed = user->getBorrowed();
        for (const auto& entry : borrowed)
        {
            std::cout << "Book Copy: " << entry.first << " " << entry.second << '\n';
        }
    }

    void search(const std::string& attribute, const std::string& value)
    {
        if (attribute == "book_id")
            bookSearchStrategy_ = std::make_unique<BookSearchByID>(value);
        else if (attribute == "author_id")
            bookSearchStrategy_ = std::make_unique<BookSearchByAuthor>(value);
        else if (attribute == "publisher_id")
            bookSearchStrategy_ = std::make_unique<BookSearchByPublisher>(value);
        else
        {
            std::cout << "Invalid search attribute\n";
            return;
        }

        std::vector<HBook> matches = bookSearchStrategy_->matches(books_);

        std::vector<std::pair<int, std::string>> racksBookCopies{};

        bookSearchStrategy_ = nullptr;
    }

private:
    std::unordered_map<std::string, HBook> books_{};
    std::unordered_map<std::string, HBookCopy> bookCopies_{};
    std::unordered_map<std::string, HUser> users_{};
    // rack -> {bookID : bookCopyID}
    std::vector<std::unordered_map<std::string, std::string>> racksToBooks_{};

    HBookSearchStrategy bookSearchStrategy_{nullptr};
    std::mutex mtx{};
    int racks_{};
    int maxBorrow_{5};
};

int main()
{
    return 0;
}