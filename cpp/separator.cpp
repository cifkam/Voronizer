#include "separator.hpp"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <stdexcept>

#include <opencv2/imgproc.hpp>

#include "utils.hpp"
using namespace std;

Separator::Separator(size_t treshold, int bg_value)
: Growing(Neighborhood::n4), treshold(treshold), bg_value(bg_value)
{

}

void Separator::add_to_group(Cell* cell, int cls)
{
    auto it = groups->find(cls);
    if (it == groups->end())
        groups->emplace_hint(it, cls, std::vector<Cell*>(1, cell)); // not found
    else
        it->second.push_back(cell); //found
}

void Separator::init_funct(set<Cell*>& opened, cv::Mat& output, bool init_cellmat)
{
    Cell* cell = nullptr;

    // Find a pixel with negative value that we will start growing from
    // (restore searching from pixel used in previous iteration instead always starting from zero)
    for (int col = last_col+1; col < output.cols; ++col)
    {
        if (output.at<int_t>(last_row,col) < 0)
        {
            cell = &(*cell_mat)[last_row][col];
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
                    cell = &(*cell_mat)[row][col];
                    last_row = row;
                    last_col = col;
                    break;
                }
            }

            if (cell != nullptr) break; 
        }
    if (cell == nullptr) return;

    // Open the found cell and assign it new area id
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



size_t Separator::compute(cv::Mat& input_data, cv::Mat& output_data,std::unique_ptr<Groups>&& groups_init, std::unique_ptr<CellMat>&& cellmat_init)
{
    if (groups == nullptr)
        this->groups = make_unique<Groups>();
    else
        this->groups = move(groups_init);
    
    bool create_new_cellmat = false;
    if (cellmat_init == nullptr)
        create_new_cellmat = true;
    else
        cell_mat = move(cellmat_init);

    cv::Mat data = input_data.clone();
    this->input_data = &input_data;
    this->last_row = 0;
    this->last_col = 0;

    // Transform the data in a way that the background value is zero and there are no pixels with 
    // positive values (so we can assign them positive values in following iterations) 
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


    if (create_new_cellmat)
    {
        // First initialization of cell_mat
        cell_mat = make_unique<CellMat>(
            data.rows, vector<Cell>(
            data.cols, Cell(0, 0, State::unseen))
        );
    }
    for (size_t row = 0; row < data.rows; ++row)
        for (size_t col = 0; col < data.cols; ++col)
        {
            Cell& c = (*cell_mat)[row][col];
            if (create_new_cellmat)
            {
                c.row = row;
                c.col = col;
            }

            c.state = (data.at<int_t>(row,col) < 0 ?
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
    if (treshold > 0)
    {
        auto tg = AfterTresholdGrowing(input_data.rows, input_data.cols);
        tg.compute(data, data, move(groups), move(cell_mat));
        groups = tg.clear_groups();
        cell_mat = tg.clear_cellmat();
    }
    

    this->input_data = nullptr;
    output_data = move(data);
    return steps;
}



Separator::AfterTresholdGrowing::AfterTresholdGrowing(int rows, int cols)
: Growing(Neighborhood::n4), rows(rows), cols(cols)
{}

void Separator::AfterTresholdGrowing::init_funct(set<Cell*>& opened, cv::Mat& output, bool create_new_cellmat)
{
    if (create_new_cellmat == true)
        throw logic_error(
            string(nameof(AfterTresholdGrowing))
            .append(" doesn't support call of ")
            .append(nameof(init_funct))
            .append(" with ")
            .append(nameof(create_new_cellmat))
            .append(" == True!")
        );

    bool bool_4_8 = (neighborhood == Neighborhood::n4);

    // Find the border of "background" pixels
    auto it = groups->find(0);
    if (it != groups->end())
    {
        auto bg = unordered_set<Cell*>();
        bg.reserve(it->second.size());
        for (auto& x : it->second)
            bg.insert(x);
        
        for (auto& cell : it->second)
        {
            for (auto& neighbor : get_neighbors(cell, bool_4_8, cols, rows))
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

void Separator::AfterTresholdGrowing::Remap(cv::Mat& data)
{
    /* */
    if (groups->size() == 0)
        return;
    auto it = groups->begin();
    
    if (it->first < 0)
        throw logic_error("Error: Found negative group id!");
    
    if (it->first == 0)
        ++it;

    int n = 1;
    while (it != groups->end())
    {
        if (it->first != n)
        {
            auto node = groups->extract(it);
            node.key() = n;
            groups->insert(std::move(node));
            ++n;
        }
        ++it;
    }

    for (auto& iter : *groups)
        if (iter.first != data.at<int16_t>(iter.second[0]->row, iter.second[0]->col))
            for (auto& c : iter.second)
                assign(data, iter.first, c);

}

size_t Separator::AfterTresholdGrowing::compute(
    cv::Mat& data,
    cv::Mat& output,
    std::unique_ptr<Groups>&& groups,
    std::unique_ptr<CellMat>&& cell_mat)
{
    auto x = Growing::compute(data,output,move(groups),move(cell_mat));
    Remap(output);
    return x;
}