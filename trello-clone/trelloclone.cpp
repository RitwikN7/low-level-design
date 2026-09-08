#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

/*
    elements:
    - user
    - board
    - list
    - card

    functionality:
    - show board/list/card details
    - create / delete board
    - create / delete list
    - create / delete card
    - assign user to card
    - move card from one list to another within same board
    - generate unique id for each element
    - url based on unique id for each element
*/

const std::string BASE_URL = "https://trelloclone.com/";

template <typename T>
class IDGenerator
{
public:
    static std::string generate(const std::string& prefix)
    {
        return prefix + "_" + std::to_string(++counter);
    }

private:
    inline static std::atomic<uint64_t> counter{0};
};

class User
{
public:
    User(std::string name, std::string email)
        : name_(std::move(name)),
          email_(std::move(email)),
          userID_(IDGenerator<User>::generate("USER"))
    {
        std::cout << "Created user: " << userID_ << '\n';
    }

    const std::string& getID()
    {
        return userID_;
    }

    friend std::ostream& operator<<(std::ostream& os, const User& user);

private:
    std::string name_;
    std::string email_;
    std::string userID_;
};

using HUser = std::shared_ptr<User>;

class Card
{
public:
    Card(std::string name, std::string parentListID)
        : name_(std::move(name)),
          cardID_(IDGenerator<Card>::generate("CARD")),
          parentListID_(std::move(parentListID))
    {
        std::cout << "Created card: " << cardID_ << '\n';
    }

    const std::string& getName()
    {
        return name_;
    }

    void setName(const std::string& name)
    {
        name_ = name;
    }

    const std::string& getDescription()
    {
        return description_;
    }

    void setDescription(const std::string& description)
    {
        description_ = description;
    }

    const std::string& getID()
    {
        return cardID_;
    }

    HUser& getAssignedUser()
    {
        return assignedUser_;
    }

    void assignUser(HUser& user)
    {
        assignedUser_ = user;
    }

    void unassignUser()
    {
        assignedUser_ = nullptr;
    }

    const std::string& getParentListID()
    {
        return parentListID_;
    }

    void setParentListID(const std::string& listID)
    {
        parentListID_ = listID;
    }

    friend std::ostream& operator<<(std::ostream& os, const Card& card);

private:
    std::string name_;
    std::string description_;
    std::string cardID_;
    std::string parentListID_;
    HUser assignedUser_{nullptr};
};

using HCard = std::shared_ptr<Card>;

class List
{
public:
    List(std::string name, std::string parentBoardID)
        : name_(std::move(name)),
          listID_(IDGenerator<List>::generate("LIST")),
          parentBoardID_(std::move(parentBoardID))
    {
        std::cout << "Created list: " << listID_ << '\n';
    }

    const std::string& getName()
    {
        return name_;
    }

    void setName(const std::string& name)
    {
        name_ = name;
    }

    const std::string& getID()
    {
        return listID_;
    }

    void addCard(HCard& card)
    {
        cards_.insert(card);
    }

    void removeCard(HCard& card)
    {
        cards_.erase(card);
    }

    std::unordered_set<HCard> getCards() const
    {
        return cards_;
    }

    const std::string& getParentBoardID()
    {
        return parentBoardID_;
    }

    friend std::ostream& operator<<(std::ostream& os, const List& list);

private:
    std::string name_;
    std::string listID_;
    std::string parentBoardID_;
    std::unordered_set<HCard> cards_;
};

using HList = std::shared_ptr<List>;

class Board
{
public:
    Board(std::string name)
        : name_(std::move(name)),
          boardID_(IDGenerator<Board>::generate("BOARD")),
          boardURL_(BASE_URL + boardID_)
    {
        std::cout << "Created board: " << boardID_ << '\n';
    }

    const std::string& getName()
    {
        return name_;
    }

    void setName(const std::string& name)
    {
        name_ = name;
    }

    const std::string& getID()
    {
        return boardID_;
    }

    const std::string& getPrivacy()
    {
        return privacy_;
    }

    void setPrivacy(const std::string& privacy)
    {
        privacy_ = privacy;
    }

    void addList(HList& list)
    {
        lists_.insert(list);
    }

    void removeList(HList& list)
    {
        lists_.erase(list);
    }

    std::unordered_set<HList> getLists() const
    {
        return lists_;
    }

    void addMember(HUser& member)
    {
        members_.insert(member);
    }

    void removeMember(HUser& member)
    {
        members_.erase(member);
    }

    bool isMember(HUser& user)
    {
        return members_.contains(user);
    }

