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

#ifndef ACMULTIDIM_CPP
#define ACMULTIDIM_CPP

#include "multidimensional.h"

using namespace std;

acMultiDim::acMultiDim()
{
	dim1_size = -1;
	dim2_size = -1;
}

acMultiDim::~acMultiDim()
{
	cleanup();
}

void acMultiDim::cleanup()
{
	if( dim1_size > -1 )
	{
		dim1.erase( dim1.begin() );
		vector<string>().swap( dim1 );
	}

	if( dim2_size > -1 )
	{
		dim2.erase( dim2.begin() );
		vector<string>().swap( dim2 );
	}

	dim1_size = 0;
	dim2_size = 0;
}

int acMultiDim::push_dim1( string value )
{
	dim1_size++;
	dim1.push_back( value );
	return dim1_size;
}

int acMultiDim::push_dim2( string value )
{
	dim2_size++;
	dim2.push_back( value );
	return dim2_size;
}

void acMultiDim::set_dim1( int index, string value )
{
	if( ( index <= dim1_size ) && ( index >= 0 ) ) dim1[ index ] = value;
}

void acMultiDim::set_dim2( int index, string value )
{
	if( ( index <= dim2_size ) && ( index >= 0 ) ) dim2[ index ] = value;
}

string acMultiDim::get_dim1( int index )
{
	if( ( index <= dim1_size ) && ( index >= 0 ) ) return dim1[ index ];
	else return "";
}

string acMultiDim::get_dim2( int index )
{
	if( ( index <= dim2_size ) && ( index >= 0 ) ) return dim2[ index ];
	else return "";
}

int acMultiDim::getSize_dim1()
{
	return dim1_size;
}

int acMultiDim::getSize_dim2()
{
	return dim2_size;
}

#endif
