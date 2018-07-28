// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <assert.h>
#include "base/basic_functions.h"
#include "libcrown/pred_expression_writer.h"

namespace crown {

PredExprWriter::PredExprWriter(ops::compare_op_t op, SymbolicExprWriter *l, SymbolicExprWriter *r, size_t s, Value_t v)
	: SymbolicExprWriter(s, v), compare_op_(op), left_(l), right_(r) {
//	std::cerr<<"new PredExprWriter "<<op<<" "<<v.floating<<"\n";
}

PredExprWriter::~PredExprWriter() {
	delete left_;
	delete right_;
}

PredExprWriter* PredExprWriter::Clone() const {
	return new PredExprWriter(compare_op_, left_->Clone(), right_->Clone(),
			size(), value());
}
#if 0
void PredExprWriter::AppendVars(set<var_t>* vars) const {
	left_->AppendVars(vars);
	right_->AppendVars(vars);
}

bool PredExprWriter::DependsOn(const map<var_t,type_t>& vars) const {
	return left_->DependsOn(vars) || right_->DependsOn(vars);
}
#endif
void PredExprWriter::AppendToString(string *s) const {
	s->append("(");
	s->append(kCompareOpStr[compare_op_]);
	s->append(" ");
	left_->AppendToString(s);
	s->append(" ");
	right_->AppendToString(s);
	s->append(")");
}


void PredExprWriter::Serialize(ostream &os) const {
	SymbolicExprWriter::Serialize(os, kCompareNodeTag);
	os.write((char*)&compare_op_, sizeof(char));
	left_->Serialize(os);
	right_->Serialize(os);
}

}  // namespace crown