    std::unordered_set<HUser> getMembers() const
    {
        return members_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Board& board);

private:
    std::string name_;
    std::string boardID_;
    std::string boardURL_;
    std::string privacy_{"PUBLIC"};
    std::unordered_set<HUser> members_;
    std::unordered_set<HList> lists_;
};

using HBoard = std::shared_ptr<Board>;

std::ostream& operator<<(std::ostream& os, const User& user)
{
    os << "{ name: " << user.name_ << ","
       << " email: " << user.email_ << ","
       << " userID: " << user.userID_ << "}";

    return os;
}

std::ostream& operator<<(std::ostream& os, const Card& card)
{
    std::string userID{"null"};
    if (card.assignedUser_)
        userID = card.assignedUser_->getID();

    os << "{ name : " << card.name_ << ","
       << " description: " << card.description_ << ","
       << " cardID: " << card.cardID_ << ","
       << " parentListID: " << card.parentListID_ << ","
       << " assignedUserID: " << userID << "}";

    return os;
}

std::ostream& operator<<(std::ostream& os, const List& list)
{
    os << "{ name: " << list.name_ << ","
       << " listID: " << list.listID_ << ","
       << " parentBoardID: " << list.parentBoardID_ << ","
       << " cards: [";

    int count{};
    for (const auto& card : list.getCards())
    {
        if (count > 0)
            os << ",\n";

        os << *card;
        count++;
    }

    os << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Board& board)
{
    os << "{ name: " << board.name_ << ","
       << " boardID: " << board.boardID_ << ","
       << " boardURL: " << board.boardURL_ << ","
       << " privacy: " << board.privacy_ << ","
       << " members: [";

    int count{};
    for (const auto& member : board.getMembers())
    {
        if (count > 0)
            os << ",\n";

        os << *member;
        count++;
    }

    os << "],\nlists: [";
    count = 0;
    for (const auto& list : board.getLists())
    {
        if (count > 0)
            os << ",\n";

        os << *list;
        count++;
    }

    os << "]}";
    return os;
}

class TrelloClone
{
public:
    void createUser(const std::string& name, const std::string& email)
    {
        HUser user = std::make_shared<User>(name, email);
        users_.insert({user->getID(), user});
    }

    void createBoard(const std::string& name)
    {
        HBoard board = std::make_shared<Board>(name);
        boards_.insert({board->getID(), board});
    }

    void setBoardName(const std::string& boardID, const std::string& name)
    {
        auto boardIter = boards_.find(boardID);
        if (boardIter == boards_.end())
        {
            std::cout << "No board found with ID: " << boardID << '\n';
            return;
        }

        HBoard& board = boardIter->second;
        if (name == board->getName())
        {
            std::cout << "Board name is already set to: " << name << '\n';
            return;
        }

        board->setName(name);
        std::cout << "Set board name to: " << name << '\n';
    }

    void addMemberToBoard(const std::string& boardID, const std::string& memberID)
    {
        auto boardIter = boards_.find(boardID);
        if (boardIter == boards_.end())
        {
            std::cout << "No board found with ID: " << boardID << '\n';
            return;
        }

        auto userIter = users_.find(memberID);
        if (userIter == users_.end())
        {
            std::cout << "No user found with ID: " << memberID << '\n';
            return;
        }

        HBoard& board = boardIter->second;
        HUser& user = userIter->second;
        if (board->isMember(user))
        {
            std::cout << "User is already a member of board with ID: " << boardID << '\n';
            return;
        }

        board->addMember(userIter->second);
        std::cout << "Added member to board with ID: " << boardID << '\n';
    }

    void removeMemberFromBoard(const std::string& boardID, const std::string& memberID)
    {
        auto boardIter = boards_.find(boardID);
        if (boardIter == boards_.end())
        {
            std::cout << "No board found with ID: " << boardID << '\n';
            return;
        }

        auto userIter = users_.find(memberID);
        if (userIter == users_.end())
        {
            std::cout << "No user found with ID: " << memberID << '\n';
            return;
        }

        HBoard& board = boardIter->second;
        HUser& user = userIter->second;
        if (!board->isMember(user))
        {
            std::cout << "User is not a member of board with ID: " << boardID << '\n';
            return;
        }

        board->removeMember(user);
        std::cout << "Removed user from board with ID: " << boardID << '\n';
    }

