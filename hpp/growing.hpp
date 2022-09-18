#ifndef GROWING_HPP
#define GROWING_HPP

#include <set>
#include <vector>
#include <opencv4/opencv2/core.hpp>
//#include <unordered_map>
#include <map>


typedef int16_t int_t;
enum Neighborhood {n4, n8, swap};
enum State {unseen, opened, closed};

struct Cell
{
    Cell(size_t row, size_t col, State state);
    friend bool operator<(const Cell& lhs, const Cell& rhs);

    size_t row;
    size_t col;
    State state;
};

typedef std::map<int, std::vector<Cell*>> Groups;
typedef std::vector<std::vector<Cell>> CellMat;

class Growing
{
public:
    Groups groups;
    CellMat cell_mat;

    Neighborhood neighborhood;
    Growing(Neighborhood neighborhood);
    virtual size_t compute(cv::Mat& input, cv::Mat& output, bool clear_groups = true);
    Groups clear_groups();
    CellMat clear_cellmat();

protected:
    virtual size_t compute_inner(cv::Mat& input_output_data, bool clear_groups = true);
    virtual void init_funct(std::set<Cell*>& opened, CellMat& cell_mat, cv::Mat& data) = 0;
    virtual void post_funct(std::vector<Cell*>& processed, cv::Mat& data);
    virtual void assign(cv::Mat& data, Cell* from, Cell* to);
    virtual void assign(cv::Mat& data, int value, Cell* to);
    virtual void add_to_group(Cell* cell, int cls);
    virtual bool grow_condition(const cv::Mat& data, const cv::Mat& output, Cell* cell, Cell* neighbor);
    std::vector<Cell*> get_neighbors(const Cell* cell, CellMat& cell_mat, bool bool_4_8, std::size_t cols, std::size_t rows);
};


#endif /* GROWING_HPP */