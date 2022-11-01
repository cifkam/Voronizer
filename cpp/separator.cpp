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
: Growing(Neighborhood::n4), treshold(treshold), bg_value(bg_value), last_row(-1), last_col(-1), n(1), input_data(nullptr)
{

}

void Separator::add_to_group(Pixel* pixel, int cls)
{
    auto it = groups->find(cls);
    if (it == groups->end())
        groups->emplace_hint(it, cls, std::vector<Pixel*>(1, pixel)); // not found
    else
        it->second.push_back(pixel); //found
}

void Separator::init_funct(set<Pixel*>& opened, cv::Mat& output, bool init_pixelmat)
{
    Pixel* pixel = nullptr;

    // Find a pixel with negative value that we will start growing from
    // (restore searching from pixel used in previous iteration instead always starting from zero)
    for (int col = last_col+1; col < output.cols; ++col)
    {
        if (output.at<int_t>(last_row,col) < 0)
        {
            pixel = &(*pixel_mat)[last_row][col];
            last_col = col;
            break;
        }
    }
    if (pixel == nullptr)
        for (int row = last_row+1; row < output.rows; ++row)
        {
            for (int col = 0; col < output.cols; ++col)
            {
                if (output.at<int_t>(row,col) < 0)
                {
                    pixel = &(*pixel_mat)[row][col];
                    last_row = row;
                    last_col = col;
                    break;
                }
            }

            if (pixel != nullptr) break; 
        }
    if (pixel == nullptr) return;

    // Open the found pixel and assign it new area id
    pixel->state = State::opened;
    assign(output, n, pixel);
    opened.insert(pixel);
}

void Separator::post_funct(std::vector<Pixel*>& processed, cv::Mat& output)
{
    if (processed.size() < treshold)
    {
        for (auto pixel : processed)
        {
            assign(output, 0, pixel);
            add_to_group(pixel, 0);
        }
    }
    else
    {
        for (auto pixel : processed)
            add_to_group(pixel, n);
    }
}


bool Separator::grow_condition(const cv::Mat& input_data, const cv::Mat& data, Pixel* pixel, Pixel* neighbor)
{
    return (neighbor->state == State::unseen) && (this->input_data->at<int_t>(pixel->row,pixel->col) == this->input_data->at<int_t>(neighbor->row, neighbor->col));   
}



size_t Separator::compute(cv::Mat& input_data, cv::Mat& output_data,std::unique_ptr<Groups>&& groups_init, std::unique_ptr<PixelMat>&& pixelmat_init)
{
    if (groups == nullptr)
        this->groups = make_unique<Groups>();
    else
        this->groups = move(groups_init);
    
    bool create_new_pixelmat = false;
    if (pixelmat_init == nullptr)
        create_new_pixelmat = true;
    else
        pixel_mat = move(pixelmat_init);

    cv::Mat data = input_data.clone();
    this->input_data = &input_data;
    this->last_row = 0;
    this->last_col = 0;

    // Transform the data in a way that the background value is zero and there are no pixels with 
    // positive values (so we can assign them positive values in following iterations) 
    if (bg_value != 0)
        for (int row = 0; row < data.rows; ++row)
            for (int col = 0; col < data.cols; ++col)
            {
                int_t value = data.at<int_t>(row,col);

                if (value > bg_value)
                    data.at<int_t>(row,col) = -value;
                else
                    data.at<int_t>(row,col) = value - bg_value;
            }
    else
        data = -data; 


    if (create_new_pixelmat)
    {
        // First initialization of pixel_mat
        pixel_mat = make_unique<PixelMat>(
            data.rows, vector<Pixel>(
            data.cols, Pixel(0, 0, State::unseen))
        );
    }
    for (int row = 0; row < data.rows; ++row)
        for (int col = 0; col < data.cols; ++col)
        {
            Pixel& c = (*pixel_mat)[row][col];
            if (create_new_pixelmat)
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
        tg.compute(data, data, move(groups), move(pixel_mat));
        groups = tg.clear_groups();
        pixel_mat = tg.clear_pixelmat();
    }
    

    this->input_data = nullptr;
    output_data = move(data);
    return steps;
}



Separator::AfterTresholdGrowing::AfterTresholdGrowing(int rows, int cols)
: Growing(Neighborhood::n4), rows(rows), cols(cols)
{}

void Separator::AfterTresholdGrowing::init_funct(set<Pixel*>& opened, cv::Mat& output, bool create_new_pixelmat)
{
    if (create_new_pixelmat == true)
        throw logic_error(
            string(nameof(AfterTresholdGrowing))
            .append(" doesn't support call of ")
            .append(nameof(init_funct))
            .append(" with ")
            .append(nameof(create_new_pixelmat))
            .append(" == True!")
        );

    bool bool_4_8 = (neighborhood == Neighborhood::n4);

    // Find the border of "background" pixels
    auto it = groups->find(0);
    if (it != groups->end())
    {
        auto bg = unordered_set<Pixel*>();
        bg.reserve(it->second.size());
        for (auto& x : it->second)
            bg.insert(x);
        
        for (auto& pixel : it->second)
        {
            for (auto& neighbor : get_neighbors(pixel, bool_4_8, cols, rows))
            {
                auto c_it = bg.find(neighbor);
                if (c_it != bg.end())
                {
                    (**c_it).state = State::opened;
                    opened.insert(pixel);
                }

            }
        }
    }
    
}

void Separator::AfterTresholdGrowing::remap(cv::Mat& data)
{
    if (groups->size() == 0)
        return;

    unique_ptr<Groups> g = make_unique<Groups>();
    auto it = groups->begin();
    
    if (it->first < 0)
        throw logic_error("Error: Found negative group id!");
    
    if (it->first == 0)
    {
        (*g)[it->first] = move(it->second);
        ++it;
    }

    int n = 1;
    while (it != groups->end())
    {
        if (it->first != n)
        {
            (*g)[n] = move(it->second);
            ++n;
        }
        ++it;
    }
    swap(this->groups, g);

    for (auto& iter : *groups)
        if (iter.first != data.at<int16_t>(iter.second[0]->row, iter.second[0]->col))
            for (auto& c : iter.second)
                assign(data, iter.first, c);

}

size_t Separator::AfterTresholdGrowing::compute(
    cv::Mat& data,
    cv::Mat& output,
    std::unique_ptr<Groups>&& groups,
    std::unique_ptr<PixelMat>&& pixel_mat)
{
    auto x = Growing::compute(data,output,move(groups),move(pixel_mat));
    // because some of the areas were removed during tresholding, we need to remap the group IDs to [1..n]
    remap(output);
    return x;
}