// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include "run_crown/atomic_expression.h"
#include "base/basic_types.h"
#include <iostream>
#include <cstdio>
#include "run_crown/symbolic_execution.h"

#include <vector>

typedef unsigned int var_t;
extern map<var_t,Z3_ast> x_decl;
/* comments written by Hyunwoo Kim (17.07.14)
 * global variable g_var_names is defined in symbolic_execution.cc
 * this variable is used to refer vector including variable name.
 */
extern std::vector<string> g_var_names;
namespace crown {

AtomicExpr::AtomicExpr(size_t size, Value_t val, var_t var)
	: SymbolicExpr(size, val), var_(var) { }

AtomicExpr* AtomicExpr::Clone() const {
	return new AtomicExpr(size(), value(), var_);
}

void AtomicExpr::AppendToString(string* s) const {
    s->append(g_var_names[var_]);
}

Z3_ast AtomicExpr::ConvertToSMT(Z3_context ctx, Z3_solver sol) const {
#ifdef DEBUG
	printf("ConvertToSMT Atomic %d\n",var_);
#endif
	global_numOfVar_++;
	return x_decl[var_];
}

bool AtomicExpr::Equals(const SymbolicExpr &e) const {
	const AtomicExpr* b = e.CastAtomicExpr();
	return (b != NULL) && (var_ == b->var_);
}

}  // namespace crown
