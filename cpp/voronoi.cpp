#include "voronoi.hpp"

using namespace std;


Voronoi::Voronoi() : Growing(Neighborhood::alternating)
{

}

void Voronoi::init_funct(set<Cell*>& opened, cv::Mat& output, bool create_new_cellmat)
{
    if (create_new_cellmat)
    {
        cell_mat = make_unique<CellMat>(
            output.rows, vector<Cell>(
            output.cols, Cell(0, 0, State::unseen)
        ));
    }

    for (int row = 0; row < output.rows; ++row)
        for (int col = 0; col < output.cols; ++col)
        {
            Cell* cell = &(*cell_mat)[row][col];
            if (create_new_cellmat)
            {
                cell->row = row;
                cell->col = col;
            }

            if (output.at<int_t>(row,col) != 0)
            {
                cell->state = State::opened;
                opened.insert(cell);
            }
            else
            {
                cell->state = State::unseen;
            }
        }
}
