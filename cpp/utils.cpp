#include "utils.hpp"

#include <map>
#include <iostream>

#include <opencv2/highgui.hpp>

using namespace std;

template <>
typename std::enable_if<std::is_unsigned<bool>::value, bool>::type tryParse(const std::string& s, bool& output) noexcept
{
    if (s == "true")
        output = true;
    else if (s == "false")
        output = false;
    else
        return false;
    return true;
}

void strLower(string& data)
{
    transform(data.begin(), data.end(), data.begin(),
        [](unsigned char c){ return tolower(c); });
}

bool strToColormap(string name, cv::ColormapTypes& output)
{
    strLower(name);
    map<string,cv::ColormapTypes> m =
    {
        {"bw", (cv::ColormapTypes)-1},
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
        {"deepgreen",cv::COLORMAP_DEEPGREEN}
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


// Shuffle values of the CV_8U image 
void randomLUT(const cv::Mat& src, cv::Mat& dst)
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

// Convert the CV_16S image to CV_8U (% 256) and apply colormap to create CV_8UC3 image
cv::Mat colorizeByCmap(const cv::Mat& input, cv::ColormapTypes map, bool copy, bool apply_random_LUT)
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
        randomLUT(data, data);
    if (map != (cv::ColormapTypes)-1) // cmap is not "bw"
        cv::applyColorMap(data, data, map);
    return data;
}

// Create an image from groups by setting the color of pixels in each group to an average color of the color_template in the area given by the group
cv::Mat colorizeByTemplate(const cv::Mat& color_template, const Groups* groups)
{
    cv::Mat data = cv::Mat::zeros(color_template.size(), CV_8UC3);

    for (auto&& cls : *groups)
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
        cv::Vec3b color((uchar)r,(uchar)g,(uchar)b);

        for (auto&& pixel : cls.second)
            data.at<cv::Vec3b>(pixel->row, pixel->col) = color;
    }

    return data;
}

// KMeans color clustering
void kmeansColor(cv::Mat ocv, cv::Mat& output, int K)
{
    // convert to float & reshape to a [3 x W*H] Mat 
    //  (so every pixel is on a row of it's own)
    cv::Mat data;
    ocv.convertTo(data, CV_32F);
    data = data.reshape(1, (int)data.total());

    // do kmeans
    cv::Mat labels, centers;
    cv::kmeans(data, K, labels, cv::TermCriteria(cv::TermCriteria::COUNT, 10, 1.0), 1, 
        cv::KMEANS_PP_CENTERS, centers);

    // reshape both to a single row of Vec3f pixels:
    centers = centers.reshape(3,centers.rows);
    data = data.reshape(3,data.rows);

    // replace pixel values with their center value:
    cv::Vec3f *p = data.ptr<cv::Vec3f>();
    for (int i=0; i<data.rows; i++) {
        int center_id = labels.at<int>(i);
        p[i] = centers.at<cv::Vec3f>(center_id);
    }
    // back to 2d, and uchar:
    output = data.reshape(3, ocv.rows);
    output.convertTo(output, CV_8U);
}
/*
For each point in "pts" (in random order), select "iter" random other (unused) points and draw a draw a line to the closest one.
You may specify, how many last points to leave out (points that will not be paired - may be useful, as there will be less points in the final iterations)
*/
cv::Mat linesFromClosestPointsRandom(std::vector<cv::Point2f>& pts, cv::Size image_size, size_t iter, size_t pts_left_out)
{
    if (pts.size()%2 != pts_left_out%2 && pts_left_out < 2)
        ++pts_left_out;

    random_device rd;  // obtain a random number from hardware
    mt19937 gen(rd()); // seed the generator
    auto rng = std::default_random_engine { rd() };

    int16_t n = 1;
    cv::Mat data = cv::Mat::zeros(image_size, CV_16S);
    while (pts.size() > pts_left_out)
    {
        uniform_int_distribution<> distr(0, (int)(pts.size()-2)); // define the range

        shuffle(pts.begin(), pts.end(), rng);
        cv::Point2f* a = &pts[pts.size()-1];
        cv::Point2f* b = nullptr;
        size_t b_index = -1;
        double dist = numeric_limits<double>::infinity();

        size_t N = min(iter, pts.size()-1);
        for (int i = 0; i < N; ++i)
        { 
            double d = cv::norm(*a-pts[i]);
            if (d < dist)
            {
                dist = d;
                b_index = i;
                b = &pts[i];
            }
        }
        if (b == nullptr) 
        {
            throw logic_error("Error: there are no points left!");
        }
        cv::line(data, *a, *b, n++, 2);
        std::swap(pts[b_index], pts[pts.size()-2]);
        pts.resize(pts.size()-2);
    }
    return data;
}