    void setBoardPrivacy(const std::string& boardID, const std::string& privacy)
    {
        if (privacy != "PUBLIC" && privacy != "PRIVATE")
        {
            std::cout << "Invalid value for privacy: " << privacy << '\n';
            return;
        }

        auto boardIter = boards_.find(boardID);
        if (boardIter == boards_.end())
        {
            std::cout << "No board found with ID: " << boardID << '\n';
            return;
        }

        HBoard& board = boardIter->second;
        if (privacy == board->getPrivacy())
        {
            std::cout << "Board is already set to privacy: " << privacy << '\n';
            return;
        }

        board->setPrivacy(privacy);
        std::cout << "Set privacy of board to: " << privacy << '\n';
    }

    void deleteBoard(const std::string& boardID)
    {
        auto boardIter = boards_.find(boardID);
        if (boardIter == boards_.end())
        {
            std::cout << "No board found with ID: " << boardID << '\n';
            return;
        }

        for (const auto& list : boardIter->second->getLists())
        {
            deleteList(list->getID());
        }

        boards_.erase(boardID);
        std::cout << "Deleted board with ID: " << boardID << '\n';
    }

    void createList(const std::string& name, const std::string& parentBoardID)
    {
        auto boardIter = boards_.find(parentBoardID);
        if (boardIter == boards_.end())
        {
            std::cout << "No board found with ID: " << parentBoardID << '\n';
            return;
        }

        HList list = std::make_shared<List>(name, parentBoardID);
        lists_.insert({list->getID(), list});
        boardIter->second->addList(list);
    }

    void setListName(const std::string& listID, const std::string& name)
    {
        auto listIter = lists_.find(listID);
        if (listIter == lists_.end())
        {
            std::cout << "No list found with ID: " << listID << '\n';
            return;
        }

        HList& list = listIter->second;
        if (name == list->getName())
        {
            std::cout << "List name already set to: " << name << '\n';
            return;
        }

        list->setName(name);
        std::cout << "Set list name to: " << name << '\n';
    }

    void deleteList(const std::string& listID)
    {
        auto listIter = lists_.find(listID);
        if (listIter == lists_.end())
        {
            std::cout << "No list found with ID: " << listID << '\n';
            return;
        }

        const std::string& parentBoardID = listIter->second->getParentBoardID();
        HBoard& board = boards_.at(parentBoardID);

        for (const auto& card : listIter->second->getCards())
        {
            deleteCard(card->getID());
        }

        board->removeList(listIter->second);
        lists_.erase(listID);
        std::cout << "Deleted list with ID: " << listID << '\n';
    }

    void createCard(const std::string& name, const std::string& parentListID)
    {
        auto listIter = lists_.find(parentListID);
        if (listIter == lists_.end())
        {
            std::cout << "No list found with ID: " << parentListID << '\n';
            return;
        }

        HCard card = std::make_shared<Card>(name, parentListID);
        cards_.insert({card->getID(), card});
        listIter->second->addCard(card);
    }

    void setCardName(const std::string& cardID, const std::string& name)
    {
        auto cardIter = cards_.find(cardID);
        if (cardIter == cards_.end())
        {
            std::cout << "No card with ID: " << cardID << '\n';
            return;
        }

        HCard& card = cardIter->second;
        if (name == card->getName())
        {
            std::cout << "Card name already set to: " << name << '\n';
            return;
        }

        card->setName(name);
        std::cout << "Set card name to: " << name << '\n';
    }

    void setCardDescription(const std::string& cardID, const std::string& description)
    {
        auto cardIter = cards_.find(cardID);
        if (cardIter == cards_.end())
        {
            std::cout << "No card with ID: " << cardID << '\n';
            return;
        }

        HCard& card = cardIter->second;
        if (description == card->getDescription())
        {
            std::cout << "Card description already set to: " << description << '\n';
            return;
        }

        card->setDescription(description);
        std::cout << "Set card description to: " << description << '\n';
    }

    void assignCardToUser(const std::string& cardID, const std::string& userID)
    {
        auto cardIter = cards_.find(cardID);
        if (cardIter == cards_.end())
        {
            std::cout << "No card with ID: " << cardID << '\n';
            return;
        }

        auto userIter = users_.find(userID);
        if (userIter == users_.end())
        {
            std::cout << "No user with ID: " << userID << '\n';
            return;
        }

        HCard& card = cardIter->second;
        HUser& assignedUser = card->getAssignedUser();
        if (assignedUser && assignedUser->getID() == userID)
        {
            std::cout << "Card already assigned to user with ID: " << userID << '\n';
            return;
        }

        card->assignUser(userIter->second);
        std::cout << "Assigned card to user with ID: " << userID << '\n';
    }

