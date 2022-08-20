#include "growing.hpp"
using namespace std;


size_t Growing::run(cv::Mat& data)
{
    bool bool_4_8 = (neighborhood == Neighborhood::n4);
    set<Cell*> opened;
    auto cell_mat = vector<vector<Cell>>(
        data.rows, vector<Cell>(
        data.cols, Cell(0, 0, State::unseen)
    ));
    auto rows = data.rows;
    auto cols = data.cols;
    for (size_t row = 0; row < rows; ++row)
        for (size_t col = 0; col < cols; ++col)
        {
            cell_mat[row][col].row = row;
            cell_mat[row][col].col = col;
        }
    init_funct(opened, cell_mat, data);
    
    size_t steps = 0;

    vector<Cell*> processed;
    //processed.reserve(rows*cols);

    while (opened.size() > 0)
    {
        set<Cell*> new_cells;
        for (auto cell : opened)
        {
            for (auto&& neighbor : get_neighbors(cell, cell_mat, bool_4_8, cols, rows))
                if (neighbor->state == State::unseen)
                {
                    new_cells.insert(neighbor);
                    data.at<int16_t>(neighbor->row, neighbor->col) = data.at<int16_t>(cell->row, cell->col);
                    neighbor->state = State::opened;
                }
            
            cell->state = State::closed;
            processed.push_back(cell);
        }

        if (neighborhood == Neighborhood::swap)
            bool_4_8 = !bool_4_8;
        
        opened = move(new_cells);
        steps += 1;
    }
    post_funct(processed, data);
    return steps;
};


vector<Cell*> Growing::get_neighbors(
    const Cell* cell, vector<vector<Cell>>& cell_mat,
    bool bool_4_8,
    size_t cols,
    size_t rows
){
    vector<int> row_delta;
    vector<int> col_delta;
    vector<Cell*> neighbors;

    if (cell->row == 0)
        row_delta = {0,1};
    else if (cell->row == rows-1)
        row_delta = {-1,0};
    else
        row_delta = {-1,0,1};

    if (cell->col == 0)
        col_delta = {0,1};
    else if (cell->col == cols-1)
        col_delta = {-1,0};
    else
        col_delta = {-1,0,1};

    if (bool_4_8)
    {
        for (auto r : row_delta)
        {
            if (r == 0) continue;
            Cell* c = &cell_mat[cell->row+r][cell->col]; 
            neighbors.push_back(c);
        }
        for (auto c : col_delta)
        {
            if (c == 0) continue;
            neighbors.push_back(&cell_mat[cell->row][cell->col+c]);
        }
    }
    else
    {
        for (auto r : row_delta)
            for (auto c : col_delta)
            {
                if (r == 0 && c == 0) continue;
                neighbors.push_back(&cell_mat[cell->row+r][cell->col+c]);
            }
    }

    return neighbors;
}

void Clustering::init_funct(set<Cell*>& opened, vector<vector<Cell>>& cell_mat, cv::Mat& data)
{
    Cell* cell = nullptr;

    for (int row = 0; row < data.rows; ++row)
    {
        for (int col = 0; col < data.cols; ++col)
            cell_mat[row][col].state = (data.at<int16_t>(row,col) == 0 ?
                State::unseen : State::closed);
    }
    for (int row = 0; row < data.rows; ++row)
    {
        for (int col = 0; col < data.cols; ++col)
        {
            if (data.at<int16_t>(row,col) == 0)
            {
                cell = &cell_mat[row][col];
                break;
            }
        }

        if (cell != nullptr) break; 
    }
    if (cell == nullptr) return;


    cell->state = State::opened;
    data.at<int16_t>(cell->row,cell->col) = n;
    opened.insert(cell);
}

void Clustering::post_funct(std::vector<Cell*>& processed, cv::Mat& data)
{
    if (processed.size() < treshold)
        for (auto cell : processed)
            data.at<int16_t>(cell->row, cell->col) = -1;        
}

size_t Clustering::run(cv::Mat& data)
{
    data.convertTo(data, CV_16S);
    data /= 255;
    data -= 1;

    n = 1;
    size_t steps = 0;
    while (true)
    {
        //TODO: reuse cell_mat instead of creating it always again
        size_t s = Growing::run(data); 
        steps += s;
        if (s == 0)
            break;
        n++; 
    }

    n = 0;
    data += 1;
    //data.convertTo(data, CV_8U);
    return steps;


}

void Voronoi::init_funct(set<Cell*>& opened, vector<vector<Cell>>& cell_mat, cv::Mat& data)
{
    for (int row = 0; row < data.rows; ++row)
        for (int col = 0; col < data.cols; ++col)
        {
            if (data.at<int16_t>(row,col) != 0)
            {
                Cell* cell = &cell_mat[row][col];
                cell->state = State::opened;
                opened.insert(cell);
            }
        }
}