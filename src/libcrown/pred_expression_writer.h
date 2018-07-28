// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef PRED_EXPRESSION_WRITER_H__
#define PRED_EXPRESSION_WRITER_H__

#include "base/basic_types.h"
#include "libcrown/symbolic_expression_writer.h"

namespace crown {

class PredExprWriter : public SymbolicExprWriter {
public:
	PredExprWriter(ops::compare_op_t op, SymbolicExprWriter *l, SymbolicExprWriter *r,
			size_t s, Value_t v);
	~PredExprWriter();

	PredExprWriter* Clone() const;
	void AppendToString(string *s) const;
	void Serialize(ostream &os) const;

	bool IsConcrete() const { return false; }

	const PredExprWriter* CastPredExprWriter() const { return this; }


	// Accessors
	ops::compare_op_t compare_op() const { return compare_op_; }
	const SymbolicExprWriter* left() const { return left_; }
	const SymbolicExprWriter* right() const { return right_; }

private:
	const ops::compare_op_t compare_op_;
	const SymbolicExprWriter *left_, *right_;
};

}  // namespace crown

#endif  // PRED_EXPRESSION_WRITER_H__
