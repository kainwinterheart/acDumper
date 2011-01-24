#ifndef ACMULTIDIM_H
#define ACMULTIDIM_H

#include <iostream>
#include <string.h>
#include <vector>

class acMultiDim {
public:
    acMultiDim();
    ~acMultiDim();

    int push_dim1 (std::string);
    int push_dim2 (std::string);

    void set_dim1 (int, std::string);
    void set_dim2 (int, std::string);

    std::string get_dim1 (int);
    std::string get_dim2 (int);

    int getSize_dim1 ();
    int getSize_dim2 ();

private:
    std::vector<std::string> dim1;
    std::vector<std::string> dim2;

    int dim1_size;
    int dim2_size;
};

#endif