    void unassignUser(const std::string& cardID)
    {
        auto cardIter = cards_.find(cardID);
        if (cardIter == cards_.end())
        {
            std::cout << "No card with ID: " << cardID << '\n';
            return;
        }

        HCard& card = cardIter->second;
        HUser& assignedUser = card->getAssignedUser();
        if (!assignedUser)
        {
            std::cout << "Card is already unassigned\n";
            return;
        }

        const std::string& userID = assignedUser->getID();
        card->unassignUser();
        std::cout << "Unassigned card from user with ID: " << userID << '\n';
    }

    void deleteCard(const std::string& cardID)
    {
        auto cardIter = cards_.find(cardID);
        if (cardIter == cards_.end())
        {
            std::cout << "No card with ID: " << cardID << '\n';
            return;
        }

        const std::string& parentListID = cardIter->second->getParentListID();
        HList& parentList = lists_.at(parentListID);

        parentList->removeCard(cardIter->second);
        cards_.erase(cardIter);
        std::cout << "Deleted card with ID: " << cardID << '\n';
    }

    void moveCard(const std::string& cardID, const std::string& destinationListID)
    {
        auto cardIter = cards_.find(cardID);
        if (cardIter == cards_.end())
        {
            std::cout << "No card with ID: " << cardID << '\n';
            return;
        }

        auto listIter = lists_.find(destinationListID);
        if (listIter == lists_.end())
        {
            std::cout << "No list with ID: " << destinationListID << '\n';
            return;
        }

        HCard& card = cardIter->second;
        const std::string& parentListID = card->getParentListID();
        if (parentListID == destinationListID)
        {
            std::cout << "Card already present in destination list with ID: " << destinationListID
                      << '\n';
            return;
        }

        HList& parentList = lists_.at(parentListID);
        HList& destinationList = listIter->second;
        if (parentList->getParentBoardID() != listIter->second->getParentBoardID())
        {
            std::cout << "Lists do not belong to same board. Cannot move card" << '\n';
            return;
        }

        card->setParentListID(destinationListID);
        parentList->removeCard(card);
        destinationList->addCard(card);
        std::cout << "Moved card to list with ID: " << destinationListID << '\n';
    }

    void displayAllBoards()
    {
        if (boards_.size() == 0)
        {
            std::cout << "No boards to display\n";
            return;
        }

        std::stringstream ss{};
        ss << "{ boards: [";
        int count{};
        for (auto& board : boards_)
        {
            if (count > 0)
                ss << ",\n";

            ss << *(board.second);
            count++;
        }
        ss << "]}\n";
        std::cout << ss.str();
    }

    void displayBoard(const std::string& boardID)
    {
        auto boardIter = boards_.find(boardID);
        if (boardIter == boards_.end())
        {
            std::cout << "No board found with ID: " << boardID << '\n';
            return;
        }

        std::cout << *(boardIter->second);
    }

    void displayList(const std::string& listID)
    {
        auto listIter = lists_.find(listID);
        if (listIter == lists_.end())
        {
            std::cout << "No list found with ID: " << listID << '\n';
            return;
        }

        std::cout << *(listIter->second);
    }

    void displayCard(const std::string& cardID)
    {
        auto cardIter = cards_.find(cardID);
        if (cardIter == cards_.end())
        {
            std::cout << "No card with ID: " << cardID << '\n';
            return;
        }

        std::cout << *(cardIter->second);
    }

private:
    std::unordered_map<std::string, HUser> users_;
    std::unordered_map<std::string, HList> lists_;
    std::unordered_map<std::string, HBoard> boards_;
    std::unordered_map<std::string, HCard> cards_;
};

using HTrelloClone = std::unique_ptr<TrelloClone>;

