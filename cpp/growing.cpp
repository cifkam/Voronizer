//#define NDEBUG // uncomment to disable assert()

#include <iostream>
#include <cassert>
#include "growing.hpp"

using namespace std;

Pixel::Pixel(int row, int col, State state)
: row(row), col(col), state(state)
{}

bool operator<(const Pixel& lhs, const Pixel& rhs)
{
    return lhs.row < rhs.row || (lhs.row == rhs.row && lhs.col < rhs.col);
}


Growing::Growing(Neighborhood neighborhood)
 : neighborhood(neighborhood)
{}

void Growing::post_funct(std::vector<Pixel*>& processed, cv::Mat& data)
{
    for (auto& pixel : processed)
    {
        int cls = data.at<int_t>(pixel->row, pixel->col);
        add_to_group(pixel, cls);
    }
}

void Growing::add_to_group(Pixel* pixel, int cls)
{
    auto it = groups->find(cls);
    if (it == groups->end())
        groups->emplace_hint(it, cls, std::vector<Pixel*>(1, pixel)); // not found
    else
        it->second.push_back(pixel); //found
}
    

void Growing::assign(cv::Mat& data, Pixel* from, Pixel* to)
{
    data.at<int_t>(to->row, to->col) = data.at<int_t>(from->row, from->col);
}

void Growing::assign(cv::Mat& data, int value, Pixel* to)
{
    data.at<int_t>(to->row, to->col) = value;
}


std::unique_ptr<Groups> Growing::clear_groups()
{
    auto g = move(groups);
    groups = nullptr;
    return g;
}

std::unique_ptr<PixelMat> Growing::clear_pixelmat()
{
    auto c = move(pixel_mat);
    pixel_mat = nullptr;
    return c;
}


bool Growing::grow_condition(const cv::Mat& input_data, const cv::Mat& data, Pixel* pixel, Pixel* neighbor)
{
    return neighbor->state == State::unseen;   
}

size_t Growing::compute(cv::Mat& input_data, cv::Mat& output_data, unique_ptr<Groups>&& groups_init, unique_ptr<PixelMat>&& pixelmat_init)
{
    cv::Mat data = input_data.clone();
    if (groups_init == nullptr)
        groups = make_unique<Groups>();
    else
        groups = move(groups_init);

    bool create_new_pixelmat = false;
    if (pixelmat_init == nullptr)
        create_new_pixelmat = true;
    else
        pixel_mat = move(pixelmat_init);

    size_t steps = compute_inner(data, create_new_pixelmat);
    output_data = move(data);
    return steps;
}

size_t Growing::compute_inner(cv::Mat& data, bool create_new_pixelmat)
{
    assert(data.depth() == CV_16S);

    bool bool_4_8 = (neighborhood == Neighborhood::n4);
    set<Pixel*> opened;
    init_funct(opened, data, create_new_pixelmat);
    
    size_t steps = 0;

    vector<Pixel*> processed;

    while (opened.size() > 0)
    {
        set<Pixel*> new_pixels;
        for (auto pixel : opened)
        {
            for (auto&& neighbor : get_neighbors(pixel, bool_4_8, data.cols, data.rows))
                if (grow_condition(data, data, pixel, neighbor))
                {
                    new_pixels.insert(neighbor);
                    assign(data, pixel, neighbor);
                    neighbor->state = State::opened;
                }
            
            pixel->state = State::closed;
            processed.push_back(pixel);
        }

        if (neighborhood == Neighborhood::alternating)
            bool_4_8 = !bool_4_8;
        
        opened = move(new_pixels);
        steps += 1;
    }
    post_funct(processed, data);
    return steps;
};


vector<Pixel*> Growing::get_neighbors(
    const Pixel* pixel,
    bool bool_4_8,
    size_t cols,
    size_t rows
){
    vector<int> row_delta;
    vector<int> col_delta;
    vector<Pixel*> neighbors;

    if (pixel->row == 0)           row_delta = {0,1};
    else if (pixel->row == rows-1) row_delta = {-1,0};
    else                          row_delta = {-1,0,1};

    if (pixel->col == 0)           col_delta = {0,1};
    else if (pixel->col == cols-1) col_delta = {-1,0};
    else                          col_delta = {-1,0,1};

    if (bool_4_8)
    {
        for (auto r : row_delta)
        {
            if (r == 0) continue;
            Pixel* c = &(*pixel_mat)[pixel->row+r][pixel->col]; 
            neighbors.push_back(c);
        }
        for (auto c : col_delta)
        {
            if (c == 0) continue;
            neighbors.push_back(&(*pixel_mat)[pixel->row][pixel->col+c]);
        }
    }
    else
    {
        for (auto r : row_delta)
            for (auto c : col_delta)
            {
                if (r == 0 && c == 0) continue;
                neighbors.push_back(&(*pixel_mat)[pixel->row+r][pixel->col+c]);
            }
    }

    return neighbors;
}



