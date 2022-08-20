#include <iostream>
#include <functional>
#include <set>
#include <limits>
#include <stdint.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>


#include "growing.hpp"

using namespace std;


template <typename T>
void set_mat(cv::Mat& mat, int i, int j, T value)
{
    mat.at<T>(i,j) = value;
}

template <typename T>
T get_mat(cv::Mat& mat, int i, int j)
{
    return mat.at<T>(i,j);
}

int main( int argc, char** argv )
{
    //string filename = "lena.jpg";
    string filename = "kupka_pts_1.png";
    cv::Mat data;
    data = cv::imread(filename,cv::IMREAD_GRAYSCALE);
    if(! data.data)
    {
        cout<< "Could not open file" << endl;
        return -1;
    }
    
    Clustering().run(data);
    Voronoi().run(data);

    //TODO: Map int16_t directly to RGB color instead of % 256
    for (int row = 0; row < data.rows; ++row)
        for (int col = 0; col < data.cols; ++col)
            data.at<int16_t>(row,col) %= 256; 
    data.convertTo(data, CV_8UC1);

    // Randomly map classes to
    auto lut = cv::Mat(256, 1, CV_8UC1);
    cv::randu(lut, 0, 256);
    cv::LUT(data, lut, data);
    cv::applyColorMap(data, data, cv::COLORMAP_TWILIGHT);
    
    /*   
    cv::cvtColor(data, data, cv::COLOR_GRAY2RGB);
    vector<cv::Mat> channels;
    cv::split(data, channels);
    auto lut0 = cv::Mat(256, 1, CV_8UC1);
    auto lut1 = cv::Mat(256, 1, CV_8UC1);
    auto lut2 = cv::Mat(256, 1, CV_8UC1);
    cv::randu(lut0, 0, 256);
    cv::randu(lut1, 0, 256);
    cv::randu(lut2, 0, 256);
    cv::LUT(channels[0], lut0, channels[0]);
    cv::LUT(channels[1], lut1, channels[1]);
    cv::LUT(channels[2], lut2, channels[2]);
    cv::merge(channels, data);
    */

   

    

    cv::namedWindow("result", cv::WINDOW_AUTOSIZE);
    cv::imshow("result", data);
    cv::waitKey(0);
    return 0;
}