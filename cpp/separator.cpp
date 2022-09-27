#include "separator.hpp"
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include "utils.hpp"
using namespace std;

Separator::Separator(size_t treshold, int bg_value)
: Growing(Neighborhood::n4), treshold(treshold), bg_value(bg_value)
{

}

void Separator::add_to_group(Cell* cell, int cls)
{
    auto it = groups.find(cls);
    if (it == groups.end())
        groups.emplace_hint(it, cls, std::vector<Cell*>(1, cell)); // not found
    else
        it->second.push_back(cell); //found
}

void Separator::init_funct(set<Cell*>& opened, CellMat& cell_mat, cv::Mat& output)
{
    cell_mat = this->cell_mat; // restore cell_mat from previous iteration
    Cell* cell = nullptr;


    //restore searching from pixel used in previous iteration instead always starting from zero
    for (int col = last_col+1; col < output.cols; ++col)
    {
        if (output.at<int_t>(last_row,col) < 0)
        {
            cell = &cell_mat[last_row][col];
            last_col = col;
            break;
        }
    }
    if (cell == nullptr)
        for (int row = last_row+1; row < output.rows; ++row)
        {
            for (int col = 0; col < output.cols; ++col)
            {
                if (output.at<int_t>(row,col) < 0)
                {
                    cell = &cell_mat[row][col];
                    last_row = row;
                    last_col = col;
                    break;
                }
            }

            if (cell != nullptr) break; 
        }
    if (cell == nullptr) return;

    cell->state = State::opened;
    assign(output, n, cell);
    opened.insert(cell);
}

void Separator::post_funct(std::vector<Cell*>& processed, cv::Mat& output)
{
    if (processed.size() < treshold)
    {
        for (auto cell : processed)
        {
            assign(output, 0, cell);
            add_to_group(cell, 0);
        }
    }
    else
    {
        for (auto cell : processed)
            add_to_group(cell, n);
    }
}


bool Separator::grow_condition(const cv::Mat& input_data, const cv::Mat& data, Cell* cell, Cell* neighbor)
{
    return (neighbor->state == State::unseen) && (this->input_data->at<int_t>(cell->row,cell->col) == this->input_data->at<int_t>(neighbor->row, neighbor->col));   
}



size_t Separator::compute(cv::Mat& input_data, cv::Mat& output_data, bool clear_groups)
{
    if (clear_groups)
        this->clear_groups();

    cv::Mat data = input_data.clone();
    this->input_data = &input_data;
    this->last_row = 0;
    this->last_col = 0;

    if (bg_value != 0)
        for (size_t row = 0; row < data.rows; ++row)
            for (size_t col = 0; col < data.cols; ++col)
            {
                int_t value = data.at<int_t>(row,col);

                if (value > bg_value)
                    data.at<int_t>(row,col) = -value;
                else
                    data.at<int_t>(row,col) = value - bg_value;
            }
    else
        data = -data;

    // First initialization of cell_mat
    cell_mat = CellMat(
        data.rows, vector<Cell>(
        data.cols, Cell(0, 0, State::unseen)));
    for (size_t row = 0; row < data.rows; ++row)
        for (size_t col = 0; col < data.cols; ++col)
        {
            cell_mat[row][col].row = row;
            cell_mat[row][col].col = col;
            cell_mat[row][col].state = (data.at<int_t>(row,col) < 0 ?
                State::unseen : State::closed);
        }

    this->n = 1;
    size_t steps = 0;
    while (true)
    {
        size_t s = Growing::compute_inner(data, false);
        ++n;
        steps += s;
        if (s == 0)
            break;
    }

    // Grow pixels after removing areas with #pixels < trehold
    auto tg = AfterTresholdGrowing(input_data.rows, input_data.cols, move(groups), move(cell_mat));
    tg.compute(data, data, false);
    groups = tg.clear_groups();
    cell_mat = tg.clear_cellmat();
    

    this->input_data = nullptr;
    output_data = move(data);
    return steps;
}



Separator::AfterTresholdGrowing::AfterTresholdGrowing(int rows, int cols, Groups&& groups, CellMat&& cell_mat) : Growing(Neighborhood::n4)
{
    this->rows = rows;
    this->cols = cols;
    this->groups = groups;
    this->cell_mat = cell_mat;
}


void Separator::AfterTresholdGrowing::init_funct(set<Cell*>& opened, CellMat& cell_mat, cv::Mat& output)
{
    bool bool_4_8 = (neighborhood == Neighborhood::n4);

    // Find the border of "background" pixels
    auto it = groups.find(0);
    if (it != groups.end())
    {
        auto bg = unordered_set<Cell*>();
        bg.reserve(it->second.size());
        for (auto& x : it->second)
            bg.insert(x);
        
        for (auto& cell : it->second)
        {
            for (auto& neighbor : get_neighbors(cell, cell_mat, bool_4_8, cols, rows))
            {
                auto c_it = bg.find(neighbor);
                if (c_it != bg.end())
                {
                    (**c_it).state = State::opened;
                    opened.insert(cell);
                }

            }
        }
    }
    
}
