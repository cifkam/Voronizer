#include <iostream>
#include <chrono>
#include <functional>
#include <filesystem>
#include <algorithm>
#include <unordered_set>
#include <memory>
#include <sstream>
#include <algorithm>

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


bool run(const string& img_path, const string& mode, const string& options, bool cmap, cv::ColormapTypes cmap_type, bool random, uint smooth, const string& output_file)
{
    cv::Mat img = cv::imread(img_path, cv::IMREAD_COLOR);
    cv::Mat data;
    cv::cvtColor(img, data, cv::COLOR_RGB2GRAY);
    if(!data.data)
    {
        cerr << "Could not open file" << endl;
        return false;
    }

    cv::Mat result(img.size(), CV_8UC3);
    
    unique_ptr<AbstractVoronizer> voronizer;
    if (mode == "sobel")
        voronizer = SobelVoronizer::create(options);
    else if (mode == "kmeans-circles")
        voronizer = KMeansVoronizerCircles::create(options);
    else if (mode == "kmeans-lines")
        voronizer = KMeansVoronizerLines::create(options);
    else if (mode == "sift-circles")
        voronizer = SIFTVoronizerCircles::create(options);
    else if (mode == "sift-lines")
        voronizer = SIFTVoronizerLines::create(options);
    else
        throw logic_error("Mode not yet implemented");

    if (voronizer == nullptr)
        help_exit("Couldn't parse options");

    if (cmap)
        voronizer->set_colormap(cmap_type, random);
    result = voronizer->run(img); 
    
    if (smooth > 0)
        smoothEdges(result, result, 9, smooth);


    if (output_file != "")
        cv::imwrite(output_file, result);
    else
        imshow(result, "result");
    return true;
}


