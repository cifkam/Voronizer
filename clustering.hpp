
#ifndef CLUSTERING_HPP
#define CLUSTERING_HPP

#include "growing.hpp"
#include <opencv2/core.hpp>

class Clustering : public Growing
{
public:
    Clustering(size_t treshold = 50);
    size_t compute(cv::Mat& data) override;

protected:
    size_t n;
    size_t treshold;

    std::vector<std::vector<Cell>> cell_mat;
    int last_row;
    int last_col;

    void init_funct(std::set<Cell*>& opened, std::vector<std::vector<Cell>>& cell_mat, cv::Mat& data) override;
    void post_funct(std::vector<Cell*>& processed, cv::Mat& data) override;
};

#endif /* CLUSTERING_HPP */