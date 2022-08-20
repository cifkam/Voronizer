#include "clustering.hpp"
using namespace std;

Clustering::Clustering(size_t treshold) 
: Growing(Neighborhood::n4), n(0), treshold(treshold)
{

}

void Clustering::init_funct(set<Cell*>& opened, vector<vector<Cell>>& cell_mat, cv::Mat& data)
{
    Cell* cell = nullptr;

    for (int row = 0; row < data.rows; ++row)
    {
        for (int col = 0; col < data.cols; ++col)
            cell_mat[row][col].state = (data.at<mat_t>(row,col) == 0 ?
                State::unseen : State::closed);
    }
    for (int row = 0; row < data.rows; ++row)
    {
        for (int col = 0; col < data.cols; ++col)
        {
            if (data.at<mat_t>(row,col) == 0)
            {
                cell = &cell_mat[row][col];
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
    data /= 255;
    data -= 1;

    n = 1;
    size_t steps = 0;
    while (true)
    {
        //TODO: reuse cell_mat instead of creating it always again
        size_t s = Growing::compute(data);
        steps += s;
        if (s == 0)
            break;
        n++; 
    }

    n = 0;
    data += 1;
    //data.convertTo(data, CV_8U);
    return steps;


}