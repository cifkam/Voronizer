#include "voronizer.hpp"

#include <limits>
#include <random>
#include <iostream>

#include "utils.hpp"
#include "separator.hpp"
#include "voronoi.hpp"
#include <opencv2/features2d.hpp>
using namespace std;

AbstractVoronizer::AbstractVoronizer()
{
    unset_colormap();
}

void AbstractVoronizer::unset_colormap()
{
    colorize_funct = [&](const cv::Mat& input, const cv::Mat& voronoi_output, const Groups* voronoi_groups)
    { return colorizeByTemplate(input, voronoi_groups); };
}


void AbstractVoronizer::set_colormap(cv::ColormapTypes cmap_type, bool random)
{
    colorize_funct = [cmap_type,random](const cv::Mat& input, const cv::Mat& voronoi_output, const Groups* voronoi_groups)
    { return colorizeByCmap(voronoi_output, cmap_type, true, random); };
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
    
    if (median_pre != 0 && median_pre%2 == 0)
        cerr << "Warning: MEDIAN_PRE must be either zero or an odd number. Changing from "
             << median_pre << " to " << ++median_pre << "." << endl;

    if (median_post != 0 && median_post%2 == 0)
        cerr << "Warning: MEDIAN_POST must be either zero or an odd number. Changing from "
             << median_post << " to " << ++median_post << "." << endl;

    return make_unique<SobelVoronizer>(median_pre, edge_treshold, median_post, cluster_size_treshold);
}

cv::Mat SobelVoronizer::run(cv::Mat& input)
{
    cv::Mat data;
    cv::cvtColor(input, data, cv::COLOR_RGB2GRAY);
    if (median_pre > 0)
        cv::medianBlur(data, data, (int)median_pre);
    cv::Sobel(data, data, CV_8U, 1, 1, 5);
    cv::threshold(data, data, (double)edge_treshold, 255, cv::THRESH_BINARY);
    if (median_post > 0)
        cv::medianBlur(data, data, (int)median_post);

    Separator separator(cluster_size_treshold, 0);
    data.convertTo(data, CV_16S);
    separator.compute(data, data);
    
    Voronoi voronoi;
    voronoi.compute(data, data, separator.clear_groups(), separator.clear_pixelmat());

    return colorize_funct(input, data, &*voronoi.groups);
}



/* --- kmeans --- */
AbstractKMeansVoronizer::AbstractKMeansVoronizer(size_t median_pre, size_t n_colors, size_t cluster_size_treshold)
{
    this->median_pre = median_pre;
    this->n_colors = n_colors;
    this->cluster_size_treshold = cluster_size_treshold;
}
KMeansVoronizerCircles::KMeansVoronizerCircles(size_t median_pre, size_t n_colors, size_t cluster_size_treshold, size_t radius, int thickness)
: AbstractKMeansVoronizer(median_pre, n_colors, cluster_size_treshold)
{
    this->radius = radius;
    this->thickness = thickness;
}
KMeansVoronizerLines::KMeansVoronizerLines(size_t median_pre, size_t n_colors, size_t cluster_size_treshold, size_t n_iter)
: AbstractKMeansVoronizer(median_pre, n_colors, cluster_size_treshold) 
{
    this->n_iter = n_iter;
}

std::unique_ptr<KMeansVoronizerCircles> KMeansVoronizerCircles::create(const std::string& args)
{
    auto vec = parse_args(args);

    size_t median_pre = default_median_pre;
    size_t n_colors = default_n_colors;
    size_t cluster_size_treshold = default_cluster_size_treshold;
    size_t radius = default_radius;
    int thickness = default_thickness;
    
    if (vec.size() >= 1 && vec[0].size() != 0 && !tryParse<size_t>(vec[0], median_pre))
        return nullptr;
    if (vec.size() >= 2 && vec[1].size() != 0 && !tryParse<size_t>(vec[1], n_colors))
        return nullptr;
    if (vec.size() >= 3 && vec[2].size() != 0 && !tryParse<size_t>(vec[2], cluster_size_treshold))
        return nullptr;
    if (vec.size() >= 4 && vec[3].size() != 0 && !tryParse<size_t>(vec[3], radius))
        return nullptr;
    if (vec.size() >= 5 && vec[4].size() != 0 && !tryParse<int>(vec[4], thickness))
        return nullptr;
    if (vec.size() >= 6)
        return nullptr; 

    if (n_colors == 0)
        return nullptr;
    
    if (median_pre != 0 && median_pre%2 == 0)
        cerr << "Warning: MEDIAN_PRE must be either zero or an odd number. Changing from "
             << median_pre << " to " << ++median_pre << "." << endl;

    return make_unique<KMeansVoronizerCircles>(median_pre, n_colors, cluster_size_treshold, radius, thickness);
}

