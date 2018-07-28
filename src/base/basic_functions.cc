/* 
 * File:   basic_functions.cc
 * Author: Tae-Jin Kim
 *
 * Created on 2015년 11월 11일 
 */

#include <assert.h>
#include <limits>
#include <string>
#include <sstream>
#include "base/basic_types.h"
#include "base/basic_functions.h"

namespace crown {
using namespace std;

std::string IntToString(int number) {
	std::ostringstream oss;
	// Works just like cout
	oss << number;
	// Return the underlying string
	return oss.str();
}

std::string* floatToBinString(float floatNum){
	string* str = new string();
	int *intNum = reinterpret_cast<int*>(&floatNum);
	for(int i = sizeof(floatNum)*8 - 1; i >=0;i--){
		str->append(IntToString((*intNum >> i)&1));
	}
	return str;
}

std::string* doubleToBinString(double doubleNum){
	string* str = new string();
	long long *intNum = reinterpret_cast<long long*>(&doubleNum);
	assert(sizeof(*intNum) == sizeof(double));
	for(int i = sizeof(doubleNum)*8 - 1; i >=0;i--){
		str->append(IntToString((*intNum >> i)&1));
	}
	return str;
}

float binStringToFloat(const char* binFloat){
	int intNum = 0;
	for(unsigned int i = 0; i <= sizeof(intNum)*8 - 1;i++){
		intNum <<= 1;
		intNum += binFloat[i] - '0';
	}

	return *reinterpret_cast<float*>(&intNum);
}

float binStringToFloat(string* binFloat){
	return binStringToFloat(binFloat->c_str());
}


double binStringToDouble(const char* binDouble){
	long long intNum = 0;
	assert(sizeof(intNum) == sizeof(double));
	for(unsigned int i = 0; i <= sizeof(intNum)*8 - 1;i++){
		intNum <<= 1;
		intNum += binDouble[i] - '0';
	}

	return *reinterpret_cast<double*>(&intNum);
}

double binStringToDouble(string* binDouble){
	return binStringToDouble(binDouble->c_str());
}

float setFloatByInts(int sign, long long int exp, long long unsigned int sig){
	//Floating point design. (-1**sign) * (1.sig) * (2**(exp-127))
	unsigned int intNum = 0;

    // HACK: If both of exp and sig are zero, intNum should be zero
    // I am not sure how to modify the code below.
    // Also, CROWN does not consider sign of 0, though it should do
    if (exp == 0 && sig == 0) return 0;

	exp = exp + 127;
	assert(0 == sign || 1 == sign);
	intNum = sign << 31 | intNum;
	intNum = ((int)exp << 23 | intNum);
	intNum = ((int)sig | intNum);

	return *reinterpret_cast<float*>(&intNum);
}

double setDoubleByInts(int sign, long long int exp, long long unsigned int sig){
	//Floating point design. (-1**sign) * (1.sig) * (2**(exp-127))
	assert(sizeof(long long int) == sizeof(double));

	unsigned long long int intNum = 0;

    // HACK: If both of exp and sig are zero, intNum should be zero
    // I am not sure how to modify the above code.
    // Also, CROWN does not consider sign of 0, though it should do
    if (exp == 0 && sig == 0) return 0;

	exp = exp + 1023;
	assert(0 == sign || 1 == sign);
	intNum = (long long int)sign << 63 | intNum;
	intNum = ((long long int)exp << 52 | intNum);
	intNum = ((long long int)sig | intNum);

	return *reinterpret_cast<double*>(&intNum);
}

}  // namespace crown

