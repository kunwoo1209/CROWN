// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef ATOMIC_TYPE_H__
#define ATOMIC_TYPE_H__

#include <map>
#include <set>
#include <string>

#include "base/basic_types.h"
#include "run_crown/symbolic_expression.h"
using std::map;
using std::set;
using std::string;


namespace crown {

class AtomicExpr : public SymbolicExpr {
public:
	AtomicExpr(size_t size, Value_t val, var_t var);
	~AtomicExpr() { }

	void AppendVars(set<var_t>* vars) const {
		vars->insert(var_);
	}

	bool DependsOn(const map<var_t,type_t>& vars) const {
		return (vars.find(var_) != vars.end());
	}

	void AppendToString(string* s) const;
	AtomicExpr* Clone() const;

	bool IsConcrete() const { return false; }

	Z3_ast ConvertToSMT(Z3_context ctx, Z3_solver sol) const;

	const AtomicExpr* CastAtomicExpr() const { return this; }

	 bool Equals(const SymbolicExpr &e) const;

	 // Accessor.
	 var_t variable() const { return var_; }

private:
	 const var_t var_;
};

}  // namespace crown

#endif  // ATOMIC_TYPE_H__
