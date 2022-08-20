#include "clustering.hpp"
using namespace std;

Clustering::Clustering(size_t treshold) 
: Growing(Neighborhood::n4), treshold(treshold)
{

}

void Clustering::init_funct(set<Cell*>& opened, vector<vector<Cell>>& cell_mat, cv::Mat& data)
{
    cell_mat = this->cell_mat; // restore cell_mat from previous iteration
    Cell* cell = nullptr;


    //restore searching from pixel used in previous iteration instead always starting from zero
    for (int col = last_col+1; col < data.cols; ++col)
    {
        if (data.at<mat_t>(last_row,col) == 0)
        {
            cell = &cell_mat[last_row][col];
            last_col = col;
            break;
        }
    }
    if (cell == nullptr)
        for (int row = last_row+1; row < data.rows; ++row)
        {
            for (int col = 0; col < data.cols; ++col)
            {
                if (data.at<mat_t>(row,col) == 0)
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
    data.at<mat_t>(cell->row,cell->col) = n;
    opened.insert(cell);
}

void Clustering::post_funct(std::vector<Cell*>& processed, cv::Mat& data)
{
    if (processed.size() < treshold)
        for (auto cell : processed)
            data.at<mat_t>(cell->row, cell->col) = -1;
}

size_t Clustering::compute(cv::Mat& data)
{
    n = 0;
    last_row = 0;
    last_col = 0;

    // Map (background: 0 -> -1), (objects: 255 -> 0)
    data /= 255;
    data -= 1;

    // First initialization of cell_mat
    cell_mat = vector<vector<Cell>>(
        data.rows, vector<Cell>(
        data.cols, Cell(0, 0, State::unseen)));
    for (size_t row = 0; row < data.rows; ++row)
        for (size_t col = 0; col < data.cols; ++col)
        {
            cell_mat[row][col].row = row;
            cell_mat[row][col].col = col;
            cell_mat[row][col].state = (data.at<mat_t>(row,col) == 0 ?
                State::unseen : State::closed);
        }


    n = 1;
    size_t steps = 0;
    while (true)
    {
        size_t s = Growing::compute(data);
        steps += s;
        if (s == 0)
            break;
        n++; 
    }

    data += 1;
    return steps;
}