int main( int argc, char** argv)
{
    args = argparse::ArgumentParser("Voronoizer", "1.0");
    args.add_argument("image")
        .help("path to image to voronize")
        .required();

    args.add_argument("-o", "--options")
        .help("Comma separated list of mode-specific positional options\n"
        "\t\t(see --mode option description for more information).\n"
        "\t\tSupports truncated list as well as using default value by omitting the value\n"
        "\t\t(e.g. \"-o ,,,0.5\" will result in setting the third option to 0.5,\n"
        "\t\tall other options will keep their default values).")        
        .default_value<string>("");

    vector<string> modes = {"sobel", "kmeans-circles", "kmeans-lines", "sift-circles", "sift-lines"};
    stringstream ss;
    ss << std::fixed << std::setprecision(2) << 
        "voronizer mode: " << to_string(modes) << "\n"
        "\n"
        "\t\t   sobel:  options ["
                <<  "MEDIAN_PRE=" << SobelVoronizer::default_median_pre
                << ",EDGE_TRESHOLD=" << SobelVoronizer::default_edge_treshold
                << ",MEDIAN_POST=" << SobelVoronizer::default_median_post
                << ",CLUSTER_SIZE_TRESHOLD=" << SobelVoronizer::default_cluster_size_treshold
                << "]\n"      
        "\t\t      Generators created with Sobel edge detector:\n"
        "\t\t      1. preprocess image by median filter of size MEDIAN_PRE\n"
        "\t\t      2. find edges by Sobel detetor\n"
        "\t\t      3. apply edge tresholding with treshold value EDGE_TRESHOLD [0-255]\n"
        "\t\t      4. apply median filter of size MEDIAN_POST\n"
        "\t\t      5. remove any regions with less than CLUSTER_SIZE_TRESHOLD pixels.\n"
        "\n"
        "\t\t   kmeans-circles:  options ["
                <<  "MEDIAN_PRE=" << KMeansVoronizerCircles::default_median_pre
                << ",N_COLORS=" << KMeansVoronizerCircles::default_n_colors
                << ",CLUSTER_SIZE_TRESHOLD=" << KMeansVoronizerCircles::default_cluster_size_treshold
                << ",RADIUS=" << KMeansVoronizerCircles::default_radius
                << ",THICKNESS=" << KMeansVoronizerCircles::default_thickness
                << "]\n"
        "\t\t      Generators created by KMeans color clustering - generators are circles\n"
        "\t\t      centered at centers of mass of regions found by KMeans:\n"            
        "\t\t      1. preprocess image by median filter of size MEDIAN_PRE\n"            
        "\t\t      2. quantize the image to N_COLORS by KMeans\n"                        
        "\t\t      3. split the clusters into regions of spatially close pixels with the\n"
        "\t\t         same color and remove any with less than CLUSTER_SIZE_TRESHOLD pixels\n"
        "\t\t      4. use centers of mass of the regions as centers of generator circles\n"
        "\t\t         with given RADIUS (0 for points instead of circles) and THICKNESS\n"
        "\t\t         (-1 to fill the circles)\n"
        "\n"
        "\t\t   kmeans-lines:  options ["
                <<  "MEDIAN_PRE=" << KMeansVoronizerLines::default_median_pre
                << ",N_COLORS=" << KMeansVoronizerLines::default_n_colors
                << ",CLUSTER_SIZE_TRESHOLD=" << KMeansVoronizerLines::default_cluster_size_treshold
                << ",RANDOM_ITER=" << KMeansVoronizerLines::default_n_iter
                << "]\n"
        "\t\t      Generators created by KMeans color clustering - generators are lines\n"
        "\t\t      where endpoints are centers of  mass of regions found by KMeans:\n"
        "\t\t      1. - 3. same as \"kmeans-circles\"\n"
        "\t\t      4. use centers of mass of the regions as endpoints of line segment generators\n"
        "\t\t         - for each point try RANDOM_ITER other (unused) points and select\n"
        "\t\t         the closest one to create new line segment\n"
        "\n"
        "\t\t   sift-circles:  options ["
                    <<  "KEYPOINT_SIZE_TRESHOLD=" << SIFTVoronizerCircles::default_keypoint_size_treshold
                    << ",RADIUS=" << SIFTVoronizerCircles::default_radius
                    << ",THICKNESS=" << SIFTVoronizerCircles::default_thickness
                    << ",RADIUS_MULTIPLIER=" << SIFTVoronizerCircles::default_radius_multiplier
                    << "]\n"
        "\t\t      Generators are points/circles created from SIFT keypoints:\n"
        "\t\t      1. Detect SIFT keypoints of the image\n"
        "\t\t      2. Filter out keypoints of size less than KEYPOINT_SIZE_TRESHOLD\n"
        "\t\t      3. Create generator circles at selected keypoints with given RADIUS\n"
        "\t\t         (0 for points instead of circles) and THICKNESS (-1 to fill the circles).\n"
        "\t\t         You can also use RADIUS = -1 for radius given by size of SIFT keypoints,\n"
        "\t\t         which can be adjusted by RADIUS_MULTIPLIER.\n"
        "\t\t         (RADIUS_MULTIPLIER is ignored in case of RADIUS >= 0)\n"
        "\n"
        "\t\t   sift-lines:  options ["
                    <<  "KEYPOINT_SIZE_TRESHOLD=" << SIFTVoronizerLines::default_keypoint_size_treshold
                    << ",RANDOM_ITER=" << SIFTVoronizerLines::default_n_iter
                    << "]\n"
        "\t\t      Generators created with SIFT keypoints - generators are lines where\n"
        "\t\t      endpoints are SIFT keypoints:\n"
        "\t\t      1. - 2. same as \"sift-circles\"\n"
        "\t\t      3. use selected SIFT keypoints as endpoints of line segment generators\n"
        "\t\t         - for each point try RANDOM_ITER other (unused) points and select\n"
        "\t\t         the closest one to create new line segment\n"
        "\t\t";
    args.add_argument("-m", "--mode")
        .help(ss.str())
        .default_value<string>("sobel");

    args.add_argument("-c", "--colormap")
        .help("OpenCV colormap name to use instead of original image as color template:\n"
        "\t\t{autumn, bone, jet, winter, rainbow, ocean, summer, spring, cool, hsv, pink, hot,\n"
        "\t\tparula, magma, inferno, plasma, viridis, cividis, twilight, twilight_shifted, turbo}")
        .default_value<string>("");

    args.add_argument("-f", "--file")
        .help("Write output to file instead of displaying it in a window")
        .default_value<string>("");

    args.add_argument("-r", "--random")
        .help("Has effect only if using colormap: shuffle colors of areas randomly,\n"
        "\t\totherwise color will depend on x and y coordinate of area")
        .default_value(false)
        .implicit_value(true);

    args.add_argument("-s", "--smooth")
        .help("Strength of edges smoothing")
        .default_value<uint>(3)
        .scan<'u', uint>();


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
    string img_path = args.get("image");
    string options = args.get("-o");
    string output_file = args.get("-f");

    if (!fs::exists(img_path) || fs::is_directory(img_path))
        help_exit("File does not exist: " + img_path);
    if (std::find(modes.begin(), modes.end(), mode) == modes.end())
        help_exit("Unrecognized mode: " + mode);
    if (cmap && !strToColormap(args.get<string>("-c"), cmap_type))
        help_exit("Unrecognized colormap: " + args.get<string>("-c"));

    run(img_path, mode, options, cmap, cmap_type, random, smooth, output_file);

    return 0;
}