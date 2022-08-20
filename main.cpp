#include <iostream>
#include <chrono>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "clustering.hpp"
#include "voronoi.hpp"

using namespace std;

#include <functional>

template <typename units = chrono::milliseconds, typename clock = chrono::steady_clock>
int64_t measure_time(function<void(void)> fun)
{
    auto start_time = clock::now();
    fun();
    auto end_time = clock::now();
    return chrono::duration_cast<units>(end_time - start_time).count();

}


void smoothEdges(cv::InputArray src, cv::OutputArray dst, int ksize=11, int iter=5)
{
    cv::pyrUp(src,dst);
    for (int i = 0; i < iter; ++i)
        cv::medianBlur(dst,dst,ksize);
    cv::pyrDown(dst,dst);
}






int main( int argc, char** argv)
{
    //string filename = "img/lena.jpg";
    string filename = "img/kupka_pts.png";
    cv::Mat data;
    data = cv::imread(filename,cv::IMREAD_GRAYSCALE);
    if(! data.data)
    {
        cout<< "Could not open file" << endl;
        return -1;
    }

    Clustering clustering(0);
    Voronoi voronoi;

    data.convertTo(data, CV_16S);
    /////////////////////////////////////////////////////////////
    cout << measure_time([&](){     clustering.compute(data);    })/1e3 << endl;

    cout << measure_time([&](){     voronoi.compute(data);       })/1e3 << endl;

    
    //TODO: Map int16_t directly to RGB color instead of % 256
    for (int row = 0; row < data.rows; ++row)
        for (int col = 0; col < data.cols; ++col)
            data.at<int16_t>(row,col) %= 256;
    

    /////////////////////////////////////////////////////////////
    data.convertTo(data, CV_8U);


    // Randomly remap classes
    auto lut = cv::Mat(256, 1, CV_8U);
    for (int i = 0; i < 256; ++i)
        lut.at<int8_t>(i,0) = i;
    //cv::randShuffle(lut);
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

   cout << measure_time([&](){     smoothEdges(data,data);    })/1e3 << endl;
   
    

    cv::namedWindow("result", cv::WINDOW_AUTOSIZE);
    cv::imshow("result", data);

    int exit = false;
    while(!exit)
    {
        int keycode = cv::waitKey() & 0xEFFFFF;
        //cout << keycode << endl;
        exit = keycode != 3 && keycode != 225 && keycode != 227 && keycode != 228 && keycode != 229 && keycode != 233;
        //           alt-gr             shift         left-ctrl         right-ctrl        caps-lock               alt
    };

    return 0;
}