//#define NDEBUG // uncomment to disable assert()

#include <iostream>
#include <cassert>
#include "growing.hpp"

using namespace std;

Cell::Cell(size_t row, size_t col, State state)
: row(row), col(col), state(state)
{}

bool operator<(const Cell& lhs, const Cell& rhs)
{
    return lhs.row < rhs.row || (lhs.row == rhs.row && lhs.col < rhs.col);
}


Growing::Growing(Neighborhood neighborhood)
 : neighborhood(neighborhood)
{}

void Growing::post_funct(std::vector<Cell*>& processed, cv::Mat& data)
{
    for (auto& cell : processed)
    {
        int cls = data.at<int_t>(cell->row, cell->col);
        add_to_group(cell, cls);
    }
}

void Growing::add_to_group(Cell* cell, int cls)
{
    auto it = groups.find(cls);
    if (it == groups.end())
        groups.emplace_hint(it, cls, std::vector<Cell*>(1, cell)); // not found
    else
        it->second.push_back(cell); //found
}
    

void Growing::assign(cv::Mat& data, Cell* from, Cell* to)
{
    data.at<int_t>(to->row, to->col) = data.at<int_t>(from->row, from->col);
}

void Growing::assign(cv::Mat& data, int value, Cell* to)
{
    data.at<int_t>(to->row, to->col) = value;
}


Groups Growing::clear_groups()
{
    auto g = move(groups);
    groups = Groups();
    return g;
}

CellMat Growing::clear_cellmat()
{
    auto c = move(cell_mat);
    cell_mat = CellMat();
    return c;
}


bool Growing::grow_condition(const cv::Mat& input_data, const cv::Mat& data, Cell* cell, Cell* neighbor)
{
    return neighbor->state == State::unseen;   
}

size_t Growing::compute(cv::Mat& input_data, cv::Mat& output_data, bool clear_groups)
{
    cv::Mat data = input_data.clone();
    size_t steps = compute_inner(data, clear_groups);
    output_data = move(data);
    return steps;
}

size_t Growing::compute_inner(cv::Mat& data, bool clear_groups)
{
    assert(data.depth() == CV_16S);
    if (clear_groups)
        this->clear_groups();

    bool bool_4_8 = (neighborhood == Neighborhood::n4);
    set<Cell*> opened;
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
                if (grow_condition(data, data, cell, neighbor))
                {
                    new_cells.insert(neighbor);
                    assign(data, cell, neighbor);
                    neighbor->state = State::opened;
                }
            
            cell->state = State::closed;
            processed.push_back(cell);
        }

        if (neighborhood == Neighborhood::alternating)
            bool_4_8 = !bool_4_8;
        
        opened = move(new_cells);
        steps += 1;
    }
    post_funct(processed, data);
    return steps;
};


vector<Cell*> Growing::get_neighbors(
    const Cell* cell,
    CellMat& cell_mat,
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



