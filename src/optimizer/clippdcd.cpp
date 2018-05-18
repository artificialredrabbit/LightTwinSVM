#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>
#include <vector>
#include <list>
#include <iterator>
#include <armadillo>


/*
Change log:
Mar 21, 2018: A bug related to the WLTSVM was fixed. the bug caused poor accuracy. 
Bug was in section which filters out indices.

Mar 23, 2018: execution time improved significantly by computing dot product in temp var.

May 4, 2018: A trick for improving dot product computation. It imporves speed by 4-5x times.

*/

#define MAX_ITER 15000

using namespace arma;


std::vector<double> clippDCD_optimizer(std::vector<std::vector<double> > &dual, const double c)
{
    // Type conversion - STD vector -> arma mat
    mat dualMatrix = zeros<mat>(dual.size(), dual.size());

    for(unsigned int i = 0; i < dualMatrix.n_rows; i++)
    {
        dualMatrix.row(i) = conv_to<rowvec>::from(dual[i]);

    }

    // Step 1: Initial Lagrange multiplies
    vec alpha = zeros<vec>(dualMatrix.n_rows);

    // Number of iterations
    unsigned int iter = 0;

    // Tolerance value
    const double tolValue = pow(10, -5);

    // Max allowed iterations
    const unsigned int maxIter = MAX_ITER;

    // Index set
    std::vector<unsigned int> indexList(dualMatrix.n_rows);

    // Initialize index set
    std::iota(std::begin(indexList), std::end(indexList), 0);

    // Store dot product values
    std::vector<double> dotList(dualMatrix.n_rows);

    // For storing objective function value
    vec objList = zeros<vec>(dualMatrix.n_rows);

    // Create index set and computing dot products for all columns of dual mat
    for(unsigned int i = 0; i < indexList.size(); ++i)
    {

        // Computing dot product here, improves speed significantly
        double temp = dot(alpha, dualMatrix.col(indexList[i]));

        //double obj = (e(*it) - dot(alpha, dualMatrix.col(*it))) / dualMatrix(*it, *it);
        double obj = (1.0 - temp) / dualMatrix(indexList[i], indexList[i]);

        dotList[indexList[i]] = temp;

        // Remove index when it makes condition false - Filtering out indexes
        if( !((alpha(indexList[i]) < c) & (obj > 0)) )
        {
            //indexList.erase(it);
            indexList.erase(indexList.begin() + i);

        }
        else
        {
            objList(indexList[i]) = pow(1.0 - temp, 2) / dualMatrix(indexList[i], indexList[i]);
        }
    }

    // Step 2: Optimillay condition
    while(iter <= maxIter)
    {

        // Find L-index
        unsigned int L_index = index_max(objList);

        // Compute lambda
        double lambda = (1.0 - dot(alpha, dualMatrix.col(L_index))) / dualMatrix(L_index, L_index);

        // Previous alpha value
        double preAlpha = alpha(L_index);

        // Step 2.2: Update miltipliers
        alpha(L_index) = alpha(L_index) + std::max(0.0, std::min(lambda, c));

        double objValue = pow(1.0 - dot(alpha, dualMatrix.col(L_index)), 2) / dualMatrix(L_index, L_index);

        ++iter;

        // Check the convergence
        if(objValue < tolValue)
        {
            //cout << "Found!"  << "Iter: " << iter << endl;
            break;
        }

        // Zeroing!
        objList = zeros<vec>(dualMatrix.n_rows);

        // Computing index list
        for(unsigned int i = 0; i < indexList.size(); ++i)
        {

            // A trick for computing dot so much faster!
            dotList[indexList[i]] = (dotList[indexList[i]] - preAlpha * dualMatrix(indexList[i], L_index)) + (alpha(L_index) * dualMatrix(indexList[i], L_index));

            double obj = (1.0 - dotList[indexList[i]]) / dualMatrix(indexList[i], indexList[i]);

            // Remove index when it makes condition false - Filtering out indexes
            if( !((alpha(indexList[i]) < c) & (obj > 0)) )
            {
                indexList.erase(indexList.begin() + i);

            }
            else
            {
                objList(indexList[i]) = pow(1.0 - dotList[indexList[i]], 2) / dualMatrix(indexList[i], indexList[i]);

            }
        }

    }

    // Type conversion
    std::vector<double> alphaVec = conv_to<std::vector<double> >::from(alpha);

    return alphaVec;
    
}


PYBIND11_MODULE(clippdcd, m) {
    m.doc() = "ClippDCD opimizer implemented in C++ by Mir, A."; // optional module docstring

    m.def("clippDCD_optimizer", &clippDCD_optimizer, "ClippDCD algorithm - solves dual optimization problem");
}
