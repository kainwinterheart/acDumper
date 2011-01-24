#ifndef ACMULTIDIM_CPP
#define ACMULTIDIM_CPP

#include "multidimensional.h"

using namespace std;

acMultiDim::acMultiDim() {
	dim1_size = -1;
	dim2_size = -1;
}

acMultiDim::~acMultiDim() {
	if (dim1_size > -1) {
		dim1.erase(dim1.begin());
		vector<string>().swap(dim1);
	}

	if (dim2_size > -1) {
		dim2.erase(dim2.begin());
		vector<string>().swap(dim2);
	}

	dim1_size = 0;
	dim2_size = 0;
}

int acMultiDim::push_dim1 (string value) {
	dim1_size++;
	dim1.push_back(value);
	return dim1_size;
}

int acMultiDim::push_dim2 (string value) {
	dim2_size++;
	dim2.push_back(value);
	return dim2_size;
}

void acMultiDim::set_dim1 (int index, string value) {
	if ((index <= dim1_size) && (index >= 0)) {
		dim1[index] = value;
	}
}

void acMultiDim::set_dim2 (int index, string value) {
	if ((index <= dim2_size) && (index >= 0)) {
		dim2[index] = value;
	}
}

string acMultiDim::get_dim1 (int index) {
	if ((index <= dim1_size) && (index >= 0)) {
		return dim1[index];
	} else {
		return "";
	}
}

string acMultiDim::get_dim2 (int index) {
	if ((index <= dim2_size) && (index >= 0)) {
		return dim2[index];
	} else {
		return "";
	}
}

int acMultiDim::getSize_dim1 () {
	return dim1_size;
}

int acMultiDim::getSize_dim2 () {
	return dim2_size;
}

#endif
