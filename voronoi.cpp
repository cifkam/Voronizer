#include <iostream>
#include <functional>
#include <set>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;

enum Neighborhood {n4, n8, swap};
enum State {unseen, opened, closed};

int main( int argc, char** argv )
{
    //string filename = "lena.jpg";
    string filename = "kupka_pts_1.png";
    std::function<set<int>(void)> x;
    cv::Mat image;
    image = cv::imread(filename,cv::IMREAD_COLOR);
    cout << image.size() << ", " << image.channels() << endl;
    cv::cvtColor(image,image,cv::COLOR_RGB2GRAY);
    cout << image.size() << ", " << image.channels() << endl;

    
    if(! image.data)
        {
            cout<< "Could not open file" << endl;
            return -1;
        }
    cv::namedWindow(filename, cv::WINDOW_AUTOSIZE);
    cv::imshow(filename, image);
    cv::waitKey(0);
    return 0;
}