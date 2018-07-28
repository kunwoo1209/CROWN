// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_Z3_SOLVER_H__
#define BASE_Z3_SOLVER_H__

#include <map>
#include <vector>
#include <z3.h>

#include "base/basic_types.h"
#include "run_crown/symbolic_expression.h"

using std::map;
using std::vector;

namespace crown {

class Z3Solver {
 public:
	 static size_t path_cnt_;
	 static size_t path_sum_;
	 static double solving_time_;

	 static size_t no_reduction_sat_count;
	 static size_t no_reduction_unsat_count;
	 static size_t reduction_sat_count;
	 static size_t reduction_unsat_count;

	 static unsigned long long no_reduction_sat_formula_length;
	 static unsigned long long no_reduction_unsat_formula_length;
	 static unsigned long long  reduction_sat_formula_length;
	 static unsigned long long  reduction_unsat_formula_length;

	 static bool IncrementalSolve(const vector<Value_t>& old_soln,
			 const map<var_t,type_t>& vars,
			 const vector<const SymbolicExpr*>& constraints,
			 map<var_t,Value_t>* soln);  //doesn't using it.

	 static bool Solve(const map<var_t,type_t>& vars, const vector<unsigned long long>& values,
				const vector<unsigned char>& hs, const vector <unsigned char>& ls, const vector <SymbolicExpr *>& exprs,
			 const vector<const SymbolicExpr*>& constraints,
			 map<var_t,Value_t>* soln);
	 static long GetRunningTime(){ return Z3_running_time;};

 private:
	 static long Z3_running_time;
	 static long Z3_running_time2;
};

}  // namespace crown


#endif  // BASE_Z3_SOLVER_H__
