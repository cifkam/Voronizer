
#ifndef CLUSTERING_HPP
#define CLUSTERING_HPP

#include "growing.hpp"
#include <opencv2/core.hpp>

class Separator : public Growing
{
public:
    Separator(size_t treshold = 50, int bg_value = 0);
    virtual size_t compute(cv::Mat& data, cv::Mat& output, bool clear_groups = true) override;
    size_t treshold;
    int bg_value;

protected:
    size_t n;

    int last_row;
    int last_col;

    const cv::Mat* input_data;

    virtual void init_funct(std::set<Cell*>& opened, CellMat& cell_mat, cv::Mat& data) override;
    virtual void post_funct(std::vector<Cell*>& processed, cv::Mat& data) override;
    virtual void add_to_group(Cell* cell, int cls) override;
    virtual bool grow_condition(const cv::Mat& data, const cv::Mat& output, Cell* cell, Cell* neighbor) override;

};

#endif /* CLUSTERING_HPP */