#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
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


#endif /* UTILS_HPP */