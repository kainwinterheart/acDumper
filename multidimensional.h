/*
Copyright (C) 2011  Kain Winterheart <http://facebook.com/kain.winterheart>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

    void cleanup();

private:
    std::vector<std::string> dim1;
    std::vector<std::string> dim2;

    int dim1_size;
    int dim2_size;
};

#endif

