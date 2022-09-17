#include <iostream>
#include <chrono>
#include <functional>
#include <filesystem>

#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/features2d.hpp>
#include <argparse/argparse.hpp>

#include "clustering.hpp"
#include "voronoi.hpp"
#include "utils.hpp"

using namespace std;
namespace fs = std::filesystem;
typedef function<cv::Mat(const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups)> color_funct_t;

cv::Mat sobel_voronizer(
    cv::Mat& input,
    color_funct_t colorize_funct,
    size_t treshold = 15
){
    
    cv::Mat data;
    cv::cvtColor(input, data, cv::COLOR_RGB2GRAY);
    cv::medianBlur(data, data, 7);
    cv::Sobel(data, data, CV_8U, 1, 1, 5, 1.0);
    cv::threshold(data, data, 30, 255, cv::THRESH_BINARY); //TODO: add treshold as parameter
    cv::medianBlur(data, data, 5);

    Clustering clustering(treshold);
    data.convertTo(data, CV_16S);
    clustering.compute(data, data);
    
    Voronoi voronoi;
    voronoi.groups = clustering.clear_groups();
    voronoi.compute(data, data, false);
    auto groups = voronoi.clear_groups();

    return colorize_funct(input, data, groups);
}

cv::Mat kmeans_voronizer(
    const cv::Mat& input,
    color_funct_t colorize_funct,
    size_t n_colors = 10,
    size_t treshold = 3
){
    cv::Mat data2;
    cv::Mat data;
    cv::medianBlur(input, data2, 5); // apply median filter to speed-up the process and remove small regions
    kmeans_color(data2, data, n_colors);
    cv::cvtColor(data, data, cv::COLOR_RGB2GRAY);
    random_LUT(data, data);

    Clustering clustering(treshold, -1);
    data.convertTo(data, CV_16S);
    clustering.compute(data, data);
    auto groups = clustering.clear_groups();
    groups.erase(0);
    
    cv::Mat im = cv::Mat::zeros(input.size(), CV_16S);
    for (auto& group : groups)
    {
        size_t row = 0;
        size_t col = 0;
        for (auto& cell : group.second)
        {
            row += cell->row;
            col += cell->col;
        }
        row /= group.second.size();
        col /= group.second.size();
        im.at<int16_t>(row,col) = group.first;
    }

    Voronoi voronoi;
    voronoi.compute(im, im);
    groups = voronoi.clear_groups();
    return colorize_funct(input, im, groups);
}

void help_exit(const string& message, const argparse::ArgumentParser args, int exitcode=1)
{
    cerr << message << endl << args;
    exit(exitcode);
}


bool run(const string& img_path, size_t treshold, const string& mode, bool cmap, cv::ColormapTypes cmap_type, bool random, uint smooth)
{
    cv::Mat img = cv::imread(img_path, cv::IMREAD_COLOR);
    cv::Mat data;
    cv::cvtColor(img, data, cv::COLOR_RGB2GRAY);
    if(!data.data)
    {
        cerr << "Could not open file" << endl;
        return false;
    }

    color_funct_t color_cmap_lambda   =   [&](const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups)
        { return colorize_by_cmap(voronoi_output, cv::COLORMAP_TWILIGHT, true, random);};
    color_funct_t color_template_lambda = [&](const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups)
        { return colorize_by_template(input, voronoi_groups);};
        
    color_funct_t color_lambda = cmap ? color_cmap_lambda : color_template_lambda;
    cv::Mat result(img.size(), CV_8UC3);
    if (mode == "sobel")
    {
        result = sobel_voronizer(img, color_lambda, treshold);

    }
    else if (mode == "kmeans")
    {
        result = kmeans_voronizer(img, color_lambda);
    }
    else
    {
        throw logic_error("Mode not yet implemented");
    }

    if (smooth > 0)
        smoothEdges(result, result, 9, smooth);
    
    imshow(result, "result");
    return true;
}



int main( int argc, char** argv)
{
    argparse::ArgumentParser args("Voronoizer", "1.0");
    args.add_argument("-i", "--img")
        .help("path to image to voronize")
        .required();

    args.add_argument("-m", "--mode")
        .help("voronizer mode: {sobel, kmeans}\n\t\t\"sobel\": ...\n\t\t\"kmeans: ...\"\n\t\t")
        .default_value<string>("sobel")
        .required();

    args.add_argument("-c", "--colormap")
        .help("cv2 colormap name to use instead of original image as template: {autumn, bone, jet, winter, rainbow, ocean, summer, spring, cool, hsv, pink, hot, parula, magma, inferno, plasma, viridis, cividis, twilight, twilight_shifted, turbo}")
        .default_value<string>("");

    args.add_argument("-r", "--random")
        .help("has effect only if using colormap: shuffle colors of areas randomly, otherwise color will depend on x and y coordinate of area")
        .default_value(false)
        .implicit_value(true);

    args.add_argument("-s", "--smooth")
        .help("smooth edges")
        .default_value((uint)3)
        .required()
        .scan<'u', uint>();

    args.add_argument("-t", "--treshold")
        .help("ignore areas with number of pixels less than treshold")
        .default_value((uint)0)
        .required()
        .scan<'u', uint>();

    try
    {
        args.parse_args(argc, argv);
    }
    catch (const exception& err)
    {
        help_exit(err.what(), args);
    }

    string mode = args.get("-m"); 
    bool cmap = (args.get("-c") != "");
    cv::ColormapTypes cmap_type;
    bool random = args.get<bool>("-r");
    uint smooth = args.get<uint>("-s");
    uint treshold = args.get<uint>("-t");
    string img_path = args.get("-i");

    if (!fs::exists(img_path) || fs::is_directory(img_path))
        help_exit("File does not exist: " + img_path, args);
    if (mode != "sobel" && mode != "kmeans")
        help_exit("Unrecognized mode: " + mode, args);
    if (treshold < 0)
        help_exit("Treshold must be nonnegative integer, found: " + to_string(treshold), args);
    if (cmap && !str_to_colormap(args.get<string>("-c"), cmap_type))
        help_exit("Unrecognized colormap: " + args.get<string>("-c"), args);

    run(img_path, treshold, mode, cmap, cmap_type, random, smooth);

    return 0;
}