int main()
{
    std::cout << "Welcome to TrelloClone!" << '\n';

    HTrelloClone trello = std::make_unique<TrelloClone>();

    // add users
    trello->createUser("Ritwik", "rit@gmail");
    trello->createUser("Arjun", "aj@gmail");

    while (true)
    {
        std::cout << '\n';
        std::string cmd{};
        std::getline(std::cin, cmd);

        std::stringstream ss(cmd);

        std::string primaryCmd{};
        if (!(ss >> primaryCmd))
        {
            std::cout << "No command entered!\n";
            continue;
        }

        if (primaryCmd == "EXIT")
        {
            std::cout << "Goodbye!\n";
            return 0;
        }
        else if (primaryCmd == "SHOW")
        {
            std::string secondCmd{};
            if (!(ss >> secondCmd))
            {
                trello->displayAllBoards();
                continue;
            }

            std::string id{};
            if (!(ss >> id))
            {
                std::cout << "No ID provided\n";
                continue;
            }

            if (secondCmd == "BOARD")
                trello->displayBoard(id);
            else if (secondCmd == "LIST")
                trello->displayList(id);
            else if (secondCmd == "CARD")
                trello->displayCard(id);
            else
                std::cout << "Unknown type to display\n";
        }
        else if (primaryCmd == "BOARD")
        {
            std::string actionOrID{};
            if (!(ss >> actionOrID))
            {
                std::cout << "Incomplete command";
                continue;
            }

            if (actionOrID == "CREATE")
            {
                std::string name{};
                if (!std::getline(ss >> std::ws, name))
                {
                    std::cout << "No name provided";
                    continue;
                }

                trello->createBoard(name);
            }
            else if (actionOrID == "DELETE")
            {
                std::string id{};
                if (!std::getline(ss >> std::ws, id))
                {
                    std::cout << "No ID provided";
                    continue;
                }

                trello->deleteBoard(id);
            }
            else // treat second word as an ID
            {
                std::string member{};
                if (!(ss >> member))
                {
                    std::cout << "No data point provided to edit";
                    continue;
                }

                if (member != "name" && member != "privacy" && member != "ADD_MEMBER" &&
                    member != "REMOVE_MEMBER")
                {
                    std::cout << "Invalid data point provided: " << member;
                    continue;
                }

                std::string data{};
                if (!std::getline(ss >> std::ws, data))
                {
                    std::cout << "No data provided";
                    continue;
                }

                if (member == "name")
                    trello->setBoardName(actionOrID, data);
                else if (member == "privacy")
                    trello->setBoardPrivacy(actionOrID, data);
                else if (member == "ADD_MEMBER")
                    trello->addMemberToBoard(actionOrID, data);
                else
                    trello->removeMemberFromBoard(actionOrID, data);
            }
        }
        else if (primaryCmd == "LIST")
        {
            std::string actionOrID{};
            if (!(ss >> actionOrID))
            {
                std::cout << "Incomplete command";
                continue;
            }

            if (actionOrID == "CREATE")
            {
                std::string boardID{};
                if (!(ss >> boardID))
                {
                    std::cout << "Incomplete command";
                    continue;
                }

                std::string name{};
                if (!std::getline(ss >> std::ws, name))
                {
                    std::cout << "No name provided";
                    continue;
                }

                trello->createList(name, boardID);
            }
            else if (actionOrID == "DELETE")
            {
                std::string id{};
                if (!std::getline(ss >> std::ws, id))
                {
                    std::cout << "No ID provided";
                    continue;
                }

                trello->deleteList(id);
            }
            else // treat second word as an ID
            {
                std::string member{};
                if (!(ss >> member))
                {
                    std::cout << "No data point provided to edit";
                    continue;
                }

                if (member != "name")
                {
                    std::cout << "Invalid data point provided: " << member;
                    continue;
                }

                std::string data{};
                if (!std::getline(ss >> std::ws, data))
                {
                    std::cout << "No data provided";
                    continue;
                }

                trello->setListName(actionOrID, data);
            }
        }
        else if (primaryCmd == "CARD")
        {
            std::string actionOrID{};
            if (!(ss >> actionOrID))
            {
                std::cout << "Incomplete command";
                continue;
            }

            if (actionOrID == "CREATE")
            {
                std::string listID{};
                if (!(ss >> listID))
                {
                    std::cout << "Incomplete command";
                    continue;
                }

                std::string name{};
                if (!std::getline(ss >> std::ws, name))
                {
                    std::cout << "No name provided";
                    continue;
                }

                trello->createCard(name, listID);
            }
            else if (actionOrID == "DELETE")
            {
                std::string id{};
                if (!std::getline(ss >> std::ws, id))
                {
                    std::cout << "No ID provided";
                    continue;
                }

                trello->deleteCard(id);
            }
            else // treat second word as an ID
            {
                std::string member{};
                if (!(ss >> member))
                {
                    std::cout << "No data point provided to edit";
                    continue;
                }

                if (member == "UNASSIGN")
                {
                    trello->unassignUser(actionOrID);
                    continue;
                }

                if (member != "name" && member != "description" && member != "ASSIGN" &&
                    member != "MOVE")
                {
                    std::cout << "Invalid data point provided: " << member;
                    continue;
                }

                std::string data{};
                if (!std::getline(ss >> std::ws, data))
                {
                    std::cout << "No data provided";
                    continue;
                }

                if (member == "name")
                    trello->setCardName(actionOrID, data);
                else if (member == "description")
                    trello->setCardDescription(actionOrID, data);
                else if (member == "ASSIGN")
                    trello->assignCardToUser(actionOrID, data);
                else
                    trello->moveCard(actionOrID, data);
            }
        }
        else
            std::cout << "Unknown command: " << primaryCmd;
    }
}
