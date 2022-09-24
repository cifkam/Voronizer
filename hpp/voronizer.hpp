#ifndef VORONIZER_HPP
#define VORONIZER_HPP

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include "growing.hpp"

typedef std::function<cv::Mat(const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups)> color_funct_t;

class AbstractVoronizer
{
public:
    virtual cv::Mat run(cv::Mat& input) = 0;
    
    cv::Mat colorize_funct_cmap(const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups);
    static cv::Mat colorize_funct_template(const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups);
    
    color_funct_t colorize_funct;
    void set_colormap(cv::ColormapTypes cmap_type, bool random);
    void unset_colormap();

protected:
    AbstractVoronizer();
    static std::vector<std::string> parse_args(const std::string& args);
};


class SobelVoronizer : public AbstractVoronizer
{
public:
    static constexpr size_t default_median_pre = 5;
    static constexpr size_t default_edge_treshold = 30;
    static constexpr size_t default_median_post = 5;
    static constexpr size_t default_cluster_size_treshold = 15;

    SobelVoronizer(
        size_t median_pre = default_median_pre,
        size_t edge_treshold = default_edge_treshold,
        size_t median_post = default_median_post,
        size_t cluster_size_treshold = default_cluster_size_treshold);
    static std::unique_ptr<SobelVoronizer> create(const std::string& args);
    virtual cv::Mat run(cv::Mat& input) override;

protected:

    size_t median_pre;
    size_t edge_treshold;
    size_t median_post;
    size_t cluster_size_treshold;
};


class AbstractKMeansVoronizer : public AbstractVoronizer
{
public:
    static constexpr size_t default_median_pre = 5;
    static constexpr size_t default_n_colors = 8;
    static constexpr size_t default_cluster_size_treshold = 15;

    AbstractKMeansVoronizer
    (
        size_t median_pre = default_median_pre,
        size_t n_colors = default_n_colors,
        size_t cluster_size_treshold = default_cluster_size_treshold
    );
    virtual cv::Mat run(cv::Mat& input);

protected:
    size_t median_pre;
    size_t n_colors;    
    size_t cluster_size_treshold;

    virtual cv::Mat drawGenerators(const Groups& groups, cv::Size image_size) = 0;
};

class KMeansVoronizerCircles : public AbstractKMeansVoronizer
{
public:
    static constexpr size_t default_radius = 0;
    static constexpr int default_thickness = -1;

    KMeansVoronizerCircles(
        size_t median_pre = default_median_pre,
        size_t n_colors = default_n_colors,
        size_t cluster_size_treshold = default_cluster_size_treshold,
        size_t radius = default_radius,
        int thickness = default_thickness);

    static std::unique_ptr<KMeansVoronizerCircles> create(const std::string& args);

protected:
    size_t radius;
    int thickness;

    virtual cv::Mat drawGenerators(const Groups& groups, cv::Size image_size) override;

};

class KMeansVoronizerLines : public AbstractKMeansVoronizer
{
public:
    static constexpr size_t default_n_iter = 25;
    static std::unique_ptr<KMeansVoronizerLines> create(const std::string& args);

    KMeansVoronizerLines
    (
        size_t median_pre = default_median_pre,
        size_t n_colors = default_n_colors,
        size_t cluster_size_treshold = default_cluster_size_treshold,
        size_t n_iter = default_n_iter
    );
protected:
    size_t n_iter;
    virtual cv::Mat drawGenerators(const Groups& groups, cv::Size image_size) override;

};



class AbstractSIFTVoronizer : public AbstractVoronizer
{
public:
    static constexpr size_t default_keypoint_size_treshold = 5;

    AbstractSIFTVoronizer(size_t keypoint_size_treshold = default_keypoint_size_treshold);
    virtual cv::Mat run(cv::Mat& input) override;

protected:
    size_t keypoint_size_treshold;

    virtual cv::Mat drawGenerators(std::vector<cv::KeyPoint> keypoints, cv::Size image_size) = 0;
};


class SIFTVoronizerCircles : public AbstractSIFTVoronizer
{
public:
    static constexpr int default_radius = -1;
    static constexpr int default_thickness = -1;
    static constexpr float default_radius_multiplier = 1.0f;
    
    SIFTVoronizerCircles(
        size_t keypoint_size_treshold = default_keypoint_size_treshold,
        int radius = default_radius,                           // radius == 0 gives single point, radius == -1 uses size given by SIFT as circles radii  
        int thickness = default_thickness,                     // -1 to fill whole circle
        float radius_multiplier = default_radius_multiplier    // applies only if radius == -1
    );
    static std::unique_ptr<SIFTVoronizerCircles> create(const std::string& args);

protected:
    int radius;
    int thickness;
    float radius_multiplier;

    virtual cv::Mat drawGenerators(std::vector<cv::KeyPoint> keypoints, cv::Size image_size) override;
};

class SIFTVoronizerLines : public AbstractSIFTVoronizer
{
public:
    static constexpr size_t default_n_iter = 25;
    SIFTVoronizerLines(
        size_t keypoint_size_treshold = default_keypoint_size_treshold,
        size_t n_iter = default_n_iter);
    static std::unique_ptr<SIFTVoronizerLines> create(const std::string& args);

protected:
    size_t n_iter;
    virtual cv::Mat drawGenerators(std::vector<cv::KeyPoint> keypoints, cv::Size image_size) override;
};

#endif /* VORONIZER_HPP */