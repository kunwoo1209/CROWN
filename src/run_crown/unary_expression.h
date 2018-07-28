// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef UNARY_EXPRESSION_H__
#define UNARY_EXPRESSION_H__

#include "base/basic_types.h"
#include "run_crown/symbolic_expression.h"

namespace crown {

class UnaryExpr : public SymbolicExpr {
 public:
  UnaryExpr(ops::unary_op_t op, SymbolicExpr *c, size_t s, Value_t v);
  ~UnaryExpr();

  UnaryExpr* Clone() const;
  void AppendToString(string *s) const;

  void AppendVars(set<var_t>* vars) const {
	  child_->AppendVars(vars);
  }

  bool DependsOn(const map<var_t,type_t>& vars) const {
	  return child_->DependsOn(vars);
  }


  bool IsConcrete() const { return false; }

  Z3_ast ConvertToSMT(Z3_context ctx, Z3_solver sol) const;

  const UnaryExpr* CastUnaryExpr() const { return this; }

  bool Equals(const SymbolicExpr &e) const;

  // Accessors
  ops::unary_op_t unary_op() const { return unary_op_; }
  const SymbolicExpr* child() const { return child_; }

 private:
  const SymbolicExpr *child_;
  const ops::unary_op_t unary_op_;
};

}  // namespace crown

#endif  // UNARY_EXPRESSION_H__
