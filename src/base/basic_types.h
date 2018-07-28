// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CREST, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_BASIC_TYPES_H__
#define BASE_BASIC_TYPES_H__

#include <string>
#include <cstddef>
#include <tuple>

namespace crown {

typedef int id_t;
typedef int branch_id_t;
typedef unsigned int function_id_t;
typedef unsigned int var_t;
typedef long long int value_t;
typedef double fp_value_t;
typedef unsigned long int addr_t;
typedef std::tuple<bool, unsigned int, const char*, const char *> branch_info_t;

// Virtual "branch ID's" used to represent function calls and returns.

static const branch_id_t kCallId = -1;
static const branch_id_t kReturnId = -2;

namespace ops {

	enum compare_op_t {
		EQ   = 0,  NEQ  = 1,
		GT   = 2,  LE   = 3,  LT   = 4,  GE = 5,
		S_GT = 6,  S_LE = 7,  S_LT = 8,  S_GE = 9
	};

	enum binary_op_t {
		ADD = 0,           SUBTRACT = 1,     MULTIPLY = 2,
		DIV = 3,           S_DIV = 4,
		MOD = 5,           S_MOD = 6,
		SHIFT_L = 7,       SHIFT_R = 8,      S_SHIFT_R = 9,
		BITWISE_AND = 10,  BITWISE_OR = 11,  BITWISE_XOR = 12,
		CONCAT = 13,       EXTRACT = 14,     CONCRETE = 15
	};

	enum pointer_op_t {
		ADD_PI = 0,       S_ADD_PI = 1,
		SUBTRACT_PI = 2,  S_SUBTRACT_PI = 3,
		SUBTRACT_PP = 4
	};

	enum unary_op_t {
		NEGATE = 0, LOGICAL_NOT = 1, BITWISE_NOT = 2,
		UNSIGNED_CAST = 3, SIGNED_CAST = 4
	};

}  // namespace ops

using ops::compare_op_t;
using ops::binary_op_t;
using ops::pointer_op_t;
using ops::unary_op_t;

compare_op_t NegateCompareOp(compare_op_t op);

extern const char* kCompareOpStr[];
extern const char* kBinaryOpStr[];
extern const char* kUnaryOpStr[];

// C numeric types.

namespace types {
	enum type_t {
		BOOLEAN = -1,
		U_CHAR = 0,       CHAR = 1,
		U_SHORT = 2,      SHORT = 3,
		U_INT = 4,        INT = 5,
		U_LONG = 6,       LONG = 7,
		U_LONG_LONG = 8,  LONG_LONG = 9,
		STRUCT = 10, POINTER = 11,
		FLOAT = 12, 
		DOUBLE = 13, LONG_DOUBLE = 14,
		BITFIELD_CHAR = 15,
		BITFIELD_UNION_SHORT = 16,
		BITFIELD_UNION_INT = 17,
		BITFIELD_UNION_LONGLONG = 18,
		};
}
using types::type_t;

/*
 * written by Hyunwoo Kim (17.07.13)
 * the new type to save location information is defined.
 * (filename, line number)
 */
typedef struct Loc_t{
    std::string fname;
    int lineno;
    Loc_t() : fname(""),lineno(-1){}
    Loc_t(std::string fname_, int lineno_) :
        fname(fname_), lineno(lineno_){}
}Loc_t;

typedef struct Value_t{
	value_t integral;
	fp_value_t floating;
	type_t type;
	Value_t() : integral(0),floating(0),type(types::STRUCT){}
	Value_t(value_t integral_, fp_value_t floating_, type_t type_) :
		integral(integral_), floating(floating_), type(type_){}
}Value_t;

extern const char* kMinValueStr[];
extern const char* kMaxValueStr[];

extern const value_t kMinValue[];
extern const value_t kMaxValue[];

extern const size_t kSizeOfType[];


}  // namespace crown

#endif  // BASE_BASIC_TYPES_H__
