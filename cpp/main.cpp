#include <iostream>
#include <chrono>
#include <functional>
#include <filesystem>
#include <algorithm>
#include <unordered_set>
#include <memory>

#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/features2d.hpp>
#include <argparse/argparse.hpp>

#include "separator.hpp"
#include "voronoi.hpp"
#include "utils.hpp"
#include "voronizer.hpp"

using namespace std;
namespace fs = std::filesystem;
typedef function<cv::Mat(const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups)> color_funct_t;

argparse::ArgumentParser args;


void help_exit(const string& message, int exitcode=1)
{
    cerr << message << endl << args;
    exit(exitcode);
}


bool run(const string& img_path, const string& mode, bool cmap, cv::ColormapTypes cmap_type, bool random, uint smooth, const string& options)
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
        { return colorize_by_cmap(voronoi_output, cmap_type, true, random);};
    color_funct_t color_template_lambda = [&](const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups)
        { return colorize_by_template(input, voronoi_groups);};
        
    color_funct_t color_lambda = cmap ? color_cmap_lambda : color_template_lambda;
    cv::Mat result(img.size(), CV_8UC3);
    
    unique_ptr<AbstractVoronizer> voronizer;
    if (mode == "sobel")
        voronizer = SobelVoronizer::create(options);
    else if (mode == "kmeans")
        voronizer = KMeansVoronizer::create(options);
    else if (mode == "sift")
        voronizer = SIFTVoronizer::create(options);
    else
        throw logic_error("Mode not yet implemented");

    if (voronizer == nullptr)
        help_exit("Couldn't parse options");

    if (cmap)
        voronizer->set_colormap(cmap_type, random);
    result = voronizer->run(img); 
    
    if (smooth > 0)
        smoothEdges(result, result, 9, smooth);
    
    imshow(result, "result");
    return true;
}


#include <cassert>
int main( int argc, char** argv)
{
    //SIFTVoronizer(size_t kp_size_tresh, int radius, int thickness, float radius_mult)
    /*assert(SIFTVoronizer::create("") != nullptr);

    assert(SIFTVoronizer::create(",") != nullptr);
    assert(SIFTVoronizer::create(",,") != nullptr);
    assert(SIFTVoronizer::create(",,,") != nullptr);
    assert(SIFTVoronizer::create(",,,,") == nullptr);

    assert(SIFTVoronizer::create("0,") != nullptr);
    assert(SIFTVoronizer::create("1") != nullptr);
    assert(SIFTVoronizer::create("-1") == nullptr);
    assert(SIFTVoronizer::create("a") == nullptr);

    assert(SIFTVoronizer::create("1.0,") == nullptr);
    assert(SIFTVoronizer::create("1,-1,33,") != nullptr);
    assert(SIFTVoronizer::create("1,-1,33,1.5") != nullptr);
    assert(SIFTVoronizer::create("1,-1,,1.5") != nullptr);*/


    args = argparse::ArgumentParser("Voronoizer", "1.0");
    args.add_argument("-i", "--img")
        .help("path to image to voronize")
        .required();

    unordered_set<string> modes = {"sobel", "kmeans", "sift"};
    args.add_argument("-m", "--mode")
        .help("voronizer mode: {sobel, kmeans, sift}\n"
        "\t\t   sobel:  ...\n"
        "\t\t      options: \n"
        "\t\t   kmeans: ...\n"
        "\t\t      options: \n"
        "\t\t   sift:   ...\n"
        "\t\t      options: \n"
        "\t\t")
        .default_value<string>("sobel")
        .required();

    args.add_argument("-c", "--colormap")
        .help("OpenCV colormap name to use instead of original image as template: {autumn, bone, jet, winter, rainbow, ocean, summer, spring, cool, hsv, pink, hot, parula, magma, inferno, plasma, viridis, cividis, twilight, twilight_shifted, turbo}")
        .default_value<string>("");

    args.add_argument("-f", "--file") //TODO: implement
        .help("Write output to file instead of displaying it in a window")
        .default_value<string>("");

    args.add_argument("-r", "--random")
        .help("Has effect only if using colormap: shuffle colors of areas randomly, otherwise color will depend on x and y coordinate of area")
        .default_value(false)
        .implicit_value(true);

    args.add_argument("-s", "--smooth")
        .help("Strength of edges smoothing")
        .default_value<uint>(3)
        .required()
        .scan<'u', uint>();

    args.add_argument("-o", "--options") //TODO: implement
        .help("Comma separated list of mode-specific positional options."
        "\n\t\tSupports truncated list as well as using default value by omitting the value"
        "\n\t\t(e.g. \"-o ,,0.5\" will result in setting the third option to 0.5, all other options will keep their default values.)")        
        .default_value<string>("");

    try
    {
        args.parse_args(argc, argv);
    }
    catch (const exception& err)
    {
        help_exit(err.what());
    }

    string mode = args.get("-m");
    bool cmap = (args.get("-c") != "");
    cv::ColormapTypes cmap_type;
    bool random = args.get<bool>("-r");
    uint smooth = args.get<uint>("-s");
    string img_path = args.get("-i");
    string options = args.get("-o");

    if (!fs::exists(img_path) || fs::is_directory(img_path))
        help_exit("File does not exist: " + img_path);
    if (modes.find(mode) == modes.end())
        help_exit("Unrecognized mode: " + mode);
    if (cmap && !str_to_colormap(args.get<string>("-c"), cmap_type))
        help_exit("Unrecognized colormap: " + args.get<string>("-c"));

    run(img_path, mode, cmap, cmap_type, random, smooth, options);

    return 0;
}