#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>

/*
Requirements:
    1. Game like connect 4, but elements are inserted from bottom, shifting tokens up
    2. Support upto 4 players (red, blue, green, yellow)

Entities:
    1. Column
    2. Board
    3. Game
*/

enum class Token : uint8_t
{
    Empty,
    Red,
    Blue,
    Green,
    Yellow
};

class Column
{
public:
    explicit Column(size_t size)
        : rows_(size, Token::Empty),
          free_rows(size)
    {
    }

    bool try_insert(Token token)
    {
        if (free_rows == 0)
            return false;

        auto index = rows_.size() - free_rows;
        rows_[index] = token;
        return true;
    }

    const Token& operator[](size_t index) const
    {
        auto adj_index = rows_.size() - (free_rows + index + 1);
        if (adj_index >= rows_.size())
            throw std::logic_error("Index out of bounds");

        return rows_[adj_index];
    }

private:
    std::vector<Token> rows_;
    size_t free_rows{};
};

enum class BoardState : uint8_t
{
    NotPlaying,
    Playing,
    Draw
};

class Board
{
private:
    std::vector<Column> columns_;
    int win_tokens{4};
};
