#include "../includes/boolean_matrix.h"

BooleanMatrix::BooleanMatrix(int num_rows, int num_cols) : 
    matrix(new bool*[num_rows])
{
    for(int i = 0; i < num_rows; ++i)
        this->matrix[i] = new bool[num_cols];

    for(int i = 0; i < num_rows; ++i)
    {
        for(int j = 0; j < num_cols; ++j)
        {
            this->matrix[i][j] = false;
        }
    }

    this->num_cols = num_cols;
    this->num_rows = num_rows;

}


BooleanMatrix::~BooleanMatrix()
{
    for(int i = 0; i < this->num_rows; ++i)
    {
        delete[] this->matrix[i];
    }

    delete[] this->matrix;
}

const bool* BooleanMatrix::operator[](int row) const
{
    if (row >= 0 && row < this->num_rows)
        return this->matrix[row];
    
    throw std::out_of_range("Row index out of range!");
}

bool* BooleanMatrix::operator[](int row)
{
    if(row >= 0 && row < this->num_rows)
        return this->matrix[row];
    throw std::out_of_range("Row index out of range!");
}