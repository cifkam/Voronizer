
#ifndef CLUSTERING_HPP
#define CLUSTERING_HPP

#include "growing.hpp"
#include <opencv2/core.hpp>

/*
Separate image into regions of spatially closed pixels with the same color
(i.e. pixels that have the same color but are not next to each other will be in different groups)
*/
class Separator : public Growing
{
public:
    size_t treshold;
    int bg_value;

    Separator(size_t treshold = 50, int bg_value = 0);
    virtual size_t compute(cv::Mat& data, cv::Mat& output,
        std::unique_ptr<Groups>&& groups = nullptr,
        std::unique_ptr<PixelMat>&& pixel_mat = nullptr) override;

protected:
    // Helper class to remove regions that were removed by Separator because of region size tresholding
    class AfterTresholdGrowing : public Growing
    {
    public:
        int rows;
        int cols;
        AfterTresholdGrowing(int rows, int cols);
        virtual size_t compute(cv::Mat& data, cv::Mat& output,
            std::unique_ptr<Groups>&& groups = nullptr,
            std::unique_ptr<PixelMat>&& pixel_mat = nullptr) override;
        
    private:
        void Remap(cv::Mat& data);
        virtual void init_funct(std::set<Pixel*>& opened, cv::Mat& data, bool create_new_pixelmat) override;

    };

    int n;

    int last_row;
    int last_col;

    const cv::Mat* input_data;

    virtual void init_funct(std::set<Pixel*>& opened, cv::Mat& data, bool create_new_pixelmat) override;
    virtual void post_funct(std::vector<Pixel*>& processed, cv::Mat& data) override;
    virtual void add_to_group(Pixel* pixel, int cls) override;
    virtual bool grow_condition(const cv::Mat& data, const cv::Mat& output, Pixel* pixel, Pixel* neighbor) override;

};






#endif /* CLUSTERING_HPP */