#include "../includes/boolean_matrix.h"

BooleanMatrix::BooleanMatrix(int _num_rows, int _num_cols) : matrix(std::vector<std::vector<bool>>(_num_rows)), num_rows(_num_rows), num_cols(_num_cols) // matrix(new bool *[num_rows])
{
    for (int i = 0; i < num_rows; ++i)
    {
        this->matrix[i].resize(num_cols, false);
        // std::fill_n(this->matrix[i], this->num_cols, false);
        // this->matrix.emplace_back(std::vector<bool>());
        // std::cout << "i: " << i << " num_rows: " << this->matrix.size() << std::endl;
        // for (int j = 0; j < num_cols; ++j)
        // {
        //     this->matrix[i].push_back(false);
        // }
    }

    // this->matrix = new bool[this->num_rows * this->num_cols];

    // std::fill_n(this->matrix, this->num_rows * this->num_cols, false);
}

BooleanMatrix::~BooleanMatrix()
{
    // for (int i = 0; i < this->num_rows; ++i)
    // {
    //     delete[] this->matrix[i];
    // }

    // delete[] this->matrix;
}

// const bool *BooleanMatrix::operator[](int row) const
// {
//     // if (row >= 0 && row < this->num_rows)
//     // {
//     //     return &this->matrix[row];
//     // }
//     // throw std::out_of_range("Row index out of range! AAAA");
// }

// bool *BooleanMatrix::operator[](int row)
// {
//     // if (row >= 0 && row < this->num_rows)
//     // {

//     //     return &this->matrix[row];
//     // }
//     // throw std::out_of_range("Row index out of range! BBBB");
// }

bool BooleanMatrix::get(int row, int col)
{
    if (row < 0 || row >= this->num_rows || col < 0 || col >= this->num_cols)
        throw std::out_of_range("Entry out of range!");

    // return this->matrix[row * this->num_cols + col];
    return this->matrix.at(row).at(col);
}

void BooleanMatrix::set(int row, int col, bool value)
{

    if (row < 0 || row >= this->num_rows || col < 0 || col >= this->num_cols)
        throw std::out_of_range("Entry out of range!");

    this->matrix.at(row).at(col) = value;
}