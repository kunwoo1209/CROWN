/* 
 * File:   basic_functions.h
 * Author: Tae-Jin Kim
 *
 * Created on 2015년 11월 11일 
 */

#ifndef BASE_BASIC_FUNCTIONS__
#define BASE_BASIC_FUNCTIONS__

#include "base/basic_types.h"

namespace crown {

	std::string* floatToBinString(float floatNum);
	std::string* doubleToBinString(double doubleNum);
	float binStringToFloat(const char* binFloat);
	float binStringToFloat(std::string* binFloat);
	double binStringToDouble(const char* binDouble);
	double binStringToDouble(std::string* binDouble);
	float setFloatByInts(int sign, long long int exp, long long unsigned int sig);
	double setDoubleByInts(int sign, long long int exp, long long unsigned int sig);

	//To print out concrete arrays
}

#endif
