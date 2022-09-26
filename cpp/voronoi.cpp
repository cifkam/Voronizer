#include "voronoi.hpp"

using namespace std;


Voronoi::Voronoi() : Growing(Neighborhood::alternating)
{

}

void Voronoi::init_funct(set<Cell*>& opened, CellMat& cell_mat, cv::Mat& output)
{
    cell_mat = CellMat(
        output.rows, vector<Cell>(
        output.cols, Cell(0, 0, State::unseen)
    ));
    
    for (int row = 0; row < output.rows; ++row)
        for (int col = 0; col < output.cols; ++col)
        {
            cell_mat[row][col].row = row;
            cell_mat[row][col].col = col;
            if (output.at<int_t>(row,col) != 0)
            {
                Cell* cell = &cell_mat[row][col];
                cell->state = State::opened;
                opened.insert(cell);
            }
        }
}
