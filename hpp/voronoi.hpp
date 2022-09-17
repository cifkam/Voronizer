
#ifndef VORONOI_HPP
#define VORONOI_HPP

#include <opencv2/core.hpp>
#include "growing.hpp"

class Voronoi : public Growing
{
public:
    Voronoi();

private:
    virtual void init_funct(std::set<Cell*>& opened, CellMat& cell_mat, cv::Mat& data) override;

};


#endif /* VORONOI_HPP */