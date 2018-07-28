// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <assert.h>
#include <iostream>
#include <cstring>
#include "run_crown/symbolic_expression_factory.h"
#include "run_crown/symbolic_expression.h"

namespace crown{

SymbolicExpr* SymbolicExprFactory::NewConcreteExpr(Value_t val) {
#ifdef DEBUG
	std::cerr<<"Concrete: "<<val.integral<<" "<<val.floating<<"\n";
#endif
	val.type = val.type;
	return new SymbolicExpr(kSizeOfType[val.type], val);
}

SymbolicExpr* SymbolicExprFactory::NewConcreteExpr(size_t size, Value_t val) {
#ifdef DEBUG
	std::cerr<<"Concrete2: "<<val.integral<<" "<<val.floating<<"\n";
#endif
	return new SymbolicExpr(size, val);
}

SymbolicExpr* SymbolicExprFactory::NewUnaryExpr(Value_t val,
		ops::unary_op_t op, SymbolicExpr* e) {
	return new UnaryExpr(op, e, kSizeOfType[val.type], val);
}

SymbolicExpr* SymbolicExprFactory::NewBinExpr(Value_t val,
		ops::binary_op_t op,
		SymbolicExpr* e1, SymbolicExpr* e2) {
	return new BinExpr(op, e1, e2, kSizeOfType[val.type], val);
}

SymbolicExpr* SymbolicExprFactory::NewBinExpr(Value_t val,
		ops::binary_op_t op,
		SymbolicExpr* e1, Value_t e2) {
	return new BinExpr(op, e1, NewConcreteExpr(e2), kSizeOfType[val.type], val);
}


SymbolicExpr* SymbolicExprFactory::NewPredExpr(Value_t val,
		ops::compare_op_t op,
		SymbolicExpr* e1, SymbolicExpr* e2) {
	return new PredExpr(op, e1, e2, kSizeOfType[val.type], val);
}

SymbolicExpr* SymbolicExprFactory::Concatenate(SymbolicExpr *e1, SymbolicExpr *e2) {
	Value_t val = Value_t();
	val.type = types::U_LONG;//FIXME
	val.integral = (e1->value().integral << (8 * e2->size())) + e2->value().integral;
	return new BinExpr(ops::CONCAT,
			e2, e1,
			e1->size() + e2->size(),
			val);
}


SymbolicExpr* SymbolicExprFactory::ExtractBytes(size_t size, Value_t value,
		size_t i, size_t n) {
	//assert(i % n == 0);
	Value_t val = value;
	val.integral = (value.integral >> (8*i)) & ((1 << (8*n)) - 1);
	return new SymbolicExpr(n, val);
}


SymbolicExpr* SymbolicExprFactory::ExtractBytes(SymbolicExpr* e, size_t i, size_t n) {
	//assert(i % n == 0);

	// Extracting i-th, i+1-th, ..., i+n-1-th least significant bytes.
	Value_t val = Value_t();
	val.type = types::U_LONG;
	val.integral = (e->value().integral >> (8*i)) & ((1 << (8*n)) - 1);
	SymbolicExpr* i_e = NewConcreteExpr(val);
	return new BinExpr(ops::EXTRACT, e, i_e,  n, val);
}

} // namespace crown

