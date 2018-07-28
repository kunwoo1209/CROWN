// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_SYMBOLIC_EXPRESSION_FACTORY_H_
#define BASE_SYMBOLIC_EXPRESSION_FACTORY_H_

#include "libcrown/symbolic_expression_writer.h"
#include "libcrown/unary_expression_writer.h"
#include "libcrown/atomic_expression_writer.h"
#include "libcrown/bin_expression_writer.h"
#include "libcrown/pred_expression_writer.h"
#include "libcrown/symbolic_object_writer.h"
#include "libcrown/deref_expression_writer.h"

namespace crown{

class SymbolicExprWriterFactory{
 public:
  // Factory methods for constructing symbolic expressions.
	 static SymbolicExprWriter* NewConcreteExpr(Value_t val);
	 static SymbolicExprWriter* NewConcreteExpr(size_t size, Value_t val);

	 static SymbolicExprWriter* NewUnaryExprWriter(Value_t val,
			 ops::unary_op_t op, SymbolicExprWriter* e);

	 static SymbolicExprWriter* NewBinExprWriter(Value_t val,
			 ops::binary_op_t op,
			 SymbolicExprWriter* e1, SymbolicExprWriter* e2);

	 static SymbolicExprWriter* NewBinExprWriter(Value_t val,
			 ops::binary_op_t op,
			 SymbolicExprWriter* e1, Value_t e2);

	 static SymbolicExprWriter* NewPredExprWriter(Value_t val,
			 ops::compare_op_t op,
			 SymbolicExprWriter* e1, SymbolicExprWriter* e2);

	 static SymbolicExprWriter* NewDerefExprWriter(Value_t val,
			 SymbolicObjectWriter& obj, SymbolicExprWriter* addr);
	 static SymbolicExprWriter* NewConstDerefExprWriter(Value_t val,
			 SymbolicObjectWriter& obj, addr_t addr);


	 static SymbolicExprWriter* Concatenate(SymbolicExprWriter* e1, SymbolicExprWriter* e2);

	 static SymbolicExprWriter* ExtractBytes(SymbolicExprWriter* e, size_t i, size_t n);
	 static SymbolicExprWriter* ExtractBytes(size_t size, Value_t value,
			 size_t i, size_t n);

};

} // namespace crown

#endif // BASE_SYMBOLIC_EXPRESSION_FACTORY_H_
