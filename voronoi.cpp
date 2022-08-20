#include "voronoi.hpp"

using namespace std;


Voronoi::Voronoi() : Growing(Neighborhood::swap)
{

}

void Voronoi::init_funct(set<Cell*>& opened, vector<vector<Cell>>& cell_mat, cv::Mat& data)
{
    //TODO: simplify
    cell_mat = vector<vector<Cell>>(
        data.rows, vector<Cell>(
        data.cols, Cell(0, 0, State::unseen)
    ));
    
    for (size_t row = 0; row < data.rows; ++row)
        for (size_t col = 0; col < data.cols; ++col)
        {
            cell_mat[row][col].row = row;
            cell_mat[row][col].col = col;
        }


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