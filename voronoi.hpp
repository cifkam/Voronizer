
#ifndef VORONOI_HPP
#define VORONOI_HPP

#include "growing.hpp"
#include <opencv2/core.hpp>

class Voronoi : public Growing
{
public:
    Voronoi();
private:
    void init_funct(std::set<Cell*>& opened, std::vector<std::vector<Cell>>& cell_mat, cv::Mat& data) override;
};


#endif /* VORONOI_HPP */