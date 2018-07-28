// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef PRED_EXPRESSION_H__
#define PRED_EXPRESSION_H__

#include "base/basic_types.h"
#include "run_crown/symbolic_expression.h"

namespace crown {

class PredExpr : public SymbolicExpr {
public:
	 PredExpr(ops::compare_op_t op, SymbolicExpr *l, SymbolicExpr *r,
			 size_t s, Value_t v);
	 ~PredExpr();

	 PredExpr* Clone() const;
	 void AppendToString(string *s) const;

	 bool DependsOn(const map<var_t, type_t>& vars) const{
		 return left_->DependsOn(vars) || right_->DependsOn(vars);
	 }
	 void AppendVars(set<var_t>* vars) const{
		 left_->AppendVars(vars);
		 right_->AppendVars(vars);
	 }

	 bool IsConcrete() const { return false; }

	 Z3_ast ConvertToSMT(Z3_context ctx, Z3_solver sol) const;
	 Z3_ast ConvertToSMTforBV(Z3_context ctx, Z3_solver sol,
			 Z3_ast e1, Z3_ast e2) const;
	 Z3_ast ConvertToSMTforFP(Z3_context ctx, Z3_solver sol,
			 Z3_ast e1, Z3_ast e2) const;

	 const PredExpr* CastPredExpr() const { return this; }

	 bool Equals(const SymbolicExpr &e) const;

	 // Accessors
	 ops::compare_op_t compare_op() const { return compare_op_; }
	 const SymbolicExpr* left() const { return left_; }
	 const SymbolicExpr* right() const { return right_; }

private:
	 const ops::compare_op_t compare_op_;
	 const SymbolicExpr *left_, *right_;
};

}  // namespace crown

#endif  // PRED_EXPRESSION_H__
