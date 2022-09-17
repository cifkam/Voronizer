#include "utils.hpp"
#include <map>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace std;

void str_lower(string& data)
{
    transform(data.begin(), data.end(), data.begin(),
        [](unsigned char c){ return tolower(c); });
}

bool str_to_colormap(string name, cv::ColormapTypes& output)
{
    str_lower(name);
    map<string,cv::ColormapTypes> m =
    {
        {"autumn",cv::COLORMAP_AUTUMN},
        {"bone",cv::COLORMAP_BONE},
        {"jet",cv::COLORMAP_JET},
        {"winter",cv::COLORMAP_WINTER},
        {"rainbow",cv::COLORMAP_RAINBOW},
        {"ocean",cv::COLORMAP_OCEAN},
        {"summer",cv::COLORMAP_SUMMER},
        {"spring",cv::COLORMAP_SPRING},
        {"cool",cv::COLORMAP_COOL},
        {"hsv",cv::COLORMAP_HSV},
        {"pink",cv::COLORMAP_PINK},
        {"hot",cv::COLORMAP_HOT},
        {"parula",cv::COLORMAP_PARULA},
        {"magma",cv::COLORMAP_MAGMA},
        {"inferno",cv::COLORMAP_INFERNO},
        {"plasma",cv::COLORMAP_PLASMA},
        {"viridis",cv::COLORMAP_VIRIDIS},
        {"cividis",cv::COLORMAP_CIVIDIS},
        {"twilight",cv::COLORMAP_TWILIGHT},
        {"twilight_shifted",cv::COLORMAP_TWILIGHT_SHIFTED},
        {"turbo",cv::COLORMAP_TURBO},
        //{"deepgreen",cv::COLORMAP_DEEPGREEN}
    };

    auto it = m.find(name);
    if (it == m.end())
        return false;

    output = it->second;
    return true;
}


void waitKey()
{
    int exit = false;
    while(!exit)
    {
        int keycode = cv::waitKey() & 0xEFFFFF;
        //cout << keycode << endl;
        exit = keycode != 3 && keycode != 225 && keycode != 227 && keycode != 228 && keycode != 229 && keycode != 233;
        //           alt-gr             shift         left-ctrl         right-ctrl        caps-lock               alt
    }
}


cv::ColormapTypes string_to_colormap(string name);
void imshow(const cv::Mat& image, const string& winname, bool wait_key)
{
    cv::namedWindow(winname, cv::WINDOW_AUTOSIZE);
    cv::imshow(winname, image);
    if (wait_key)
    {
        waitKey();
        try { cv::destroyWindow(winname); }
        catch (exception e) {}
    }
}


void random_LUT(const cv::Mat& src, cv::Mat& dst)
{
    auto lut = cv::Mat(256, 1, CV_8U);
    for (int i = 0; i < 256; ++i)
        lut.at<int8_t>(i,0) = i;
    cv::randShuffle(lut);
    cv::LUT(src, lut, dst);
}

void smoothEdges(cv::InputArray src, cv::OutputArray dst, int ksize, int iter)
{
    cv::pyrUp(src,dst);
    for (int i = 0; i < iter; ++i)
        cv::medianBlur(dst,dst,ksize);
    cv::pyrDown(dst,dst);
}

cv::Mat colorize_by_cmap(const cv::Mat& input, cv::ColormapTypes map, bool copy, bool apply_random_LUT)
{
    cv::Mat data;
    if (copy) 
        input.copyTo(data);
    else
        data  = cv::Mat(input);

    for (int row = 0; row < data.rows; ++row)
        for (int col = 0; col < data.cols; ++col)
            data.at<int16_t>(row,col) %= 256;

    data.convertTo(data, CV_8U);
    if (apply_random_LUT)
        random_LUT(data, data);
    cv::applyColorMap(data, data, map);
    return data;
}

cv::Mat colorize_by_template(const cv::Mat& color_template, const Groups& voronoi_groups)
{
    cv::Mat data = cv::Mat::zeros(color_template.size(), CV_8UC3);
    

    for (auto&& cls : voronoi_groups)
    {
        uint64_t r(0),g(0),b(0);
        for (auto&& pixel : cls.second)
        {
            auto& c = color_template.at<cv::Vec3b>(pixel->row, pixel->col);
            r += c[0];
            g += c[1];
            b += c[2];
        }
        r /= cls.second.size();
        g /= cls.second.size();
        b /= cls.second.size();
        cv::Vec3b color(r,g,b);

        for (auto&& pixel : cls.second)
            data.at<cv::Vec3b>(pixel->row, pixel->col) = color;
    }

    return data;
}


void kmeans_color(cv::Mat ocv, cv::Mat& output, int K)
{
    // convert to float & reshape to a [3 x W*H] Mat 
    //  (so every pixel is on a row of it's own)
    cv::Mat data;
    ocv.convertTo(data,CV_32F);
    data = data.reshape(1,data.total());

    // do kmeans
    cv::Mat labels, centers;
    cv::kmeans(data, K, labels, cv::TermCriteria(cv::TermCriteria::COUNT, 10, 1.0), 1, 
        cv::KMEANS_PP_CENTERS, centers);

    // reshape both to a single row of Vec3f pixels:
    centers = centers.reshape(3,centers.rows);
    data = data.reshape(3,data.rows);

    // replace pixel values with their center value:
    cv::Vec3f *p = data.ptr<cv::Vec3f>();
    for (size_t i=0; i<data.rows; i++) {
        int center_id = labels.at<int>(i);
        p[i] = centers.at<cv::Vec3f>(center_id);
    }

    // back to 2d, and uchar:
    output = data.reshape(3, ocv.rows);
    output.convertTo(output, CV_8U);
}

