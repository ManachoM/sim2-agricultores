#ifndef _BOOLEAN_MATRIX_
#define _BOOLEAN_MATRIX_

#include "glob.h"

class BooleanMatrix
{
private:
    bool **matrix;

public:
    int num_rows;
    int num_cols;

    BooleanMatrix(int num_rows = 0, int num_cols = 0);

    ~BooleanMatrix();

    const bool *operator[](int row) const;

    bool *operator[](int row);
};

#endif // !_BOOLEAN_MATRIX_