std::unique_ptr<KMeansVoronizerLines> KMeansVoronizerLines::create(const std::string& args)
{
    auto vec = parse_args(args);

    size_t median_pre = default_median_pre;
    size_t n_colors = default_n_colors;
    size_t cluster_size_treshold = default_cluster_size_treshold;
    size_t n_iter = default_n_iter;
    
    if (vec.size() >= 1 && vec[0].size() != 0 && !tryParse<size_t>(vec[0], median_pre))
        return nullptr;
    if (vec.size() >= 2 && vec[1].size() != 0 && !tryParse<size_t>(vec[1], n_colors))
        return nullptr;
    if (vec.size() >= 3 && vec[2].size() != 0 && !tryParse<size_t>(vec[2], cluster_size_treshold))
        return nullptr;
    if (vec.size() >= 4 && vec[3].size() != 0 && !tryParse<size_t>(vec[3], n_iter))
        return nullptr;
    if (vec.size() >= 5)
        return nullptr;

    if (median_pre != 0 && median_pre%2 == 0)
        cerr << "Warning: MEDIAN_PRE must be either zero or an odd number. Changing from "
             << median_pre << " to " << ++median_pre << "." << endl;

    if (n_colors == 0 || n_iter == 0)
        return nullptr;
    
    return make_unique<KMeansVoronizerLines>(median_pre, n_colors, cluster_size_treshold, n_iter);

}

cv::Mat AbstractKMeansVoronizer::run(cv::Mat& input)
{
    cv::Mat data;
    if (median_pre > 0)
        cv::medianBlur(input, data, (int)median_pre); // apply median filter to speed-up the process and remove small regions
    else
        input.copyTo(data);
    kmeansColor(data, data, (int)n_colors);
    cv::cvtColor(data, data, cv::COLOR_RGB2GRAY);
    Separator separator(cluster_size_treshold, -1);
    data.convertTo(data, CV_16S);
    separator.compute(data, data);
    
    auto groups = separator.clear_groups();
    groups->erase(0);
    
    cv::Mat im = drawGenerators(&*groups, input.size());

    //Show generators
    /*cv::Mat m(im);
    for (int r = 0; r < m.rows; ++r)
        for (int c = 0; c < m.cols; ++c)
            m.at<int16_t>(r,c) %= 256;
    m.convertTo(m, CV_8U);
    imshow(m, "m");
    */

    Voronoi voronoi;
    voronoi.compute(im, im, nullptr, separator.clear_pixelmat());

    groups = voronoi.clear_groups();
    return colorize_funct(input, im, &*groups);
}

cv::Mat KMeansVoronizerCircles::drawGenerators(const Groups* groups, cv::Size image_size)
{
    cv::Mat im = cv::Mat::zeros(image_size, CV_16S);
    for (auto& group : *groups)
    {
        size_t row = 0;
        size_t col = 0;
        for (auto& pixel : group.second)
        {
            row += pixel->row;
            col += pixel->col;
        }
        row /= group.second.size();
        col /= group.second.size();
        cv::circle(im, cv::Point2d((float)col,(float)row), (int)(radius), group.first, thickness);
    }
    return im;
}


