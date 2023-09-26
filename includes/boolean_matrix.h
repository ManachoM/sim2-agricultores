#ifndef _BOOLEAN_MATRIX_
#define _BOOLEAN_MATRIX_

#include "glob.h"

class BooleanMatrix
{
private:
    bool **matrix;
    int num_rows;
    int num_cols;

public:
    BooleanMatrix(int num_rows, int num_cols);
    
    ~BooleanMatrix();

    const bool *operator[](int row) const;

    bool *operator[](int row);
};

#endif // !_BOOLEAN_MATRIX_