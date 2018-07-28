// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "base/basic_functions.h"
#include "run_crown/bin_expression.h"

namespace crown {

BinExpr::BinExpr(ops::binary_op_t op, SymbolicExpr *l, SymbolicExpr *r,
		size_t s, Value_t v)
	: SymbolicExpr(s,v), binary_op_(op), left_(l), right_(r) {
	}

BinExpr::~BinExpr() {
	delete left_;
	delete right_;
}

BinExpr* BinExpr::Clone() const {
	return new BinExpr(binary_op_, left_->Clone(), right_->Clone(),
			size(), value());
}

/* comments written by Hyunwoo Kim (17.07.14)
 * calling order was chagned to print path condition
 * in the form of infix.
 */
void BinExpr::AppendToString(string *s) const {
	s->append("(");
	left_->AppendToString(s);
	s->append(" ");
	s->append(kBinaryOpStr[binary_op_]);
	s->append(" ");
	right_->AppendToString(s);
	s->append(")");
}


Z3_ast BinExpr::ConvertToSMT(Z3_context ctx, Z3_solver sol) const {
	Z3_ast e1 = left_->ConvertToSMT(ctx, sol);
	Z3_ast e2 = right_->ConvertToSMT(ctx, sol);
	Z3_sort_kind s1, s2;
	global_numOfOperator_++;
#ifdef DEBUG
	string s;
	AppendToString(&s);
	std::cerr << "(bin)** " << s << " **" << std::endl;
	std::cerr << Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e1)) << std::endl;
	std::cerr << Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e2)) << std::endl;
#endif
	s1 = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e1));
	s2 = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e2));

	assert((s1 == Z3_FLOATING_POINT_SORT && s2 == Z3_FLOATING_POINT_SORT) ||
			(s1 != Z3_FLOATING_POINT_SORT && s2 != Z3_FLOATING_POINT_SORT));

	if(s1 == Z3_FLOATING_POINT_SORT && s2 == Z3_FLOATING_POINT_SORT){
#ifdef DEBUG
		std::cerr<<"Z3_Float bin s1 "<<s1<<" "<<left_->value().floating<<"\n";
		std::cerr<<"Z3_Float bin s2 "<<s2<<" "<<right_->value().floating<<"\n";
#endif
		return ConvertToSMTforFP(ctx, sol, e1, e2);
	}else{
		return ConvertToSMTforBV(ctx, sol, e1, e2);
	}
}


Z3_ast BinExpr::ConvertToSMTforBV(Z3_context ctx, Z3_solver sol,
		Z3_ast e1, Z3_ast e2) const {
	Z3_ast bve1, bve2;
	Z3_sort_kind s1, s2;
	s1 = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e1));
	s2 = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e2));

	unsigned bve1_size, bve2_size;
	bool is_signed = (binary_op_ == ops::S_SHIFT_R || binary_op_ == ops::S_MOD)?true:false;
	unsigned end = 0, start = 0;

	if (s1 == Z3_BV_SORT){
		bve1 = e1;
	}else if (s1 == Z3_BOOL_SORT){
		Z3_ast one, zero, cond;
		one = Z3_mk_numeral(ctx, "1", Z3_mk_bv_sort(ctx, left_->size() * 8));
		zero = Z3_mk_numeral(ctx, "0", Z3_mk_bv_sort(ctx, left_->size() * 8));
		cond = Z3_mk_eq(ctx, e1, Z3_mk_false(ctx));
		bve1 = Z3_mk_ite(ctx, cond, zero, one);
	}else{
		std::cerr << "Bin. Op. " << binary_op_ 
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
		std::cerr << "Bin. Op. " << binary_op_
			<< " expected Z3_BV_SORT or Z3_BOOL_SORT but encountered "
			<< Z3_get_symbol_string(ctx, Z3_get_sort_name(ctx, Z3_get_sort(ctx, e2)));
		assert(0);
	}


	bve1_size = Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, bve1));
	bve2_size = Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, bve2));
	if (binary_op_ != ops::CONCAT){
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
	}
	e1 = bve1; e2 = bve2;
#ifdef DEBUG
	std::cerr << "bve1: " << bve1 << ", bve2: " << bve2 << std::endl;
	std::cerr << "e1: " << Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, e1)) << ", e2: " 
		<< Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, e2)) << std::endl;