cv::Mat KMeansVoronizerLines::drawGenerators(const Groups* groups, cv::Size image_size)
{
    std::vector<cv::Point2f> points;
    points.reserve(groups->size());
    for (auto& group : *groups)
    {
        int row = 0;
        int col = 0;
        for (auto& pixel : group.second)
        {
            row += pixel->row;
            col += pixel->col;
        }
        row = (int)(row/group.second.size());
        col = (int)(col/group.second.size());
        points.push_back(cv::Point2f((float)col,(float)row));
    }

    return linesFromClosestPointsRandom(points, image_size, n_iter);
}

/* --- sift --- */
cv::Mat AbstractSIFTVoronizer::run(cv::Mat& input)
{
    auto detector = cv::SIFT::create(0, 3, 0.03, 10, 1.6);
    std::vector<cv::KeyPoint> keypoints;
    detector->detect(input, keypoints);

    keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(),
        [&](cv::KeyPoint x){return x.size < keypoint_size_treshold;}),
    keypoints.end());

    cv::Mat im = drawGenerators(keypoints, input.size());

    //Show generators
    /* cv::Mat m(im);
    for (int r = 0; r < m.rows; ++r)
        for (int c = 0; c < m.cols; ++c)
            m.at<int16_t>(r,c) %= 256;
    m.convertTo(m, CV_8U);
    imshow(m, "m"); */

    Voronoi voronoi;
    voronoi.compute(im, im);

    auto groups = voronoi.clear_groups();
    return colorize_funct(input, im, &*groups);
}

AbstractSIFTVoronizer::AbstractSIFTVoronizer(size_t keypoint_size_treshold)
{
    this->keypoint_size_treshold = keypoint_size_treshold;
}


SIFTVoronizerCircles::SIFTVoronizerCircles(size_t keypoint_size_treshold, int radius, int thickness, float radius_multiplier)
: AbstractSIFTVoronizer(keypoint_size_treshold)
{
    
    this->radius = radius;
    this->thickness = thickness;
    this->radius_multiplier = radius_multiplier;
}


std::unique_ptr<SIFTVoronizerCircles> SIFTVoronizerCircles::create(const std::string& args)
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

    if (radius < 0 && radius != -1)
        return nullptr;
    if (radius_multiplier < 0)
        return nullptr;
    
    return make_unique<SIFTVoronizerCircles>(keypoint_size_treshold, radius, thickness, radius_multiplier);
}


cv::Mat SIFTVoronizerCircles::drawGenerators(std::vector<cv::KeyPoint> keypoints, cv::Size image_size)
{
    cv::Mat data = cv::Mat::zeros(image_size, CV_16S);
    int16_t n = 1;
    if (radius < 0)
        for (auto& k : keypoints)
            cv::circle(data, k.pt, (int)(k.size*radius_multiplier), n++, thickness);
    else
        for (auto& k : keypoints)
            cv::circle(data, k.pt, radius, n++, thickness);
    return data;
}

SIFTVoronizerLines::SIFTVoronizerLines(
    size_t keypoint_size_treshold,
    size_t n_iter)
    : AbstractSIFTVoronizer(keypoint_size_treshold)
{
    this->n_iter = n_iter;
}

std::unique_ptr<SIFTVoronizerLines> SIFTVoronizerLines::create(const std::string& args)
{
    auto vec = parse_args(args);

    size_t keypoint_size_treshold = default_keypoint_size_treshold;
    size_t n_iter = default_n_iter; 

    if (vec.size() >= 1 && vec[0].size() != 0 && !tryParse<size_t>(vec[0], keypoint_size_treshold))
        return nullptr;
    if (vec.size() >= 2 && vec[1].size() != 0 && !tryParse<size_t>(vec[1], n_iter))
        return nullptr;

    if (n_iter == 0)
        return nullptr;
    
    return make_unique<SIFTVoronizerLines>(keypoint_size_treshold, n_iter);
}

cv::Mat SIFTVoronizerLines::drawGenerators(std::vector<cv::KeyPoint> keypoints, cv::Size image_size)
{
    std::vector<cv::Point2f> pts;
    pts.reserve(keypoints.size());
    for (auto& x : keypoints)
        pts.push_back(move(x.pt));
    return linesFromClosestPointsRandom(pts, image_size, n_iter);   
}