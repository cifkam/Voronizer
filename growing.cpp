//#define NDEBUG // uncomment to disable assert()

#include <iostream>
#include <cassert>
#include "growing.hpp"

using namespace std;

Cell::Cell(size_t row, size_t col, State state)
: row(row), col(col), state(state)
{

}

bool operator<(const Cell& lhs, const Cell& rhs)
{
    return lhs.row < rhs.row || (lhs.row == rhs.row && lhs.col < rhs.col);
}


Growing::Growing(Neighborhood neighborhood)
 : neighborhood(neighborhood)
 {

 }

size_t Growing::compute(cv::Mat& data)
{
    assert(data.depth() == CV_16S);


    bool bool_4_8 = (neighborhood == Neighborhood::n4);
    set<Cell*> opened;
    vector<vector<Cell>> cell_mat;
    init_funct(opened, cell_mat, data);
    
    size_t steps = 0;

    vector<Cell*> processed;
    //processed.reserve(rows*cols);

    while (opened.size() > 0)
    {
        set<Cell*> new_cells;
        for (auto cell : opened)
        {
            for (auto&& neighbor : get_neighbors(cell, cell_mat, bool_4_8, data.cols, data.rows))
                if (neighbor->state == State::unseen)
                {
                    new_cells.insert(neighbor);
                    data.at<mat_t>(neighbor->row, neighbor->col) = data.at<mat_t>(cell->row, cell->col);
                    neighbor->state = State::opened;
                }
            
            cell->state = State::closed;
            processed.push_back(cell);
        }

        if (neighborhood == Neighborhood::swap)
            bool_4_8 = !bool_4_8;
        
        opened = move(new_cells);
        steps += 1;
    }
    post_funct(processed, data);
    return steps;
};


vector<Cell*> Growing::get_neighbors(
    const Cell* cell, vector<vector<Cell>>& cell_mat,
    bool bool_4_8,
    size_t cols,
    size_t rows
){
    vector<int> row_delta;
    vector<int> col_delta;
    vector<Cell*> neighbors;

    if (cell->row == 0)
        row_delta = {0,1};
    else if (cell->row == rows-1)
        row_delta = {-1,0};
    else
        row_delta = {-1,0,1};

    if (cell->col == 0)
        col_delta = {0,1};
    else if (cell->col == cols-1)
        col_delta = {-1,0};
    else
        col_delta = {-1,0,1};

    if (bool_4_8)
    {
        for (auto r : row_delta)
        {
            if (r == 0) continue;
            Cell* c = &cell_mat[cell->row+r][cell->col]; 
            neighbors.push_back(c);
        }
        for (auto c : col_delta)
        {
            if (c == 0) continue;
            neighbors.push_back(&cell_mat[cell->row][cell->col+c]);
        }
    }
    else
    {
        for (auto r : row_delta)
            for (auto c : col_delta)
            {
                if (r == 0 && c == 0) continue;
                neighbors.push_back(&cell_mat[cell->row+r][cell->col+c]);
            }
    }

    return neighbors;
}



