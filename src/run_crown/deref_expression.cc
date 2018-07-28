// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

/***
 * Author: Sudeep juvekar (sjuvekar@eecs.berkeley.edu)
 * 4/17/09
 */

#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <z3.h>
#include <iostream>

#include "base/basic_functions.h"
#include "run_crown/deref_expression.h"
#include "run_crown/symbolic_object.h"
#include "run_crown/object_tracker.h"
#include "run_crown/symbolic_expression_factory.h"

namespace crown {

DerefExpr::DerefExpr(SymbolicExpr *c, size_t managerIdx, size_t snapshotIdx,
		size_t s, Value_t v)
	: SymbolicExpr(s,v), managerIdx_(managerIdx), snapshotIdx_(snapshotIdx), addr_(c) { }
	


DerefExpr::DerefExpr(const DerefExpr& de)
	: SymbolicExpr(de.size(), de.value()),
	managerIdx_(de.managerIdx_), snapshotIdx_(de.snapshotIdx_),
	object_(de.object_), addr_(de.addr_->Clone())
{ }


DerefExpr::~DerefExpr() {
	delete addr_;
}

DerefExpr* DerefExpr::Clone() const {
	return new DerefExpr(*this);
}

void DerefExpr::AppendVars(set<var_t>* vars) const {
	ObjectTracker* tracker = global_tracker_;
	assert(tracker->snapshotManager().size() > managerIdx_);
	assert(tracker->snapshotManager()[managerIdx_]->size() > snapshotIdx_);
	SymbolicObject*	object = tracker->snapshotManager()[managerIdx_]->at(snapshotIdx_);
	addr_->AppendVars(vars);

	for(size_t i = 0; i < object->writes().size(); i++){
		SymbolicExpr *index = object->writes()[i].first;
		SymbolicExpr *exp = object->writes()[i].second;
		index->AppendVars(vars);
		exp->AppendVars(vars);
	}
}

bool DerefExpr::DependsOn(const map<var_t,type_t>& vars) const {
	ObjectTracker* tracker = global_tracker_;
	assert(tracker->snapshotManager().size() > managerIdx_);
	assert(tracker->snapshotManager()[managerIdx_]->size() > snapshotIdx_);
	SymbolicObject*	object = tracker->snapshotManager()[managerIdx_]->at(snapshotIdx_);

	for(size_t i = 0; i < object->writes().size(); i++){
		SymbolicExpr *index = object->writes()[i].first;
		SymbolicExpr *exp = object->writes()[i].second;
		bool res = index->DependsOn(vars) || exp->DependsOn(vars);
		if(res == true){
			return true;
		}
	}
	return addr_->DependsOn(vars);
}

void DerefExpr::AppendToString(string *s) const {
	char buff[92];
	s->append("(a!");
	sprintf(buff, "%d%d[ ", managerIdx_,snapshotIdx_);
	s->append(buff);
//	s->append(", ");
	addr_->AppendToString(s);
	s->append(" ])");
}


Z3_ast DerefExpr::ConvertToSMT(Z3_context ctx, Z3_solver sol) const {
#ifdef DEBUG
	printf("ConvertToSMT Deref %d %d\n",managerIdx_, snapshotIdx_);
#endif
	global_numOfOperator_++;
	size_t size_ = size();
	Value_t value_ = value();
	ObjectTracker* tracker = global_tracker_;

	assert(tracker->snapshotManager().size() > managerIdx_);
	assert(tracker->snapshotManager()[managerIdx_]->size() > snapshotIdx_);
	SymbolicObject*	object = tracker->snapshotManager()[managerIdx_]->at(snapshotIdx_);

	//If the snapshots are not created as a SMT, then create dummy SMT
	if(tracker->isASTInit == false){
		char name[24] = "t0";
		Z3_sort ty = Z3_mk_bv_sort(ctx, 32);
		Z3_ast con = Z3_mk_const(ctx, Z3_mk_string_symbol(ctx, name), ty);

		for(size_t iter = 0; iter < tracker->snapshotManager().size(); iter++){
			size_t objSize = tracker->snapshotManager()[iter]->size();
			for(size_t iter2 = 0; iter2 < objSize; iter2++){
				tracker->astManager()[iter]->push_back(con);
				tracker->isCreateAST()[iter]->push_back(false);
			}
		}
		tracker->isASTInit = true;
	}


	//size_t mem_length = object->writes().size();
	//Naming the uninterpreted function
	char c[32];
	sprintf(c, "a%ld",(unsigned long)this);

	//Bit-blast the address
	Z3_ast args_z3_f[1] = {addr_->ConvertToSMT(ctx, sol)};


	Z3_sort input_type =  Z3_mk_bv_sort(ctx, addr_->size() * 8);
	Z3_sort output_type;
	//Output values of a[x] needs to be registered. (e.g. double a[4])
	if(value_.type == types::FLOAT){
		output_type = Z3_mk_fpa_sort_single(ctx);
	}else if(value_.type == types::DOUBLE){
		output_type = Z3_mk_fpa_sort_double(ctx);
	}else{
		output_type = Z3_mk_bv_sort(ctx, size_*8);
	}	
	
	Z3_symbol array_symbol = Z3_mk_string_symbol(ctx, c);
	Z3_sort array_sort = Z3_mk_array_sort(ctx, input_type, output_type);
	Z3_ast array = Z3_mk_const(ctx, array_symbol, array_sort);

	//If the object isn't already created as SMT, then create it. 
	if(tracker->isCreateAST()[managerIdx_]->at(snapshotIdx_) == false){
		array = object->ConvertToSMT(ctx, sol, array, output_type);
		tracker->astManager()[managerIdx_]->at(snapshotIdx_) = array;
		tracker->isCreateAST()[managerIdx_]->at(snapshotIdx_) = true;
	}else{
		array = tracker->astManager()[managerIdx_]->at(snapshotIdx_);
	}

	//Dereference has to be in array (e.g. a[4] -> index can be 0,1,2,3)
	int sortSizeOfArray = Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, args_z3_f[0]));
	Z3_ast startAST = Z3_mk_int64(ctx, object->start(), Z3_mk_bv_sort(ctx, sortSizeOfArray));
	Z3_ast endAST = Z3_mk_int64(ctx, object->start()+object->size(), Z3_mk_bv_sort(ctx, sortSizeOfArray));
	Z3_ast startAST2 = Z3_mk_bvuge(ctx, args_z3_f[0], startAST);
	Z3_ast endAST2 = Z3_mk_bvult(ctx, args_z3_f[0], endAST);
	
	global_numOfExpr_+=2;
	//Assert that symbolic address is equal to at one of the values in domain
	Z3_solver_assert(ctx, sol,startAST2);
	Z3_solver_assert(ctx, sol,endAST2);

	//Return the application of the function to addr_
	Z3_ast tmp = Z3_mk_select(ctx, array, args_z3_f[0]);
	return tmp;
}

bool DerefExpr::Equals(const SymbolicExpr& e) const {
	const DerefExpr* d = e.CastDerefExpr();
	return ((d != NULL)
			&& addr_->Equals(*d->addr_)
			&& object_->Equals(*d->object_));
}

}  // namespace crown


