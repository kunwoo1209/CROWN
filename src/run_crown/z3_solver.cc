// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <assert.h>
#include <queue>
#include <set>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <z3.h>
#include <sys/time.h>
#include "run_crown/z3_solver.h"
#include "base/basic_functions.h"
#include "run_crown/symbolic_expression.h"

//#define DEBUG
#ifdef DEBUG
#define IFDEBUG(x) x
#else
#define IFDEBUG(x)
#endif

typedef unsigned int var_t;
using std::make_pair;
using std::queue;
using std::set;
map<var_t,Z3_ast> x_decl;
static inline long myclock(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
size_t crown::Z3Solver::path_cnt_ = 0;
size_t crown::Z3Solver::path_sum_ = 0;
double crown::Z3Solver::solving_time_  = 0;
size_t crown::Z3Solver::reduction_sat_count = 0;
size_t crown::Z3Solver::reduction_unsat_count = 0;
unsigned long long crown::Z3Solver::reduction_sat_formula_length = 0;
unsigned long long crown::Z3Solver::reduction_unsat_formula_length = 0;

namespace crown {

typedef vector<const SymbolicExpr*>::const_iterator PredIt;

long Z3Solver::Z3_running_time = 0;
long Z3Solver::Z3_running_time2 = 0;
bool Z3Solver::IncrementalSolve(const vector<Value_t>& old_soln,
		const map<var_t,type_t>& vars,
		const vector<const SymbolicExpr*>& constraints,
		map<var_t,Value_t>* soln) {
	set<var_t> tmp;
	typedef set<var_t>::const_iterator VarIt;
	long t2 = myclock(), dt;
	// Build a graph on the variables, indicating a dependence when two
	// variables co-occur in a symbolic predicate.
	vector< set<var_t> > depends(vars.size());
	for (PredIt i = constraints.begin(); i != constraints.end(); ++i) {
		tmp.clear();
		(*i)->AppendVars(&tmp);
		for (VarIt j = tmp.begin(); j != tmp.end(); ++j) {
			depends[*j].insert(tmp.begin(), tmp.end());
		}
	}

	// Initialize the set of dependent variables to those in the constraints.
	// (Assumption: Last element of constraints is the only new constraint.)
	// Also, initialize the queue for the BFS.
	map<var_t,type_t> dependent_vars;
	vector<unsigned long long> dependent_values; //TODO?: need to insert
	vector<unsigned char> dependent_h;
	vector<unsigned char> dependent_l;
    vector<SymbolicExpr *> dependent_exprs;
	queue<var_t> Q;
	tmp.clear();
	constraints.back()->AppendVars(&tmp);
	for (VarIt j = tmp.begin(); j != tmp.end(); ++j) {
		dependent_vars.insert(*vars.find(*j));
		Q.push(*j);
	}

	// Run the BFS.
	while (!Q.empty()) {
		var_t i = Q.front();
		Q.pop();
		for (VarIt j = depends[i].begin(); j != depends[i].end(); ++j) {
			if (dependent_vars.find(*j) == dependent_vars.end()) {
				Q.push(*j);
				dependent_vars.insert(*vars.find(*j));
			}
		}
	}

	// Generate the list of dependent constraints.
	vector<const SymbolicExpr*> dependent_constraints;
	for (PredIt i = constraints.begin(); i != constraints.end(); ++i) {
		if ((*i)->DependsOn(dependent_vars))
			dependent_constraints.push_back(*i);
	}

	soln->clear();
	if (Solve(dependent_vars,dependent_values, dependent_h, dependent_l, dependent_exprs, dependent_constraints, soln)) {
		// Merge in the constrained variables.
		for (PredIt i = constraints.begin(); i != constraints.end(); ++i) {
			(*i)->AppendVars(&tmp);
		}
		for (set<var_t>::const_iterator i = tmp.begin(); i != tmp.end(); ++i) {
			if (soln->find(*i) == soln->end()) {
				soln->insert(make_pair(*i, old_soln[*i]));
			}
		}
		dt = myclock() - t2;
		Z3_running_time2 += dt;
		return true;
	}
	dt = myclock() - t2;
	Z3_running_time2 += dt;
	return false;
}

bool Z3Solver::Solve(const map<var_t,type_t>& vars, const vector<unsigned long long>& values,
		const vector<unsigned char>& hs, const vector<unsigned char> & ls,
        const vector<SymbolicExpr*>& exprs,
		const vector<const SymbolicExpr*>& constraints,
		map<var_t,Value_t>* soln) {
	long t, dt;
	t = myclock();
	//clock_t clk = clock();
	global_numOfExpr_ = 0;
	global_numOfVar_ = 0;
	global_numOfOperator_ = 0;

	typedef map<var_t,type_t>::const_iterator VarIt;
	Z3_config cfg = Z3_mk_config();
	Z3_set_param_value(cfg, "MODEL", "true");
	//    Z3_set_param_value(cfg, "TYPE_CHECK", "false");
	//    Z3_set_param_value(cfg, "WELL_SORTED_CHECK", "false");
	Z3_context ctx = Z3_mk_context(cfg);
	Z3_solver sol = Z3_mk_solver(ctx);
	Z3_del_config(cfg);
	assert(ctx);
	
	// Variable declarations.
	for (VarIt i = vars.begin(); i != vars.end(); ++i){
		char name[24];
		sprintf(name, "x%u", i->first);
		size_t size = 8 * kSizeOfType[i->second];
		Z3_sort ty;
		unsigned long long oldValue = values[i->first];
        SymbolicExpr *oldExpr = exprs[i->first];

		
		if(i->second == types::FLOAT){
			ty = Z3_mk_fpa_sort_single(ctx);
		}else if(i->second == types::DOUBLE){
			ty = Z3_mk_fpa_sort_double(ctx);
		}else{
			ty = Z3_mk_bv_sort(ctx, size);
		}
#ifdef DEBUG
		printf("name : %s\n", name);
		printf("oldv : %d\n", oldValue);
#endif
		x_decl[i->first] = Z3_mk_const(ctx, Z3_mk_string_symbol(ctx, name), ty);

		char oldName[24];
		if(i->first != 0){
			sprintf(oldName,"x%u", i->first - 1);
		}

		if( (i->second) >= 15 /*it is bitfield */ ){
			unsigned char l = ls[i->first]; 	//	0 <= l < size
			unsigned char h = hs[i->first];	//  0 < h <= size

			if (l > 0) {
			Z3_ast preserveLowBits;		
			unsigned long long lowMask = (1 << l) - 1;
			char lowMaskstr[12];
			char oldValuestr[12];
			sprintf(lowMaskstr, "%llu",lowMask);
			sprintf(oldValuestr, "%llu",oldValue);

			Z3_sort bv_sort = Z3_mk_bv_sort(ctx, size);
			
			Z3_ast lhs,rhs;
			// (x & lowMask) == (oldValue & lowMask)

			Z3_ast lowMaskNumeral = Z3_mk_numeral(ctx, lowMaskstr, bv_sort);
            Z3_ast oldExprAst = oldExpr->ConvertToSMT(ctx, sol);
#ifdef DEBUG
            std::cerr << "OldExpr: " << Z3_ast_to_string(ctx, oldExprAst) << std::endl;
#endif
			lhs = Z3_mk_bvand(ctx, Z3_mk_const(ctx, Z3_mk_string_symbol(ctx, name), ty), lowMaskNumeral);

            unsigned int oldExprAstSize = Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, oldExprAst));
            if (oldExprAstSize > size){
                oldExprAst = Z3_mk_extract(ctx, size-1, 0, oldExprAst);
            }else if (oldExprAstSize < size){
                oldExprAst = Z3_mk_zero_ext(ctx, size-oldExprAstSize, oldExprAst);
            }

            rhs = Z3_mk_bvand(ctx, oldExprAst, lowMaskNumeral);

			preserveLowBits = Z3_mk_eq(ctx, lhs,rhs);
#ifdef DEBUG
            std::cerr << Z3_ast_to_string(ctx, preserveLowBits) << std::endl;
#endif    
			Z3_solver_assert(ctx,sol,preserveLowBits);
			}

			if (h < size) {
			Z3_ast preserveHighBits;	// (x & highMask) == (oldValue & highMask)
			unsigned long long highMask = (-1 << h);
			char highMaskstr[12];
			char oldValuestr[12];
			sprintf(highMaskstr, "%llu", highMask);
			sprintf(oldValuestr, "%llu", oldValue);

			Z3_sort bv_sort = Z3_mk_bv_sort(ctx, size);
			
			Z3_ast highMaskNumeral = Z3_mk_numeral(ctx, highMaskstr, bv_sort);
			Z3_ast rhs, lhs;
            Z3_ast oldExprAst = oldExpr->ConvertToSMT(ctx, sol);

#ifdef DEBUG
            std::cerr << "OldExpr: " << Z3_ast_to_string(ctx, oldExprAst) << std::endl;
#endif

			lhs = Z3_mk_bvand(ctx, Z3_mk_const(ctx, Z3_mk_string_symbol(ctx, name), ty), highMaskNumeral);
            unsigned int oldExprAstSize = Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, oldExprAst));
            if (oldExprAstSize > size){
                oldExprAst = Z3_mk_extract(ctx, size-1, 0, oldExprAst);
            }else if (oldExprAstSize < size){
                oldExprAst = Z3_mk_zero_ext(ctx, size-oldExprAstSize, oldExprAst);
            }
            rhs = Z3_mk_bvand(ctx, oldExprAst, highMaskNumeral);
            
			preserveHighBits = Z3_mk_eq(ctx, lhs, rhs);
#ifdef DEBUG
            std::cerr << Z3_ast_to_string(ctx, preserveHighBits) << std::endl;
#endif

			Z3_solver_assert(ctx, sol, preserveHighBits);
			}
		}

		if(i->second == types::FLOAT || i->second == types::DOUBLE){
			Z3_ast removeNanAndInf;
			removeNanAndInf = Z3_mk_not(ctx, Z3_mk_fpa_is_nan(ctx, Z3_mk_const(ctx, Z3_mk_string_symbol(ctx, name), ty)));
#ifdef DEBUG
            std::cerr << Z3_ast_to_string(ctx, removeNanAndInf) << std::endl;
#endif
			Z3_solver_assert(ctx, sol, removeNanAndInf);
#ifdef DEBUG
            std::cerr << Z3_ast_to_string(ctx, removeNanAndInf) << std::endl;
#endif
			removeNanAndInf = Z3_mk_not(ctx, Z3_mk_fpa_is_infinite(ctx, Z3_mk_const(ctx, Z3_mk_string_symbol(ctx, name), ty)));
			Z3_solver_assert(ctx, sol, removeNanAndInf);
		}

		assert(x_decl[i->first]);
	}

