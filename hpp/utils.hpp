#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>
#include <type_traits>
#include <limits>
#include <random>
#include <vector>
#include <sstream>
#include <string>
#include <opencv2/imgproc.hpp>

#include "growing.hpp"

#define nameof(x) #x


#include <chrono>
#include <functional>
template <typename units = std::chrono::milliseconds, typename clock = std::chrono::steady_clock>
int64_t measureTime(std::function<void(void)> fun)
{
    auto start_time = clock::now();
    fun();
    auto end_time = clock::now();
    return std::chrono::duration_cast<units>(end_time - start_time).count();

}



void waitKey();
bool strToColormap(std::string name, cv::ColormapTypes& output);
void imshow(const cv::Mat& image, const std::string& winname = "", bool wait_key = true);
void randomLUT(const cv::Mat& src, cv::Mat& dst);
void smoothEdges(cv::InputArray src, cv::OutputArray dst, int ksize=9, int iter=3);
cv::Mat colorizeByCmap(const cv::Mat& input, cv::ColormapTypes map = cv::COLORMAP_TWILIGHT, bool copy = true, bool apply_random_LUT = false);
cv::Mat colorizeByTemplate(const cv::Mat& color_template, const Groups* voronoi_groups);
void kmeansColor(cv::Mat ocv, cv::Mat& output, int K);

cv::Mat linesFromClosestPointsRandom(std::vector<cv::Point2f>& pts, cv::Size image_size, size_t iter, size_t pts_left_out = 3);


template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, bool>::type tryParse(const std::string& s, T& output) noexcept
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
typename std::enable_if<std::is_unsigned<bool>::value, bool>::type tryParse(const std::string& s, bool& output) noexcept;

template <typename T>
typename std::enable_if<!std::is_unsigned<T>::value, bool>::type tryParse(const std::string& s, T& output) noexcept
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

template <typename T>
std::string to_string(const std::vector<T>& vec)
{
	if (vec.size() == 0)
		return "{}";

	std::stringstream ss;
	ss << "{";
	for (int i = 0; i < vec.size()-1; ++i)
		ss << vec[i] << ", ";
	ss << vec[vec.size()-1] << "}"; 
	return ss.str();
}



#endif /* UTILS_HPP */