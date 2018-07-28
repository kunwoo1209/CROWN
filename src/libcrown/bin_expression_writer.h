// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BIN_EXPRESSION_WRITER_H__
#define BIN_EXPRESSION_WRITER_H__

#include "base/basic_types.h"
#include "libcrown/symbolic_expression_writer.h"

namespace crown {

class BinExprWriter : public SymbolicExprWriter {
public:
	BinExprWriter(ops::binary_op_t op, SymbolicExprWriter *l, SymbolicExprWriter *r,
			size_t size, Value_t val);
	~BinExprWriter();


	BinExprWriter* Clone() const;
	void AppendToString(string *s) const;
	void Serialize(ostream &os) const;


	bool IsConcrete() const { return false; }

	const BinExprWriter* CastBinExprWriter() const { return this; }

	// Accessors
	const ops::binary_op_t get_binary_op() const { return binary_op_; }
	const SymbolicExprWriter* get_left() const { return left_; }
	const SymbolicExprWriter* get_right() const { return right_; }

private:
	const ops::binary_op_t binary_op_;
	const SymbolicExprWriter *left_, *right_;
};

}  // namespace crown

#endif  // BIN_EXPRESSION_WRITER_H__
