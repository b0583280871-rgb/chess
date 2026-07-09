#pragma once

#include <string>
#include <vector>

struct Board {
    std::vector<std::vector<std::string>> grid;
    int rows() const { return (int)grid.size(); }
    int cols() const { return grid.empty() ? 0 : (int)grid[0].size(); }
};

bool isEmpty(const std::string& tok);
char colorOf(const std::string& tok);
char pieceOf(const std::string& tok);
