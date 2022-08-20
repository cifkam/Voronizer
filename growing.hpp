#include <iostream>
#include <functional>
#include <set>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

enum Neighborhood {n4, n8, swap};
enum State {unseen, opened, closed};
  

struct Cell
{
    Cell() : row(0), col(0), state(State::unseen){}
    Cell(size_t row, size_t col, State state) : row(row), col(col), state(state) {}
    friend bool operator<(const Cell& lhs, const Cell& rhs) { return lhs.row < rhs.row || (lhs.row == rhs.row && lhs.col < rhs.col); }

    size_t row;
    size_t col;
    State state;
};


class Growing
{
public:
    
    Neighborhood neighborhood;
    Growing(Neighborhood neighborhood) : neighborhood(neighborhood) {};
    virtual size_t run(cv::Mat& data);

protected:
    virtual void init_funct(std::set<Cell*>& opened, std::vector<std::vector<Cell>>& cell_mat, cv::Mat& data) = 0;
    virtual void post_funct(std::vector<Cell*>& processed, cv::Mat& data){}
    std::vector<Cell*> get_neighbors(const Cell* cell, std::vector<std::vector<Cell>>& cell_mat, bool bool_4_8, std::size_t cols, std::size_t rows);
};

class Clustering : public Growing
{
public:
    Clustering(size_t treshold = 50)
        : Growing(Neighborhood::n4), n(0), treshold(treshold) {}
    size_t run(cv::Mat& data) override;
protected:
    size_t n;
    size_t treshold;
    void init_funct(std::set<Cell*>& opened, std::vector<std::vector<Cell>>& cell_mat, cv::Mat& data) override;
    void post_funct(std::vector<Cell*>& processed, cv::Mat& data) override;
};

class Voronoi : public Growing
{
public:
    Voronoi() : Growing(Neighborhood::swap){}
private:
    void init_funct(std::set<Cell*>& opened, std::vector<std::vector<Cell>>& cell_mat, cv::Mat& data) override;
};