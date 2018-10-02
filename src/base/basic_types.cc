// Copyright (c) 2015-2018, Software Testing & Verification Group
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <limits>
#include "base/basic_types.h"

using std::numeric_limits;

namespace crown {

	compare_op_t NegateCompareOp(compare_op_t op) {
		return static_cast<compare_op_t>(op ^ 1);
	}

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

	const char* kCompareOpStr[] = {
		"==", "!=",
		">", "<=", "<", ">=",
		">", "<=", "<", ">="
	};

	const char* kBinaryOpStr[] = {
		"+", "-", "*",
		"/", "/",
		"%", "%",
		"<<", ">>>", ">>",
		"&", "|", "^",
		"@", "<-->", "?"
	};

	const char* kUnaryOpStr[] = {
		"-", "!", "~", "u_cast", "s_cast"
	};

	const char* kMinValueStr[] = {
		"0",
		"-128",
		"0",
		"-32768",
		"0",
		"-2147483648",
		"0",
		"-2147483648",
		"0",
		"-9223372036854775808",
	};

	const char* kMaxValueStr[] = {
		"255",
		"127",
		"65535",
		"32767",
		"4294967295",
		"2147483647",
		"4294967295",
		"2147483647",
		"18446744073709551615",
		"9223372036854775807",
	};

	const value_t kMinValue[] = {
		(value_t)numeric_limits<unsigned char>::min(),
		(value_t)numeric_limits<char>::min(),
		(value_t)numeric_limits<unsigned short>::min(),
		(value_t)numeric_limits<short>::min(),
		(value_t)numeric_limits<unsigned int>::min(),
		(value_t)numeric_limits<int>::min(),
		(value_t)numeric_limits<unsigned long>::min(),
		(value_t)numeric_limits<long>::min(),
		(value_t)numeric_limits<unsigned long long>::min(),
		(value_t)numeric_limits<long long>::min(),
		0,
		(value_t)numeric_limits<unsigned long>::min(),
		(value_t)numeric_limits<float>::min(),
		(value_t)numeric_limits<double>::min(),
		(value_t)numeric_limits<long double>::min(), 
		(value_t)numeric_limits<unsigned char>::min(),	
		(value_t)numeric_limits<unsigned short>::min(),
		(value_t)numeric_limits<unsigned int>::min(),
		(value_t)numeric_limits<unsigned long long>::min(),
	};

	const value_t kMaxValue[] = {
		(value_t)numeric_limits<unsigned char>::max(),
		(value_t)numeric_limits<char>::max(),
		(value_t)numeric_limits<unsigned short>::max(),
		(value_t)numeric_limits<short>::max(),
		(value_t)numeric_limits<unsigned int>::max(),
		(value_t)numeric_limits<int>::max(),
		(value_t)numeric_limits<unsigned long>::max(),
		(value_t)numeric_limits<long>::max(),
		(value_t)numeric_limits<unsigned long long>::max(),
		(value_t)numeric_limits<long long>::max(),
		0,
		(value_t)numeric_limits<unsigned long>::max(),
		(value_t)numeric_limits<float>::max(),
		(value_t)numeric_limits<double>::max(),
		(value_t)numeric_limits<long double>::max(),
		(value_t)numeric_limits<unsigned char>::max(),
		(value_t)numeric_limits<unsigned short>::max(),
		(value_t)numeric_limits<unsigned int>::max(),
		(value_t)numeric_limits<unsigned long long>::max(),



	};

	const size_t kSizeOfType[] = {
		sizeof(unsigned char),
		sizeof(signed char),
		sizeof(unsigned short),
		sizeof(short),
		sizeof(unsigned int),
		sizeof(int),
		sizeof(unsigned long int),
		sizeof(long int),
		sizeof(unsigned long long int),
		sizeof(long long int),
		0,
		sizeof(void *),
		sizeof(float),
		sizeof(double),
		sizeof(long double),
		sizeof(unsigned char),
		sizeof(unsigned short),
		sizeof(unsigned int),
		sizeof(unsigned long long),

	};

}  // namespace crown

