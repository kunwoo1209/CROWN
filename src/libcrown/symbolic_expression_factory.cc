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
#include "libcrown/symbolic_expression_factory.h"
#include "libcrown/symbolic_expression_writer.h"

namespace crown{

SymbolicExprWriter* SymbolicExprWriterFactory::NewConcreteExpr(Value_t val) {
#ifdef DEBUG
	std::cerr<<"Concrete: "<<val.integral<<" "<<val.floating<<"\n";
#endif
	val.type = val.type;
	return new SymbolicExprWriter(kSizeOfType[val.type], val);
}

SymbolicExprWriter* SymbolicExprWriterFactory::NewConcreteExpr(size_t size, Value_t val) {
#ifdef DEBUG
	std::cerr<<"Concrete2: "<<val.integral<<" "<<val.floating<<"\n";
#endif
	return new SymbolicExprWriter(size, val);
}

SymbolicExprWriter* SymbolicExprWriterFactory::NewUnaryExprWriter(Value_t val,
		ops::unary_op_t op, SymbolicExprWriter* e) {
	return new UnaryExprWriter(op, e, kSizeOfType[val.type], val);
}

SymbolicExprWriter* SymbolicExprWriterFactory::NewBinExprWriter(Value_t val,
		ops::binary_op_t op,
		SymbolicExprWriter* e1, SymbolicExprWriter* e2) {
	return new BinExprWriter(op, e1, e2, kSizeOfType[val.type], val);
}

SymbolicExprWriter* SymbolicExprWriterFactory::NewBinExprWriter(Value_t val,
		ops::binary_op_t op,
		SymbolicExprWriter* e1, Value_t e2) {
	return new BinExprWriter(op, e1, NewConcreteExpr(e2), kSizeOfType[val.type], val);
}


SymbolicExprWriter* SymbolicExprWriterFactory::NewPredExprWriter(Value_t val,
		ops::compare_op_t op,
		SymbolicExprWriter* e1, SymbolicExprWriter* e2) {
	return new PredExprWriter(op, e1, e2, kSizeOfType[val.type], val);
}

SymbolicExprWriter* SymbolicExprWriterFactory::Concatenate(SymbolicExprWriter *e1, SymbolicExprWriter *e2) {
	Value_t val = Value_t();
	val.type = types::U_LONG;//FIXME
	val.integral = (e1->value().integral << (8 * e2->size())) + e2->value().integral;
	return new BinExprWriter(ops::CONCAT,
			e2, e1,
			e1->size() + e2->size(),
			val);
}

SymbolicExprWriter* SymbolicExprWriterFactory::NewDerefExprWriter(Value_t val,
				SymbolicObjectWriter& obj, SymbolicExprWriter* addr) {
	return new DerefExprWriter(addr, &obj, kSizeOfType[val.type], val);
}

SymbolicExprWriter* SymbolicExprWriterFactory::NewConstDerefExprWriter(Value_t val,
		SymbolicObjectWriter& obj, addr_t addr) {

	return new DerefExprWriter(NewConcreteExpr(Value_t(addr, 0, types::U_LONG)),
			&obj, kSizeOfType[val.type], val);
}



SymbolicExprWriter* SymbolicExprWriterFactory::ExtractBytes(size_t size, Value_t value,
		size_t i, size_t n) {
	//assert(i % n == 0);
	Value_t val = value;
	val.integral = (value.integral >> (8*i)) & ((1 << (8*n)) - 1);
	return new SymbolicExprWriter(n, val);
}


SymbolicExprWriter* SymbolicExprWriterFactory::ExtractBytes(SymbolicExprWriter* e, size_t i, size_t n) {
	//assert(i % n == 0);

	// Extracting i-th, i+1-th, ..., i+n-1-th least significant bytes.

	Value_t val = Value_t();
	Value_t val2 = Value_t();
	val2.integral = i;
	val2.type = types::U_LONG;
	val.type = types::U_LONG;
	val.integral = (e->value().integral >> (8*i)) & ((1 << (8*n)) - 1);
	SymbolicExprWriter* i_e = NewConcreteExpr(val2);
	return new BinExprWriter(ops::EXTRACT, e, i_e,  n, val);
}

} // namespace crown

