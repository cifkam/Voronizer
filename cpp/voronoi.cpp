#include "voronoi.hpp"

using namespace std;


Voronoi::Voronoi() : Growing(Neighborhood::alternating)
{

}

void Voronoi::init_funct(set<Pixel*>& opened, cv::Mat& output, bool create_new_pixelmat)
{
    if (create_new_pixelmat)
    {
        pixel_mat = make_unique<PixelMat>(
            output.rows, vector<Pixel>(
            output.cols, Pixel(0, 0, State::unseen)
        ));
    }

    for (int row = 0; row < output.rows; ++row)
        for (int col = 0; col < output.cols; ++col)
        {
            Pixel* pixel = &(*pixel_mat)[row][col];
            if (create_new_pixelmat)
            {
                pixel->row = row;
                pixel->col = col;
            }

            if (output.at<int_t>(row,col) != 0)
            {
                pixel->state = State::opened;
                opened.insert(pixel);
            }
            else
            {
                pixel->state = State::unseen;
            }
        }
}
