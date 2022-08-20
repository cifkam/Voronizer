#include "voronoi.hpp"

using namespace std;


Voronoi::Voronoi() : Growing(Neighborhood::swap)
{

}

void Voronoi::init_funct(set<Cell*>& opened, vector<vector<Cell>>& cell_mat, cv::Mat& data)
{
    for (int row = 0; row < data.rows; ++row)
        for (int col = 0; col < data.cols; ++col)
        {
            if (data.at<mat_t>(row,col) != 0)
            {
                Cell* cell = &cell_mat[row][col];
                cell->state = State::opened;
                opened.insert(cell);
            }
        }
}