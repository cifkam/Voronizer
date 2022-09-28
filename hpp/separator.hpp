
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
    Separator(size_t treshold = 50, int bg_value = 0);
    virtual size_t compute(cv::Mat& data, cv::Mat& output, bool clear_groups = true) override;
    size_t treshold;
    int bg_value;

protected:
    // Helper class to remove regions that were removed by Separator because of region size tresholding
    class AfterTresholdGrowing : public Growing
    {
    public:
        int rows;
        int cols;
        AfterTresholdGrowing(int rows, int cols, Groups&& groups, std::unique_ptr<CellMat>&& cell_mat);
    private:
        virtual void init_funct(std::set<Cell*>& opened, cv::Mat& data) override;

    };

    size_t n;

    int last_row;
    int last_col;

    const cv::Mat* input_data;

    virtual void init_funct(std::set<Cell*>& opened, cv::Mat& data) override;
    virtual void post_funct(std::vector<Cell*>& processed, cv::Mat& data) override;
    virtual void add_to_group(Cell* cell, int cls) override;
    virtual bool grow_condition(const cv::Mat& data, const cv::Mat& output, Cell* cell, Cell* neighbor) override;

};






#endif /* CLUSTERING_HPP */