#ifdef DEBUG
	string s = "";
	global_tracker_->AppendToString(&s);
	for(size_t j = 0; j < constraints.size(); j++) {
		char buf[10];
		sprintf(buf, "%lu", j+1);
		s += "(";
		s += buf;
		s += ") ";
		constraints[j]->AppendToString(&s);
		s.push_back('\n');
	}
	fprintf(stderr, "%s\n", s.c_str());
#endif

	//Z3_ast* constraints_ast = new Z3_ast[constraints.size()];

	for (PredIt i = constraints.begin(); i != constraints.end(); ++i) {
		const SymbolicExpr& se = **i;
		global_numOfExpr_++;
		dt = myclock() - t;
		Z3_running_time += dt;

		Z3_ast e = se.ConvertToSMT(ctx, sol);
		t = myclock();
		//		std::cerr << "(" << (i - constraints.begin())+1 << ") " << Z3_ast_to_string(ctx, e) << std::endl;
#ifdef DEBUG
		std::cerr << "(" << (i - constraints.begin())+1 << ") " << Z3_ast_to_string(ctx, e) << std::endl;
#endif
		Z3_solver_assert(ctx, sol,e);
		//		constraints_ast[i-constraints.begin()] = e;
	}

	IFDEBUG(std::cerr << "Start evalution"<< std::endl);
	//	std::cout<<"ConvertToSMT time "<<((double)clock() - clk)/CLOCKS_PER_SEC<<" constraint size "<<(int)(constraints.end() - constraints.begin() +1)<<" op "<<global_numOfOperator_<<" var "<<global_numOfVar_<<" clause "<<global_numOfExpr_<<std::endl;
	Z3_lbool result = Z3_solver_check(ctx, sol);
	Z3_model model = 0;
	//	std::cout<<"Z3 solve time "<<((double)clock() - clk)/CLOCKS_PER_SEC<<std::endl;
	IFDEBUG(std::cerr << "End evalution"<< std::endl);

	switch(result){
		case Z3_L_FALSE:
			Z3Solver::reduction_unsat_formula_length += constraints.size();
			Z3Solver::reduction_unsat_count++;
#ifdef DEBUG
            std::cerr << "UNSAT" << std::endl;
#endif
			break;
		case Z3_L_UNDEF:
			break;
		case Z3_L_TRUE:
			model = Z3_solver_get_model(ctx, sol);
			assert(model);
			for (VarIt i = vars.begin(); i != vars.end(); ++i) {
				Value_t val = Value_t();
				val.type = i->second;
				Z3_ast v;
				assert(Z3_model_eval(ctx, model, x_decl[i->first], Z3_TRUE, &v));

				Z3_sort_kind v_kind = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, v));
				if(v_kind == Z3_FLOATING_POINT_SORT){
					long long unsigned int signi;
					long long int expon;
					int sign;
					Z3_fpa_get_numeral_significand_uint64(ctx,v,&signi);
					Z3_fpa_get_numeral_exponent_int64(ctx,v,&expon);
					Z3_fpa_get_numeral_sign(ctx,v,&sign);

					if(Z3_get_sort(ctx, v) == Z3_mk_fpa_sort_single(ctx)){
						val.floating = setFloatByInts(sign,expon,signi);
						val.type = types::FLOAT;
					}else if(Z3_get_sort(ctx, v) == Z3_mk_fpa_sort_double(ctx)){
						val.floating = setDoubleByInts(sign,expon,signi);
						val.type = types::DOUBLE;
					}
#ifdef DEBUG
					std::cerr<<"Solved floating value: "<<signi<<" "<<expon<<" "<<val.floating<<" SolvedValue\n";
#endif
				}else{
					Z3_get_numeral_int64(ctx, v, &val.integral);
					val.type = i->second; 
					IFDEBUG(std::cerr<<"Solved int Value: "
							<<val.integral<<" ty: "<<val.type<<"\n");
				}
				IFDEBUG(std::cerr<<Z3_ast_to_string(ctx,v)
						<<" SolvedValue ty: "<<val.type<<"\n");
				soln->insert(make_pair(i->first, val));
				// Z3_del_model(ctx, model);
			}
			Z3Solver::reduction_sat_formula_length += constraints.size();
			Z3Solver::reduction_sat_count++;
#ifdef DEBUG
            std::cerr << "SAT" << std::endl;
#endif
			break;
	}
	Z3_del_context(ctx);
	Z3_reset_memory();

	dt = myclock() - t;
	Z3_running_time += dt;

	return (result == Z3_L_TRUE);
}

}  // namespace crown

