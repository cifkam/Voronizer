#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
#include <sstream>
#include <type_traits>
#include <opencv2/imgproc.hpp>
#include "growing.hpp"


#include <chrono>
#include <functional>
template <typename units = std::chrono::milliseconds, typename clock = std::chrono::steady_clock>
int64_t measure_time(std::function<void(void)> fun);



void waitKey();
bool str_to_colormap(std::string name, cv::ColormapTypes& output);
void imshow(const cv::Mat& image, const std::string& winname = "", bool wait_key = true);
void random_LUT(const cv::Mat& src, cv::Mat& dst);
void smoothEdges(cv::InputArray src, cv::OutputArray dst, int ksize=9, int iter=3);
cv::Mat colorize_by_cmap(const cv::Mat& input, cv::ColormapTypes map = cv::COLORMAP_TWILIGHT, bool copy = true, bool apply_random_LUT = false);
cv::Mat colorize_by_template(const cv::Mat& color_template, const Groups& voronoi_groups);
void kmeans_color(cv::Mat ocv, cv::Mat& output, int K);




template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, bool>::type tryParse(const std::string& s, T& output)
{
    try
    {
        std::stringstream stream(s);
        T value;
        char test;

        if ((!(stream >> value)) || (stream >> test))
        {
            return false;
        }

        for (size_t i = 0; i < s.size(); ++i)
        {
            if (s[i] == '-')
                return false;
            else if (isdigit(s[i]) || s[i] == '+')
                break;
        }

        output = value;
        return true;
    }
    catch(...)
    {
        return false;
    }
}

template <>
typename std::enable_if<std::is_unsigned<bool>::value, bool>::type tryParse(const std::string& s, bool& output);

template <typename T>
typename std::enable_if<!std::is_unsigned<T>::value, bool>::type tryParse(const std::string& s, T& output)
{
    try
    {
        std::stringstream stream(s);
        char test;

        if ((!(stream >> output)) || (stream >> test))
        {
            return false;
        }
        return true;
    }
    catch(...)
    {
        return false;
    }
}


#endif /* UTILS_HPP */