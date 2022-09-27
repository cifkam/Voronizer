#ifndef VORONIZER_HPP
#define VORONIZER_HPP

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "growing.hpp"

typedef std::function<cv::Mat(const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups)> color_funct_t;

// Abstract class for Voronizing an image - provides an interface for running the Voronizer and setting colorization type
class AbstractVoronizer
{
public:
    // Main function to run the Voronizer
    virtual cv::Mat run(cv::Mat& input) = 0;
    
    color_funct_t colorize_funct;
    // Set the colorization function to cmap by OpenCV colormap
    void set_colormap(cv::ColormapTypes cmap_type, bool random);
    // Sets the colorization function to use image template - each region will be colored by average color of underlying pixels of input image 
    void unset_colormap();
    virtual ~AbstractVoronizer() = default;

protected:
    AbstractVoronizer();
    // Colorization by OpenCV cmap
    cv::Mat colorize_funct_cmap(const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups);
    // Colorization by average color of pixel in each group
    static cv::Mat colorize_funct_template(const cv::Mat& input, const cv::Mat& voronoi_output, const Groups& voronoi_groups);
    // Splits string args separated by comma into vector
    static std::vector<std::string> parse_args(const std::string& args);
};


/*
Voronizer class where generators are created with Sobel edge detector:
1. preprocess image by median filter of size MEDIAN_PRE
2. find edges by Sobel detetor
3. apply edge tresholding with treshold value EDGE_TRESHOLD [0-255]
4. apply median filter of size MEDIAN_POST
5. remove any regions with less than CLUSTER_SIZE_TRESHOLD pixels´.
*/
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
        size_t cluster_size_treshold = default_cluster_size_treshold
    );

    // Create SobelVoronizer instance from string of comma-separated constructor arguments
    static std::unique_ptr<SobelVoronizer> create(const std::string& args);
    virtual cv::Mat run(cv::Mat& input) override;

protected:

    size_t median_pre;
    size_t edge_treshold;
    size_t median_post;
    size_t cluster_size_treshold;
};

/*
Voronizer class where generators are created by KMeans color clustering - generators are created from centers of mass of regions found by KMeans:
1. preprocess image by median filter of size MEDIAN_PRE
2. quantize the image to N_COLORS by KMeans
3. split the clusters into regions of spatially close pixels with the same color and remove any with less than CLUSTER_SIZE_TRESHOLD pixels
(4. use centers of mass of the regions to create generators via abstract "drawGenerators" member function) 
*/
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

    // Draw an image of generators (given the computed groups) 
    virtual cv::Mat drawGenerators(const Groups& groups, cv::Size image_size) = 0;
};

/*
Voronizer class where generators are created by KMeans color clustering - generators are circles centered at centers of mass of regions found by KMeans:
1. preprocess image by median filter of size MEDIAN_PRE
2. quantize the image to N_COLORS by KMeans
3. split the clusters into regions of spatially close pixels with the same color and remove any with less than CLUSTER_SIZE_TRESHOLD pixels
4. use centers of mass of the regions as centers of generator circles with given RADIUS (0 for points instead of circles) and THICKNESS (-1 to fill the circles)
*/
class KMeansVoronizerCircles : public AbstractKMeansVoronizer
{
public:
    static constexpr size_t default_radius = 0;
    static constexpr int default_thickness = -1;

    /*
    Use "thickness=-1" to fill the circles.
    */
    KMeansVoronizerCircles(
        size_t median_pre = default_median_pre,
        size_t n_colors = default_n_colors,
        size_t cluster_size_treshold = default_cluster_size_treshold,
        size_t radius = default_radius,
        int thickness = default_thickness);

    /*
    Create KMeansVoronizerCircles instance from string of comma-separated constructor arguments
    */
    static std::unique_ptr<KMeansVoronizerCircles> create(const std::string& args);

protected:
    size_t radius;
    int thickness;

    // Draw an image of generators (given the computed groups)
    virtual cv::Mat drawGenerators(const Groups& groups, cv::Size image_size) override;

};

