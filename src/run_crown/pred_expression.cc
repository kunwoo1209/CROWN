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
#include "run_crown/pred_expression.h"

namespace crown {

PredExpr::PredExpr(ops::compare_op_t op, SymbolicExpr *l, SymbolicExpr *r, size_t s, Value_t v)
	: SymbolicExpr(s, v), compare_op_(op), left_(l), right_(r) {
//	std::cerr<<"new PredExpr "<<op<<" "<<v.floating<<"\n";
}

PredExpr::~PredExpr() {
	delete left_;
	delete right_;
}

PredExpr* PredExpr::Clone() const {
	return new PredExpr(compare_op_, left_->Clone(), right_->Clone(),
			size(), value());
}
#if 0
void PredExpr::AppendVars(set<var_t>* vars) const {
	left_->AppendVars(vars);
	right_->AppendVars(vars);
}

bool PredExpr::DependsOn(const map<var_t,type_t>& vars) const {
	return left_->DependsOn(vars) || right_->DependsOn(vars);
}
#endif
/* comments written by Hyunwoo Kim (17.07.14)
 * calling order was chagned to print path condition
 * in the form of infix.
 */

void PredExpr::AppendToString(string *s) const {
	s->append("(");
	left_->AppendToString(s);
	s->append(" ");
	s->append(kCompareOpStr[compare_op_]);
	s->append(" ");
	right_->AppendToString(s);
	s->append(")");
}

Z3_ast PredExpr::ConvertToSMT(Z3_context ctx, Z3_solver sol) const {
	Z3_ast e1 = left_->ConvertToSMT(ctx, sol);
	Z3_ast e2 = right_->ConvertToSMT(ctx, sol);
	Z3_sort_kind s1, s2;
	global_numOfOperator_++;
#if DEBUG
	string s;
	AppendToString(&s);
	std::cerr << "(cmp)** " << s << " **" << std::endl;
	std::cerr << Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e1)) << std::endl;
	std::cerr << Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e2)) << std::endl;
#endif
	s1 = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e1));
	s2 = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e2));

	assert((s1 == Z3_FLOATING_POINT_SORT && s2 == Z3_FLOATING_POINT_SORT) ||
			(s1 != Z3_FLOATING_POINT_SORT && s2 != Z3_FLOATING_POINT_SORT));

	if(s1 == Z3_FLOATING_POINT_SORT && s2 == Z3_FLOATING_POINT_SORT){
#ifdef DEBUG
		std::cerr<<"Z3_Float pred s1 "<<s1<<" "<<left_->value().floating<<"\n";
		std::cerr<<"Z3_Float pred s2 "<<s2<<" "<<right_->value().floating<<"\n";
#endif
		return ConvertToSMTforFP(ctx, sol, e1, e2);
	}else{
		return ConvertToSMTforBV(ctx, sol, e1, e2);
	}
}

