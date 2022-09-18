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
    SobelVoronizer(
        size_t median_pre = default_median_pre,
        size_t edge_treshold = default_edge_treshold,
        size_t median_post = default_median_post,
        size_t cluster_size_treshold = default_cluster_size_treshold);
    static std::unique_ptr<SobelVoronizer> create(const std::string& args);
    virtual cv::Mat run(cv::Mat& input) override;

protected:
    static constexpr size_t default_median_pre = 5;
    static constexpr size_t default_edge_treshold = 30;
    static constexpr size_t default_median_post = 5;
    static constexpr size_t default_cluster_size_treshold = 15;

    size_t median_pre;
    size_t edge_treshold;
    size_t median_post;
    size_t cluster_size_treshold;
};

class KMeansVoronizer : public AbstractVoronizer
{
public:
    KMeansVoronizer(
        size_t median_pre = default_median_pre,
        size_t n_colors = default_n_colors,
        size_t cluster_size_treshold = default_cluster_size_treshold);

    static std::unique_ptr<KMeansVoronizer> create(const std::string& args);
    virtual cv::Mat run(cv::Mat& input) override;

protected:
    static constexpr size_t default_median_pre = 5;
    static constexpr size_t default_n_colors = 8;
    static constexpr size_t default_cluster_size_treshold = 15;

    size_t median_pre;
    size_t n_colors;
    size_t cluster_size_treshold;
};



class SIFTVoronizer : public AbstractVoronizer
{
public:
    SIFTVoronizer(
        size_t keypoint_size_treshold = default_keypoint_size_treshold,
        int radius = default_radius,                        // radius == 0 gives single point, radius == -1 uses size given by SIFT as circles radii  
        int thickness = default_thickness,                  // -1 to fill
        float radius_multiplier = default_radius_multiplier // applies only if radius == -1
    );
    static std::unique_ptr<SIFTVoronizer> create(const std::string& args);
    virtual cv::Mat run(cv::Mat& input) override;

protected:
    static constexpr size_t default_keypoint_size_treshold = 5;
    static constexpr int default_radius = -1;
    static constexpr int default_thickness = -1;
    static constexpr float default_radius_multiplier = 1.0f;

    size_t keypoint_size_treshold;
    int radius;
    int thickness;
    float radius_multiplier;
};

#endif /* VORONIZER_HPP */