
#ifndef VORONOI_HPP
#define VORONOI_HPP

#include <opencv2/core.hpp>
#include "growing.hpp"

// Growing class that creates Voronoi cells from raster generators - starts from specified generators and grows while alternating 4- and 8-neighborhood
class Voronoi : public Growing
{
public:
    Voronoi();

private:
    virtual void init_funct(std::set<Cell*>& opened, cv::Mat& data, bool create_new_cellmat) override;

};


#endif /* VORONOI_HPP */