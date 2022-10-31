#ifndef GROWING_HPP
#define GROWING_HPP

#include <set>
#include <vector>
#include <opencv2/core.hpp>
#include <map>
#include <memory>

typedef int16_t int_t;
enum Neighborhood {n4, n8, alternating};
enum State {unseen, opened, closed};

struct Pixel
{
    Pixel(int row, int col, State state);
    friend bool operator<(const Pixel& lhs, const Pixel& rhs);

    int row;
    int col;
    State state;
};

typedef std::map<int, std::vector<Pixel*>> Groups;
typedef std::vector<std::vector<Pixel>> PixelMat;

/*
Abstract class for region growing - 
*/
class Growing
{
public:
    std::unique_ptr<Groups> groups;
    std::unique_ptr<PixelMat> pixel_mat;
    Neighborhood neighborhood;
    
    // Constructor that takes type of neighborhood (4/8-neighborhood or alternating)
    Growing(Neighborhood neighborhood);
    // Public wrapper function to run the growing. 
    virtual size_t compute(cv::Mat& input_data, cv::Mat& output_data, std::unique_ptr<Groups>&& groups = nullptr, std::unique_ptr<PixelMat>&& pixel_mat = nullptr);
    // Returns map of stored groups (assignment of values to individual pixels) and clears the variable.
    std::unique_ptr<Groups> clear_groups();
    // Returns matrix stored in pixel_mat and clears the variable.
    std::unique_ptr<PixelMat> clear_pixelmat();

protected:
    // Main function that runs the growing
    virtual size_t compute_inner(cv::Mat& input_output_data, bool create_new_pixelmat = true);
    // Initialization of pixel_mat and opened pixels
    virtual void init_funct(std::set<Pixel*>& opened, cv::Mat& data, bool create_new_pixelmat) = 0;
    // Possible postprocessing of pixels that were assigned a new value during the call of compute_inner
    virtual void post_funct(std::vector<Pixel*>& processed, cv::Mat& data);
    // Wrapper for assignment a new value to data
    virtual void assign(cv::Mat& data, Pixel* from, Pixel* to);
    // Wrapper for assignment a new value to data
    virtual void assign(cv::Mat& data, int value, Pixel* to);
    // Wrapper for adding pixel (pixel) to given group
    virtual void add_to_group(Pixel* pixel, int cls);
    // Checks if the the value from pixel should be assigned to its neighbor
    virtual bool grow_condition(const cv::Mat& data, const cv::Mat& output, Pixel* pixel, Pixel* neighbor);
    std::vector<Pixel*> get_neighbors(const Pixel* pixel, bool bool_4_8, std::size_t cols, std::size_t rows);
};


#endif /* GROWING_HPP */