#endif

	switch (binary_op_) {
		case ops::ADD:
			return Z3_mk_bvadd(ctx, e1, e2);
		case ops::SUBTRACT:
			return Z3_mk_bvsub(ctx, e1, e2);
		case ops::MULTIPLY:
			return Z3_mk_bvmul(ctx, e1, e2);
			break;
		case ops::SHIFT_L:{
							  return Z3_mk_bvshl(ctx, e1, e2);
							  break;}
		case ops::SHIFT_R:{
							  return Z3_mk_bvlshr(ctx, e1, e2);
							  break;}
		case ops::S_SHIFT_R:{
								return Z3_mk_bvashr(ctx, e1, e2);
								break;}
		case ops::BITWISE_AND:
							return Z3_mk_bvand(ctx, e1, e2);
		case ops::BITWISE_OR:
							return Z3_mk_bvor(ctx, e1, e2);
		case ops::BITWISE_XOR:
							return Z3_mk_bvxor(ctx, e1, e2);
		case ops::CONCAT:
							return Z3_mk_concat(ctx, e1, e2);
		case ops::EXTRACT:
							// Extract the i-th, i+1-th, ..., i+size()-th least significant bytes.
							// (Assumption: right_ is concrete.)
							start = 8 * right_->value().integral;
							end = start + 8*size() - 1;
							return Z3_mk_extract(ctx, end, start, e1);
		case ops::DIV:
							return Z3_mk_bvudiv(ctx, e1, e2);
		case ops::S_DIV:
							return Z3_mk_bvsdiv(ctx, e1, e2);        
		case ops::MOD:
							return Z3_mk_bvurem(ctx, e1, e2);
		case ops::S_MOD:
							return Z3_mk_bvsrem(ctx, e1, e2);

		default:
							fprintf(stderr, "Unknown/unhandled binary operator: %d\n", binary_op_);
							exit(1);
	}

}


Z3_ast BinExpr::ConvertToSMTforFP(Z3_context ctx, Z3_solver sol,
		Z3_ast e1, Z3_ast e2) const {
	//Z3_sort rm_sort = Z3_mk_fpa_rounding_mode_sort(ctx);
	//Z3_symbol s_rm = Z3_mk_string_symbol(ctx, "rm");
	//Z3_ast rm = Z3_mk_const(ctx, s_rm, rm_sort);
	Z3_ast rm = Z3_mk_fpa_round_nearest_ties_to_even(ctx);
	Z3_sort s1, s2;
	s1 = Z3_get_sort(ctx, e1);
	s2 = Z3_get_sort(ctx, e2);

	if( s1 == s2 ){

	}else{
		std::cerr<<"Type: "<<left_->value().type<<" "<<right_->value().type<<std::endl;
		assert(0 && "Type sync (float vs double) needed");
	}

#ifdef DEBUG
	printf("BinOpOnFP %d\n",binary_op_);
#endif
	switch (binary_op_) {
		case ops::ADD:
			return Z3_mk_fpa_add(ctx, rm, e1, e2);
		case ops::SUBTRACT:
			return Z3_mk_fpa_sub(ctx, rm, e1, e2);
		case ops::MULTIPLY:
			return Z3_mk_fpa_mul(ctx, rm, e1, e2);
		case ops::DIV:
		case ops::S_DIV:
			return Z3_mk_fpa_div(ctx, rm, e1, e2);
		case ops::MOD:
		case ops::S_MOD:
			return Z3_mk_fpa_rem(ctx, e1, e2);
		case ops::SHIFT_L:
		case ops::SHIFT_R:
		case ops::S_SHIFT_R:
		case ops::BITWISE_AND:
		case ops::BITWISE_OR:
		case ops::BITWISE_XOR:
			assert("Floating point doesn't support bitwise operators.\n");
			exit(1);
			//FIXME Z3_mk_fpa_fma need?
		default:
			fprintf(stderr, "Unknown/unhandled binary operator: %d\n", binary_op_);
			exit(1);
	}
}

bool BinExpr::Equals(const SymbolicExpr &e) const {
	const BinExpr* b = e.CastBinExpr();
	return ((b != NULL)
			&& (binary_op_ == b->binary_op_)
			&& left_->Equals(*b->left_)
			&& right_->Equals(*b->right_));
}
}  // namespace crown
