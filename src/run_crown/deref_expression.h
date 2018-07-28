// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

/***
 * Author: Sudeep Juvekar (sjuvekar@eecs.berkeley.edu)
 */

// TODO: Implement Parse and Serialize

#ifndef DEREF_EXPRESSION_H__
#define DEREF_EXPRESSION_H__

#include "base/basic_types.h"
#include "run_crown/symbolic_expression.h"
#include "run_crown/symbolic_object.h"
namespace crown {

class DerefExpr : public SymbolicExpr {
public:
	DerefExpr(SymbolicExpr* addr, size_t managerIdx, size_t snapshotIdx,
			size_t size, Value_t val);

	DerefExpr(const DerefExpr& de);
	~DerefExpr();

	DerefExpr* Clone() const;

	void AppendVars(set<var_t>* vars) const;
	bool DependsOn(const map<var_t,type_t>& vars) const;
	void AppendToString(string *s) const;

	bool IsConcrete() const { return false; }

	Z3_ast ConvertToSMT(Z3_context ctx, Z3_solver sol) const;

	const DerefExpr* CastDerefExpr() const { return this; }

	bool Equals(const SymbolicExpr &e) const;

	const size_t managerIdx_;
	const size_t snapshotIdx_;

private:
	// The symbolic object corresponding to the dereference.
	SymbolicObject *object_;

	// A symbolic expression representing the symbolic address of this deref.
	const SymbolicExpr *addr_;


	Value_t ConcreteValueFromBytes(size_t from,
						 size_t size_, Value_t val) const;
};

}  // namespace crown

#endif  // DEREF_EXPRESSION_H__

