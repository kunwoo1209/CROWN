// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef UNARY_EXPRESSION_WRITER_H__
#define UNARY_EXPRESSION_WRITER_H__

#include "base/basic_types.h"
#include "libcrown/symbolic_expression_writer.h"

namespace crown {

class UnaryExprWriter : public SymbolicExprWriter {
public:
	UnaryExprWriter(ops::unary_op_t op, SymbolicExprWriter *c, size_t s, Value_t v);
	~UnaryExprWriter();

	UnaryExprWriter* Clone() const;
	void AppendToString(string *s) const;
	void Serialize(ostream &os) const;

	bool IsConcrete() const { return false; }


	const UnaryExprWriter* CastUnaryExprWriter() const { return this; }

	// Accessors
	ops::unary_op_t unary_op() const { return unary_op_; }
	const SymbolicExprWriter* child() const { return child_; }

private:
	const SymbolicExprWriter *child_;
	const ops::unary_op_t unary_op_;
};

}  // namespace crown

#endif  // UNARY_EXPRESSION_WRITER_H__
