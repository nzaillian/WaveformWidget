#ifndef MATHUTIL_H
#define MATHUTIL_H
#include <math.h>
#include <stdlib.h>
#include <vector>

/*!
    \file MathUtil.h
    \brief MathUtil header/implementation file.  Contains a few useful utility math functions.
*/

using namespace std;

/*!
    \brief Contains three simple utility functions leveraged by the WaveformWidget class
*/
class MathUtil
{
public:
    /*!\brief Returns the maximum of all elements in an array of double-precision floating point values.*/
    static double getMax(double* arr)
    {
        double max = arr[0];

        if(sizeof(arr)/sizeof(double) == 0)
            return *arr;
        for(int i = 0; i < (int) (sizeof(arr)/sizeof(double)); i++)
        {
            if(arr[i]>max)
                max = arr[i];
        }
        return max;
    }

    /*!\brief Returns the maximum of all elements in a vector of double-precision floating point values.*/
    static double getVMax(vector<double> vec)
    {
        double max = vec.at(0);

        if(vec.size() == 0)
            return vec.at(0);
        for(unsigned int i = 0; i < vec.size(); i++)
        {
            if(vec[i]>max)
                max = vec[i];
        }
        return max;
    }

    /*!\brief Rounds a double-precision floating point value to the nearest integer.*/
    static double round(double value)
    {
      return floor(value + 0.5);
    }


};

#endif // MATHUTIL_H
