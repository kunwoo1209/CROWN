// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_SYMBOLIC_EXPRESSION_FACTORY_H_
#define BASE_SYMBOLIC_EXPRESSION_FACTORY_H_

#include "run_crown/symbolic_expression.h"
#include "run_crown/unary_expression.h"
#include "run_crown/atomic_expression.h"
#include "run_crown/bin_expression.h"
#include "run_crown/pred_expression.h"
#include "run_crown/symbolic_object.h"
#include "run_crown/deref_expression.h"

namespace crown{

class SymbolicExprFactory{
 public:
  // Factory methods for constructing symbolic expressions.
	 static SymbolicExpr* NewConcreteExpr(Value_t val);
	 static SymbolicExpr* NewConcreteExpr(size_t size, Value_t val);

	 static SymbolicExpr* NewUnaryExpr(Value_t val,
			 ops::unary_op_t op, SymbolicExpr* e);

	 static SymbolicExpr* NewBinExpr(Value_t val,
			 ops::binary_op_t op,
			 SymbolicExpr* e1, SymbolicExpr* e2);

	 static SymbolicExpr* NewBinExpr(Value_t val,
			 ops::binary_op_t op,
			 SymbolicExpr* e1, Value_t e2);

	 static SymbolicExpr* NewPredExpr(Value_t val,
			 ops::compare_op_t op,
			 SymbolicExpr* e1, SymbolicExpr* e2);


	 static SymbolicExpr* Concatenate(SymbolicExpr* e1, SymbolicExpr* e2);

	 static SymbolicExpr* ExtractBytes(SymbolicExpr* e, size_t i, size_t n);
	 static SymbolicExpr* ExtractBytes(size_t size, Value_t value,
			 size_t i, size_t n);

};

} // namespace crown

#endif // BASE_SYMBOLIC_EXPRESSION_FACTORY_H_