Z3_ast PredExpr::ConvertToSMTforBV(Z3_context ctx, Z3_solver sol, 
		Z3_ast e1, Z3_ast e2) const {
	Z3_ast bve1, bve2;
	Z3_sort_kind s1, s2;
	s1 = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e1));
	s2 = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e2));

	bool is_signed = (ops::S_GT <= compare_op_ && compare_op_ <= ops::S_GE)?true:false;
	unsigned bve1_size, bve2_size;


	if (s1 == Z3_BV_SORT){
		bve1 = e1;
	}else if (s1 == Z3_BOOL_SORT){
		Z3_ast one, zero, cond;
		one = Z3_mk_numeral(ctx, "1", Z3_mk_bv_sort(ctx, left_->size() * 8));
		zero = Z3_mk_numeral(ctx, "0", Z3_mk_bv_sort(ctx, left_->size() * 8));
		cond = Z3_mk_eq(ctx, e1, Z3_mk_false(ctx));
		bve1 = Z3_mk_ite(ctx, cond, zero, one);
	}else{
		std::cerr << "Cmp. Op. " << compare_op_
			<< " expected Z3_BV_SORT or Z3_BOOL_SORT but encountered "
			<< Z3_get_symbol_string(ctx, Z3_get_sort_name(ctx, Z3_get_sort(ctx, e1)));
		assert(0);
	}


	if (s2 == Z3_BV_SORT){
		bve2 = e2;
	}else if (s2 == Z3_BOOL_SORT){
		Z3_ast one, zero, cond;
		one = Z3_mk_numeral(ctx, "1", Z3_mk_bv_sort(ctx, right_->size() * 8));
		zero = Z3_mk_numeral(ctx, "0", Z3_mk_bv_sort(ctx, right_->size() * 8));
		cond = Z3_mk_eq(ctx, e2, Z3_mk_false(ctx));
		bve2 = Z3_mk_ite(ctx, cond, zero, one);
	}else{
		std::cerr << "Cmp. Op. " << compare_op_
			<< " expected Z3_BV_SORT or Z3_BOOL_SORT but encountered "
			<< Z3_get_symbol_string(ctx, Z3_get_sort_name(ctx, Z3_get_sort(ctx, e2)));
		assert(0);
	}

	bve1_size = Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, bve1));
	bve2_size = Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, bve2));
	if (bve1_size < bve2_size){
		if (is_signed){
			bve1 = Z3_mk_sign_ext(ctx, bve2_size - bve1_size, bve1);
		}else{
			bve1 = Z3_mk_zero_ext(ctx, bve2_size - bve1_size, bve1);
		}
	}else if(bve1_size > bve2_size){
		if (is_signed){
			bve2 = Z3_mk_sign_ext(ctx, bve1_size - bve2_size, bve2);
		}else{
			bve2 = Z3_mk_zero_ext(ctx, bve1_size - bve2_size, bve2);
		}
	}

	e1 = bve1; e2 = bve2;
	assert(Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, e1)) == 
			Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, e2)));

	switch (compare_op_) {
		case ops::EQ:
			return Z3_mk_eq(ctx, e1, e2);
		case ops::NEQ:
			return Z3_mk_not(ctx, Z3_mk_eq(ctx, e1, e2));
		case ops::GT:
			return Z3_mk_bvugt(ctx, e1, e2);
		case ops::LE:
			return Z3_mk_bvule(ctx, e1, e2);
		case ops::LT:
			return Z3_mk_bvult(ctx, e1, e2);
		case ops::GE:
			return Z3_mk_bvuge(ctx, e1, e2);
		case ops::S_GT:
			return Z3_mk_bvsgt(ctx, e1, e2);
		case ops::S_LE:
			return Z3_mk_bvsle(ctx, e1, e2);
		case ops::S_LT:
			return Z3_mk_bvslt(ctx, e1, e2);
		case ops::S_GE:
			return Z3_mk_bvsge(ctx, e1, e2);
		default:
			fprintf(stderr, "Unknown comparison operator: %d\n", compare_op_);
			exit(1);
	}
}

Z3_ast PredExpr::ConvertToSMTforFP(Z3_context ctx, Z3_solver sol, 
		Z3_ast e1, Z3_ast e2) const {
	Z3_sort s1, s2;
	s1 = Z3_get_sort(ctx, e1);
	s2 = Z3_get_sort(ctx, e2);

	/*
	//This operates only when e2 is const
	long long unsigned int signi;
	long long int expon;
	int sign;
	Z3_fpa_get_numeral_significand_uint64(ctx,e2,&signi);
	Z3_fpa_get_numeral_exponent_int64(ctx,e2,&expon);
	Z3_fpa_get_numeral_sign(ctx,e2,&sign);
	if(s2 == Z3_mk_fpa_sort_single(ctx)){
	float RHSnum = setFloatByInts(sign,expon,signi);
	std::cout<<signi<<" "<<expon<<" "<<RHSnum<<" e2 floatNum\n";
	}
	 */
	if( s1 == s2 ){

	}else{
		std::cerr<<"Type: "<<left_->value().type<<" "<<right_->value().type<<std::endl;
		assert(0 && "Type sync (float vs double) needed");
		//Z3_mk_fpa_numeral_double(ctx,
		//			(double)left_->value().floating, Z3_get_sort(ctx, e2));
	}

#ifdef DEBUG
	printf("CompareOpOnFP %d\n",compare_op_);
#endif
	switch (compare_op_) {
		case ops::EQ:
			return Z3_mk_fpa_eq(ctx, e1, e2);
		case ops::NEQ:
			return Z3_mk_not(ctx, Z3_mk_fpa_eq(ctx, e1, e2));
		case ops::GT:
		case ops::S_GT:
			return Z3_mk_fpa_gt(ctx, e1, e2);
		case ops::LE:
		case ops::S_LE:
			return Z3_mk_fpa_leq(ctx, e1, e2);
		case ops::LT:
		case ops::S_LT:
			return Z3_mk_fpa_lt(ctx, e1, e2);
		case ops::GE:
		case ops::S_GE:
			return Z3_mk_fpa_geq(ctx, e1, e2);
		default:
			fprintf(stderr, "Unknown comparison operator: %d\n", compare_op_);
			exit(1);
	}
}


bool PredExpr::Equals(const SymbolicExpr &e) const {
	const PredExpr* c = e.CastPredExpr();
	return ((c != NULL)
			&& (compare_op_ == c->compare_op_)
			&& left_->Equals(*c->left_)
			&& right_->Equals(*c->right_));
}
}  // namespace crown
