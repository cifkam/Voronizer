#ifndef GROWING_HPP
#define GROWING_HPP

#include <set>
#include <vector>
#include <opencv2/core.hpp>

using mat_t = int16_t;

enum Neighborhood {n4, n8, swap};
enum State {unseen, opened, closed};

struct Cell
{
    Cell(size_t row, size_t col, State state);
    friend bool operator<(const Cell& lhs, const Cell& rhs);

    size_t row;
    size_t col;
    State state;
};


class Growing
{
public:
    
    Neighborhood neighborhood;
    Growing(Neighborhood neighborhood);
    virtual size_t compute(cv::Mat& data);

protected:
    virtual void init_funct(std::set<Cell*>& opened, std::vector<std::vector<Cell>>& cell_mat, cv::Mat& data) = 0;
    virtual void post_funct(std::vector<Cell*>& processed, cv::Mat& data){}
    std::vector<Cell*> get_neighbors(const Cell* cell, std::vector<std::vector<Cell>>& cell_mat, bool bool_4_8, std::size_t cols, std::size_t rows);
};


#endif /* GROWING_HPP */