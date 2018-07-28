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
#include "run_crown/unary_expression.h"

#ifdef DEBUG
#define IFDEBUG(x) x
#else
#define IFDEBUG(x)
#endif

namespace crown {

UnaryExpr::UnaryExpr(ops::unary_op_t op, SymbolicExpr *c, size_t s, Value_t v)
  : SymbolicExpr(s, v), child_(c), unary_op_(op) { }

UnaryExpr::~UnaryExpr() {
	delete child_;
}

UnaryExpr* UnaryExpr::Clone() const {
	return new UnaryExpr(unary_op_, child_->Clone(), size(), value());
}

/* comments written by Hyunwoo Kim (17.07.13)
 * data type were decided to print in symbolic path.
 */

void UnaryExpr::AppendToString(string *s) const {
	if(unary_op_ != 3 && unary_op_ != 4)
		s->append(kUnaryOpStr[unary_op_]);

	if (unary_op_ == ops::SIGNED_CAST || unary_op_ == ops::UNSIGNED_CAST){
		char buff[32];
		
        switch(value().type){
			case -1:
				sprintf(buff, "(bool)");
				break;
			case 0:
				sprintf(buff, "(unsigned char)");
				break;
			case 1:
				sprintf(buff, "(char)");
				break;
			case 2:
				sprintf(buff, "(unsigned short)");
				break;
			case 3:
				sprintf(buff, "(short)");
				break;
			case 4:
				sprintf(buff, "(unsigned int)");
				break;
			case 5:
				sprintf(buff, "(int)");
				break;
			case 6:
				sprintf(buff, "(unsigned long)");
				break;
			case 7:
				sprintf(buff, "(long)");
				break;
			case 8:
				sprintf(buff, "(unsigned long long)");
				break;
			case 9:
				sprintf(buff, "(long long)");
				break;
			case 10:
				sprintf(buff, "(struct)");
				sprintf(buff, "[%d]", size()*8);
				break;
			case 11:
				sprintf(buff, "(pointer)");
				sprintf(buff, "[%d]", size()*8);
				break;
			case 12:
				sprintf(buff, "(float)");
				break;
			case 13:
				sprintf(buff, "(double)");
				break;
			case 14:
				sprintf(buff, "(long double)");
				break;
			default :
				sprintf(buff, "[%d]", size()*8);

		}
		s->append(buff);
	}
	s->append(" ");
	child_->AppendToString(s);
}

Z3_ast UnaryExpr::ConvertToSMT(Z3_context ctx, Z3_solver sol) const {
	Z3_ast e = child_->ConvertToSMT(ctx, sol);
	Z3_ast rm = Z3_mk_fpa_round_nearest_ties_to_even(ctx);
	global_numOfOperator_++;
	size_t child_size = 0;
	size_t curr_size = 0;


	IFDEBUG(printf("UnaryConvertToSmt %d\n",unary_op_));
	IFDEBUG(std::cout<<Z3_ast_to_string(ctx,e)<<"\n");

	switch (unary_op_) {
		case ops::NEGATE:{
			 Z3_sort_kind s = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e));
			 if(s == Z3_FLOATING_POINT_SORT){
				 return Z3_mk_fpa_neg(ctx, e);
			 }else{
				 return Z3_mk_bvneg(ctx, e);
			 }
		 }
		case ops::LOGICAL_NOT:{
			Z3_sort_kind s = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e));
			if (s == Z3_BV_SORT){
				Z3_ast cond, zero, t, f;
				zero = Z3_mk_numeral(ctx, "0", Z3_mk_bv_sort(ctx, child_->size() * 8));
				t = Z3_mk_true(ctx);
				f = Z3_mk_false(ctx);
				cond = Z3_mk_eq(ctx, e, zero);
				return Z3_mk_ite(ctx, cond, t, f);
				//FIXME else if(s == Z3_Float , DOUBLE. for !float val. 
			}else if (s == Z3_BOOL_SORT){
				return Z3_mk_not(ctx, e);
			}else{
				std::cerr << "LOGICAL_NOT expected Z3_BV_SORT or Z3_BOOL_SORT"
					<< " but encountered " 
					<< Z3_get_symbol_string(ctx, Z3_get_sort_name(ctx, Z3_get_sort(ctx, e)))
					<< std::endl;
				assert(0);
			}
		}
		case ops::BITWISE_NOT:
			return Z3_mk_bvnot(ctx, e);

		case ops::SIGNED_CAST:{
			  Z3_sort_kind s = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e));
			  if (s == Z3_BOOL_SORT){
				  Z3_ast cond, zero, one;
				  cond = Z3_mk_eq(ctx, e, Z3_mk_false(ctx));
				  zero = Z3_mk_numeral(ctx, "0", Z3_mk_bv_sort(ctx, 8*size()));
				  one = Z3_mk_numeral(ctx, "1", Z3_mk_bv_sort(ctx, 8*size()));
				  return Z3_mk_ite(ctx, cond, zero, one);
			  }else if(s == Z3_BV_SORT){
				  if(value().type == types::FLOAT){
					  //FIXME child int -> float //Check whether signed is right or not.
					  return Z3_mk_fpa_to_fp_signed(ctx,rm,e,Z3_mk_fpa_sort_single(ctx));
				  }else if(value().type == types::DOUBLE){
					  return Z3_mk_fpa_to_fp_signed(ctx,rm,e,Z3_mk_fpa_sort_double(ctx));
				  }else{
					  //std::cout<<"Check!"<<std::endl;
					  child_size = 8*child_->size();
					  curr_size = 8*size();
					  if(curr_size < child_size) { // Downcast: Extract the lowest order bits
						  return Z3_mk_extract(ctx, curr_size - 1, 0, e);
					  }else if(curr_size > child_size) { //Upcast: sign-extend
						  return Z3_mk_sign_ext(ctx, curr_size - child_size, e);
					  }else {
						  return e;
					  }
				  }
			  }else if(s == Z3_FLOATING_POINT_SORT){
				  //child float -> int(bv) .. using cur_size.Z3_mk_fpa_to_fp_signed
				  //child float -> double .. Z3_mk_fpa_to_fp_float
				  //child double -> float
				  if(child_->value().type == types::FLOAT){
					  if(value().type == types::DOUBLE){
						  return Z3_mk_fpa_to_fp_float(ctx,rm,e,Z3_mk_fpa_sort_double(ctx));
					  }else if(value().type < types::FLOAT){
						  curr_size = 8*size();
						  return Z3_mk_fpa_to_sbv(ctx, rm, e, curr_size);
					  }
				  }else if(child_->value().type == types::DOUBLE){
					  if(value().type == types::FLOAT){
						  return Z3_mk_fpa_to_fp_float(ctx,rm,e,Z3_mk_fpa_sort_single(ctx));
					  }else if(value().type < types::FLOAT){
						  curr_size = 8*size();
						  return Z3_mk_fpa_to_sbv(ctx, rm, e, curr_size);
					  }
				  }
				  if(value().type == child_->value().type){
					  return e;
				  }
				  std::cerr<<"Parent type: "<<value().type<<" Child type: "<<child_->value().type<<std::endl;
				  assert(0);
			  }else{
				  assert(0);
			  }
		}

		case ops::UNSIGNED_CAST:{
				Z3_sort_kind s = Z3_get_sort_kind(ctx, Z3_get_sort(ctx, e));
				if (s == Z3_BOOL_SORT){
					Z3_ast cond, zero, one;
					cond = Z3_mk_eq(ctx, e, Z3_mk_false(ctx));
					zero = Z3_mk_numeral(ctx, "0", Z3_mk_bv_sort(ctx, 8*size()));
					one = Z3_mk_numeral(ctx, "1", Z3_mk_bv_sort(ctx, 8*size()));
					return Z3_mk_ite(ctx, cond, zero, one);
				}else if(s == Z3_BV_SORT){
					if(value().type == types::FLOAT){
						//FIXME child int -> float //Check whether signed is right or not.
						return Z3_mk_fpa_to_fp_unsigned(ctx,rm,e,Z3_mk_fpa_sort_single(ctx));
					}else if(value().type == types::DOUBLE){
						return Z3_mk_fpa_to_fp_unsigned(ctx,rm,e,Z3_mk_fpa_sort_double(ctx));
					}else{
						//std::cout<<"Check2!"<<std::endl;
						child_size = 8*child_->size();
						curr_size = 8*size();
						if(curr_size < child_size) { // Downcast: Extract the lowest order bits
							return Z3_mk_extract(ctx, curr_size - 1, 0, e);
						} else if(curr_size > child_size) { //Upcast: In this case, append the child with zeroes
							return Z3_mk_zero_ext(ctx, curr_size - child_size, e);
						} else {
							return e;
						}
					}
				}else if(s == Z3_FLOATING_POINT_SORT){
					//child float -> int(bv) .. using cur_size.Z3_mk_fpa_to_fp_signed
					//child float -> double .. Z3_mk_fpa_to_fp_float
					//child double -> float
					if(child_->value().type == types::FLOAT){
						if(value().type == types::DOUBLE){
							return Z3_mk_fpa_to_fp_float(ctx,rm,e,Z3_mk_fpa_sort_double(ctx));
						}else if(value().type < types::FLOAT){
							curr_size = 8*size();
							return Z3_mk_fpa_to_ubv(ctx, rm, e, curr_size);
						}
					}else if(child_->value().type == types::DOUBLE){
						if(value().type == types::FLOAT){
							return Z3_mk_fpa_to_fp_float(ctx,rm,e,Z3_mk_fpa_sort_single(ctx));
						}else if(value().type < types::FLOAT){
							curr_size = 8*size();
							return Z3_mk_fpa_to_ubv(ctx, rm, e, curr_size);
						}//FIXME
					}
					if(value().type == child_->value().type){
						return e;
					}
					std::cerr<<"Parent type: "<<value().type<<" Child type: "<<child_->value().type<<std::endl;
					assert(0);
				}else{
					assert(0);
				}
			}
			default:
				fprintf(stderr, "Unknown unary operator: %d\n", unary_op_);
				exit(1);
		}

}

bool UnaryExpr::Equals(const SymbolicExpr &e) const {
	const UnaryExpr* u = e.CastUnaryExpr();
	return ((u != NULL)
			&& (unary_op_ == u->unary_op_)
			&& child_->Equals(*u->child_));
}
}  // namespace crown
