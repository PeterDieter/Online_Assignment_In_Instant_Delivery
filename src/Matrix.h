
#ifndef MATRIX_H
#define MATRIX_H

#include <vector>

// Implementation of a matrix in a C++ vector
// This class is used because a flat vector is faster than a vector of vectors which requires two lookup operations rather than one to index a matrix element
class Matrix
{
    int cols_;                  // The number of columns of the matrix
    int rows_;                  // The number of rows of the matrix
    std::vector<int> data_;     // The vector where all the data is stored (this represents the matrix)

public:
    // Empty constructor: with zero columns and rows and a vector of size zero
    Matrix() : cols_(0), rows_(0), data_(std::vector<int>(0))
    {}

    // Constructor: create a matrix of size dimension by dimension, using a C++ vector of size dimension * dimension 
    Matrix(const int dimensionX, const int dimensionY) : cols_(dimensionY), rows_(dimensionX)
    {
        data_ = std::vector<int>(dimensionX * dimensionY);
    }

    // Set a value val at position (row, col) in the matrix
    void set(const int row, const int col, const int val)
    {
        data_[cols_ * row + col] = val;
    }

    // Get the value at position (row, col) in the matrix
    int get(const int row, const int col) const
    {
        return data_[cols_ * row + col];
    }

    // Get row of the matrix
    std::vector<int> getRow(const int row) const
    {
        std::vector<int> vectorSlice; 
        
        for (int col=0; col<cols_; col++)
        {
            vectorSlice.push_back(data_[cols_ * row + col]);
        }
        
        return vectorSlice;
    }

};

#endif
