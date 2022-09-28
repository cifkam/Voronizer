#ifndef GROWING_HPP
#define GROWING_HPP

#include <set>
#include <vector>
#include <opencv2/core.hpp>
#include <map>


typedef int16_t int_t;
enum Neighborhood {n4, n8, alternating};
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

/*
Abstract class for region growing - 
*/
class Growing
{
public:
    Groups groups;
    CellMat cell_mat;
    Neighborhood neighborhood;
    
    // Constructor that takes type of neighborhood (4/8-neighborhood or alternating)
    Growing(Neighborhood neighborhood);
    // Public wrapper function to run the growing. 
    virtual size_t compute(cv::Mat& input, cv::Mat& output, bool clear_groups = true);
    // Returns map of stored groups (assignment of values to individual pixels) and clears the variable.
    void clear_groups();
    // Returns matrix stored in cell_mat and clears the variable.
    void clear_cellmat();

protected:
    // Main function that runs the growing
    virtual size_t compute_inner(cv::Mat& input_output_data, bool clear_groups = true);
    // Initialization of cell_mat and opened cells
    virtual void init_funct(std::set<Cell*>& opened, CellMat& cell_mat, cv::Mat& data) = 0;
    // Possible postprocessing of pixels that were assigned a new value during the call of compute_inner
    virtual void post_funct(std::vector<Cell*>& processed, cv::Mat& data);
    // Wrapper for assignment a new value to data
    virtual void assign(cv::Mat& data, Cell* from, Cell* to);
    // Wrapper for assignment a new value to data
    virtual void assign(cv::Mat& data, int value, Cell* to);
    // Wrapper for adding cell (pixel) to given group
    virtual void add_to_group(Cell* cell, int cls);
    // Checks if the the value from cell should be assigned to its neighbor
    virtual bool grow_condition(const cv::Mat& data, const cv::Mat& output, Cell* cell, Cell* neighbor);
    std::vector<Cell*> get_neighbors(const Cell* cell, CellMat& cell_mat, bool bool_4_8, std::size_t cols, std::size_t rows);
};


#endif /* GROWING_HPP */