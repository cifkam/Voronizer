#include "voronizer.hpp"
#include "utils.hpp"
#include "separator.hpp"
#include "voronoi.hpp"
#include <opencv4/opencv2/features2d.hpp>
using namespace std;

AbstractVoronizer::AbstractVoronizer()
{
    unset_colormap();
}

void AbstractVoronizer::unset_colormap()
{
    colorize_funct = [&](const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups)
    { return colorize_by_template(input, voronoi_groups); };
}


void AbstractVoronizer::set_colormap(cv::ColormapTypes cmap_type, bool random)
{
    colorize_funct = [cmap_type,random](const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups)
    { return colorize_by_cmap(voronoi_output, cmap_type, true, random); };
}


vector<string> AbstractVoronizer::parse_args(const std::string& args)
{
    vector<string> vec;
        if (args.size() == 0)
        return vec;

    int i = 0;
    for (int j = 0; j < args.size(); ++j)
    {
        if (args[j] == ',')
        {
            vec.push_back(args.substr(i, j-i));
            i = j+1;
        }
    }
    
    if (i != args.size() || args[args.size()-1] == ',')
        vec.push_back(args.substr(i, args.size() - i));

    return vec;    
}



/* --- sobel --- */
SobelVoronizer::SobelVoronizer(size_t median_pre, size_t edge_treshold, size_t median_post, size_t cluster_size_treshold)
{
    this->median_pre = median_pre;
    this->edge_treshold = edge_treshold;
    this->median_post = median_post;
    this->cluster_size_treshold = cluster_size_treshold;
}

std::unique_ptr<SobelVoronizer> SobelVoronizer::create(const std::string& args)
{
    auto vec = parse_args(args);
    
    size_t median_pre = default_median_pre;
    size_t edge_treshold = default_edge_treshold;
    size_t median_post = default_median_post;
    size_t cluster_size_treshold = default_cluster_size_treshold;
    
    if (vec.size() >= 1 && vec[0].size() != 0 && !tryParse<size_t>(vec[0], median_pre))
        return nullptr;
    if (vec.size() >= 2 && vec[1].size() != 0 && !tryParse<size_t>(vec[1], edge_treshold))
        return nullptr;
    if (vec.size() >= 3 && vec[2].size() != 0 && !tryParse<size_t>(vec[2], median_post))
        return nullptr;
    if (vec.size() >= 4 && vec[3].size() != 0 && !tryParse<size_t>(vec[3], cluster_size_treshold))
        return nullptr;
    if (vec.size() >= 5)
        return nullptr;
    
    return make_unique<SobelVoronizer>(median_pre, edge_treshold, median_post, cluster_size_treshold);
}

cv::Mat SobelVoronizer::run(cv::Mat& input)
{
    cv::Mat data;
    cv::cvtColor(input, data, cv::COLOR_RGB2GRAY);
    cv::medianBlur(data, data, median_pre);
    cv::Sobel(data, data, CV_8U, 1, 1, 5);
    cv::threshold(data, data, edge_treshold, 255, cv::THRESH_BINARY);
    cv::medianBlur(data, data, median_post);

    Separator separator(cluster_size_treshold, 0);
    data.convertTo(data, CV_16S);
    separator.compute(data, data);
    
    Voronoi voronoi;
    voronoi.groups = separator.clear_groups();
    voronoi.compute(data, data, false);

    auto groups = voronoi.clear_groups();
    return colorize_funct(input, data, groups);
}



/* --- kmeans --- */
KMeansVoronizer::KMeansVoronizer(size_t median_pre, size_t n_colors, size_t cluster_size_treshold)
{
    this->median_pre = median_pre;
    this->n_colors = n_colors;
    this->cluster_size_treshold = cluster_size_treshold;
}

std::unique_ptr<KMeansVoronizer> KMeansVoronizer::create(const std::string& args)
{
    auto vec = parse_args(args);

    size_t median_pre = default_median_pre;
    size_t n_colors = default_n_colors;
    size_t cluster_size_treshold = default_cluster_size_treshold;
    
    if (vec.size() >= 1 && vec[0].size() != 0 && !tryParse<size_t>(vec[0], median_pre))
        return nullptr;
    if (vec.size() >= 2 && vec[1].size() != 0 && !tryParse<size_t>(vec[1], n_colors))
        return nullptr;
    if (vec.size() >= 3 && vec[2].size() != 0 && !tryParse<size_t>(vec[2], cluster_size_treshold))
        return nullptr;
    if (vec.size() >= 4)
        return nullptr;
    
    return make_unique<KMeansVoronizer>(median_pre, n_colors, cluster_size_treshold);
}

cv::Mat KMeansVoronizer::run(cv::Mat& input)
{
    cv::Mat data;
    cv::medianBlur(input, data, median_pre); // apply median filter to speed-up the process and remove small regions
    kmeans_color(data, data, n_colors);
    cv::cvtColor(data, data, cv::COLOR_RGB2GRAY);
    //random_LUT(data, data);

    Separator separator(cluster_size_treshold, -1);
    data.convertTo(data, CV_16S);
    separator.compute(data, data);
    auto groups = separator.clear_groups();
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



/* --- sift --- */
SIFTVoronizer::SIFTVoronizer(size_t keypoint_size_treshold, int radius, int thickness, float radius_multiplier)
{
    this->keypoint_size_treshold = keypoint_size_treshold;
    this->radius = radius;
    this->thickness = thickness;
    this->radius_multiplier = radius_multiplier;
}


std::unique_ptr<SIFTVoronizer> SIFTVoronizer::create(const std::string& args)
{
    auto vec = parse_args(args);

    size_t keypoint_size_treshold = default_keypoint_size_treshold;
    int radius = default_radius; 
    int thickness = default_thickness;
    float radius_multiplier = default_radius_multiplier;
    
    if (vec.size() >= 1 && vec[0].size() != 0 && !tryParse<size_t>(vec[0], keypoint_size_treshold))
        return nullptr;
    if (vec.size() >= 2 && vec[1].size() != 0 && !tryParse<int>(vec[1], radius))
        return nullptr;
    if (vec.size() >= 3 && vec[2].size() != 0 && !tryParse<int>(vec[2], thickness))
        return nullptr;
    if (vec.size() >= 4 && vec[3].size() != 0 && !tryParse<float>(vec[3], radius_multiplier))
        return nullptr;
    if (vec.size() >= 5)
        return nullptr;
    
    return make_unique<SIFTVoronizer>(keypoint_size_treshold, radius, thickness, radius_multiplier);
}

cv::Mat SIFTVoronizer::run(cv::Mat& input)
{
    auto detector = cv::SIFT::create(0, 3, 0.03, 10, 1.6);
    std::vector<cv::KeyPoint> keypoints;
    detector->detect(input, keypoints);

    keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(),
        [&](cv::KeyPoint x){return x.size < keypoint_size_treshold;}),
    keypoints.end());

    cv::Mat data = cv::Mat::zeros(input.size(), CV_16S);
    int16_t n = 1;
    if (radius < 0)
        for (auto& k : keypoints)
            cv::circle(data, k.pt, k.size*radius_multiplier, n++, thickness);
    else
        for (auto& k : keypoints)
            cv::circle(data, k.pt, radius, n++, thickness);

    Voronoi voronoi;
    voronoi.compute(data, data);

    auto groups = voronoi.clear_groups();
    return colorize_funct(input, data, groups);
}