/*
Voronizer class where generators are created by KMeans color clustering - generators are lines where endpoints are centers of  mass of regions found by KMeans:
1. preprocess image by median filter of size MEDIAN_PRE
2. quantize the image to N_COLORS by KMeans
3. split the clusters into regions of spatially close pixels with the same color and remove any with less than CLUSTER_SIZE_TRESHOLD pixels
4. use centers of mass of the regions as endpoints of line segment generators - for each point try RANDOM_ITER other (unused) points and select the closest one to create new line segment
*/
class KMeansVoronizerLines : public AbstractKMeansVoronizer
{
public:
    static constexpr size_t default_n_iter = 25;

    // Create KMeansVoronizerLines instance from string of comma-separated constructor arguments
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

    // Draw an image of generators (given the computed groups)
    virtual cv::Mat drawGenerators(const Groups& groups, cv::Size image_size) override;

};


/*
Abstract Voronizer class where generators are created from SIFT keypoints:
1. Detect SIFT keypoints of the image
2. Filter out keypoints of size less than KEYPOINT_SIZE_TRESHOLD
(3. use SIFT keypoints to create generators via abstract "drawGenerators" member function) 
*/
class AbstractSIFTVoronizer : public AbstractVoronizer
{
public:
    static constexpr size_t default_keypoint_size_treshold = 5;

    AbstractSIFTVoronizer(size_t keypoint_size_treshold = default_keypoint_size_treshold);
    virtual cv::Mat run(cv::Mat& input) override;

protected:
    size_t keypoint_size_treshold;

    // Draw an image of generators (given the computed groups)
    virtual cv::Mat drawGenerators(std::vector<cv::KeyPoint> keypoints, cv::Size image_size) = 0;
};


/*
Generators are points/circles created from SIFT keypoints:
1. Detect SIFT keypoints of the image
2. Filter out keypoints of size less than KEYPOINT_SIZE_TRESHOLD
3. Create generator circles at selected keypoints with given RADIUS (0 for points instead of circles) and THICKNESS (-1 to fill the circles).
    You can also use RADIUS = -1 for radius given by size of SIFT keypoints, which can be adjusted by RADIUS_MULTIPLIER
    (RADIUS_MULTIPLIER is ignored in case of RADIUS >= 0)
*/
class SIFTVoronizerCircles : public AbstractSIFTVoronizer
{
public:
    static constexpr int default_radius = -1;
    static constexpr int default_thickness = -1;
    static constexpr float default_radius_multiplier = 1.0f;
    
    /*
    Use "radius=-1" to use size of keypoints as circles radii.
    Use "thickness=-1" to fill the circle.
    The radius_multiplier applies only if "radius==-1", i.e. size of keypoints are used as circles radii.
    */
    SIFTVoronizerCircles(
        size_t keypoint_size_treshold = default_keypoint_size_treshold,
        int radius = default_radius,
        int thickness = default_thickness,
        float radius_multiplier = default_radius_multiplier
    );

    // Create SIFTVoronizerCircles instance from string of comma-separated constructor arguments
    static std::unique_ptr<SIFTVoronizerCircles> create(const std::string& args);

protected:
    int radius;
    int thickness;
    float radius_multiplier;

    // Draw an image of generators (given the computed SIFT keypoints)
    virtual cv::Mat drawGenerators(std::vector<cv::KeyPoint> keypoints, cv::Size image_size) override;
};

/*
Voronizer class where generators are created with SIFT keypoints - generators are lines where endpoints are SIFT keypoints:
1. Detect SIFT keypoints of the image
2. Filter out keypoints of size less than KEYPOINT_SIZE_TRESHOLD
3. use selected SIFT keypoints as endpoints of line segment generators - for each point try RANDOM_ITER other (unused) points
    and select the closest one to create new line segment
*/
class SIFTVoronizerLines : public AbstractSIFTVoronizer
{
public:
    static constexpr size_t default_n_iter = 25;
    SIFTVoronizerLines(
        size_t keypoint_size_treshold = default_keypoint_size_treshold,
        size_t n_iter = default_n_iter
    );
    // Create SIFTVoronizerLines instance from string of comma-separated constructor arguments
    static std::unique_ptr<SIFTVoronizerLines> create(const std::string& args);

protected:
    size_t n_iter;
    // Draw an image of generators (given the computed SIFT keypoints)
    virtual cv::Mat drawGenerators(std::vector<cv::KeyPoint> keypoints, cv::Size image_size) override;
};

#endif /* VORONIZER_HPP */