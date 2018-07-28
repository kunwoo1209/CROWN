// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <assert.h>
#include <z3.h>
#include "run_crown/symbolic_object.h"
#include "run_crown/symbolic_expression_factory.h"

#define OPT2METHOD

using namespace std;

namespace crown {

SymbolicObject::SymbolicObject(addr_t start, size_t size, 
				size_t managerIdx, size_t snapshotIdx)
	: start_(start), size_(size), managerIdx_(managerIdx),
		snapshotIdx_(snapshotIdx), writes_() { }


SymbolicObject::SymbolicObject(const SymbolicObject &obj)
	: start_(obj.start_), size_(obj.size_),
		managerIdx_(obj.managerIdx_), snapshotIdx_(obj.snapshotIdx_),
	mem_(obj.mem_), writes_(obj.writes_) {
	for (vector<Write>::iterator it = writes_.begin(); 
			it != writes_.end(); ++it) {
		it->first = it->first->Clone();
		it->second = it->second->Clone();
	}
}


SymbolicObject::~SymbolicObject() {
	for (vector<Write>::iterator it = writes_.begin(); 
			it != writes_.end(); ++it) {
		delete it->first;
		delete it->second;
	}
}

void SymbolicObject::AppendToString(string *s) const {
	char buff[92];
	sprintf(buff, "(let a!%d%d (a%d%d", managerIdx_,snapshotIdx_,managerIdx_,snapshotIdx_);
	s->append(buff);
	
	for(unsigned int i=0;i<writes_.size();i++){
		s->append("\n\t{");
		writes_[i].first->AppendToString(s);
		s->append(" <- ");
		writes_[i].second->AppendToString(s);
		s->append("}");
	}
	s->append(")\n");
}


SymbolicObject* SymbolicObject::Parse(istream& s) {
	addr_t start;
	size_t size, managerIdx, snapshotIdx;

	s.read((char*)&start, sizeof(start));
	s.read((char*)&size, sizeof(size));
	s.read((char*)&managerIdx, sizeof(managerIdx));
	s.read((char*)&snapshotIdx, sizeof(snapshotIdx));
	if (s.fail()) return NULL;
	assert(start + size > start);

	SymbolicObject* obj = new SymbolicObject(start, size, managerIdx, snapshotIdx);
	if (obj->ParseInternal(s)) {
		return obj;
	}

	// Failed.
	delete obj;
	return NULL;
}


bool SymbolicObject::ParseInternal(istream& s) {
	// Assumption: This object is empty, so we do not have to clear it out.
	assert(writes_.size() == 0);
	size_t size;
	if (!mem_.Parse(s)){
		return false;
	}
	s.read((char*)&size, sizeof(size));
	if (s.fail()) return false;

	for (size_t i = 0; i<size; i++){
		SymbolicExpr *expr1, *expr2;
		expr1 = SymbolicExpr::Parse(s);
		expr2 = SymbolicExpr::Parse(s);
		if (expr1 == NULL || expr2 == NULL){
			return false;
		}

		writes_.push_back(make_pair(expr1, expr2));
	}
	return true;
}


Z3_ast SymbolicObject::ConvertToSMT(Z3_context ctx, Z3_solver sol, Z3_ast array, Z3_sort output_type) const {
	Z3_sort_kind s1;
	s1 = Z3_get_sort_kind(ctx, output_type);

#ifdef OPT2METHOD
	if(snapshotIdx_ != 0){
		ObjectTracker* tracker = global_tracker_;
		if(tracker->isCreateAST()[managerIdx_]->at(snapshotIdx_-1) == false){
			SymbolicObject* object = tracker->snapshotManager()[managerIdx_]->at(snapshotIdx_-1);
			array = object->ConvertToSMT(ctx, sol, array, output_type);
			tracker->astManager()[managerIdx_]->at(snapshotIdx_-1) = array;
			tracker->isCreateAST()[managerIdx_]->at(snapshotIdx_-1) = true;
		}else{
			array = tracker->astManager()[managerIdx_]->at(snapshotIdx_-1);
		}
	}
#endif

	// Populate the function and assert
	for(size_t i = 0; i < writes().size(); i++){
		SymbolicExpr *index = writes()[i].first;
		SymbolicExpr *exp = writes()[i].second;
	
		/*  bit_vector_i is addr expr of a[i],
		 *  and expression_at_i is the value expr of a[i]
		 */
		Z3_ast bit_vector_i = index->ConvertToSMT(ctx, sol);
		Z3_ast expression_at_i = exp->ConvertToSMT(ctx, sol);
		
		int exp_size = Z3_get_bv_sort_size(ctx, Z3_get_sort(ctx, expression_at_i));
		int output_size = Z3_get_bv_sort_size(ctx, output_type);
		int isSigned = -1;
		if((int)exp->value().type < 10){
			isSigned = (int)exp->value().type;
		}
		
		//Match the bv sort of array and its element when the size is not correctly binded
		if(s1 != Z3_FLOATING_POINT_SORT){
			if(exp_size < output_size){
				if(isSigned){
					expression_at_i = Z3_mk_sign_ext(ctx, output_size - exp_size,expression_at_i);
				}else if(isSigned != -1){
					expression_at_i = Z3_mk_zero_ext(ctx, output_size - exp_size,expression_at_i);
				}
			}else if(exp_size > output_size){
				expression_at_i = Z3_mk_extract(ctx, output_size - 1, 0, expression_at_i);
			}
		}
		global_numOfOperator_++;
		array = Z3_mk_store(ctx, array, bit_vector_i, expression_at_i);
	}


	return array;
}

void SymbolicObject::Dump() const {
	mem_.Dump();
}
}  // namespace crown

