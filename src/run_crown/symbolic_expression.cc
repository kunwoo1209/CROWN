// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.
 
#include <assert.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <z3.h>

#include "base/basic_functions.h"
#include "run_crown/symbolic_expression.h"
#include "run_crown/unary_expression.h"
#include "run_crown/bin_expression.h"
#include "run_crown/pred_expression.h"
#include "run_crown/atomic_expression.h"
#include "run_crown/symbolic_object.h"
#include "run_crown/deref_expression.h"

#ifdef DEBUG
#define IFDEBUG(x) x
#else
#define IFDEBUG(x)
#endif

namespace crown {

long long int global_numOfExpr_ = 0;
long long int global_numOfOperator_ = 0;
long long int global_numOfVar_ = 0;

typedef map<var_t,value_t>::iterator It;
typedef map<var_t,value_t>::const_iterator ConstIt;

size_t SymbolicExpr::next = 0;
map <size_t, SymbolicExpr *>SymbolicExpr::read_table;

SymbolicExpr::~SymbolicExpr() { }

SymbolicExpr* SymbolicExpr::Clone() const {
	return new SymbolicExpr(size_, value_);
}

void SymbolicExpr::AppendToString(string* s) const {
	assert(IsConcrete());

	char buff[92];
	if(value().type == types::FLOAT ||value().type ==types::DOUBLE){
		std::ostringstream fp_out;
		fp_out<<value().floating;
		strcpy(buff, fp_out.str().c_str());
		s->append(buff);
	}else{
		if(value().integral < 0x40000000){
			sprintf(buff, "%lld", value().integral);
		}else{
			sprintf(buff, "0x%llx", value().integral);
		}
		s->append(buff);
	}
}

bool SymbolicExpr::Equals(const SymbolicExpr &e) const {
	return (e.IsConcrete()
			&& (value().integral == e.value().integral) //FIXME
			&& (size() == e.size()));
}

Z3_ast SymbolicExpr::ConvertToSMT(Z3_context ctx, Z3_solver sol) const {
	assert(size() <= sizeof(long double));
	IFDEBUG(std::cerr<<"ConvertOnSymExpr Value_t: "<<value().integral<<" "<<value().floating<<" ty: "<<value().type<<"\n");
	if(value().type == types::FLOAT){
		return Z3_mk_fpa_numeral_float(ctx, value().floating, Z3_mk_fpa_sort_single(ctx));
	}else if(value().type == types::DOUBLE){
		return Z3_mk_fpa_numeral_double(ctx, value().floating, Z3_mk_fpa_sort_double(ctx));
	}else{
		return Z3_mk_int64(ctx, value().integral, Z3_mk_bv_sort(ctx, 8*size()));
	}
}

SymbolicExpr* SymbolicExpr::Parse(istream& s) {
	Value_t val = Value_t();
	size_t size;
	size_t id;
	var_t var;

	SymbolicExpr *left, *right, *child;
	compare_op_t cmp_op_;
	binary_op_t bin_op_;
	unary_op_t un_op_;

	//SymbolicObject *obj;
	SymbolicExpr *addr;
	//unsigned char* bytes;
	global_numOfExpr_++;


	s.read((char*)&val, sizeof(Value_t));
	if (s.fail()) return NULL;
	s.read((char*)&size, sizeof(size_t));
	if (s.fail()) return NULL;
	s.read((char*)&id, sizeof(size_t));
	if (s.fail()) return NULL;

	char type_ = s.get();
	IFDEBUG(std::cerr<<"ParseInSymExpr: "<<val.type<<" "<<val.integral<<" "<<val.floating<<" size:"<<(size_t)size<<" id:"<<(size_t)id<<" nodeTy: "<<(int)type_<<std::endl);

	//Debug for floating point number as bin
	//IFDEBUG(std::cout<<*doubleToBinString(val.floating)<<std::endl);
	switch(type_) {
		case kBasicNodeTag:
			global_numOfVar_++;
			s.read((char*)&var, sizeof(var_t));
			if(s.fail()) return NULL;
//			if(read_table[id] != NULL) return read_table[id];
			read_table[id] = new AtomicExpr(size, val, var);
			return read_table[id];

		case kCompareNodeTag:
			global_numOfOperator_++;
			cmp_op_ = (compare_op_t)s.get();
			if (s.fail()) return NULL;
			left = Parse(s);
			right = Parse(s);
			if (!left || !right) {
				return NULL;
			}
//			if(read_table[id] != NULL) return read_table[id];
			read_table[id] = new PredExpr(cmp_op_, left, right, size, val);
			return read_table[id];

		case kBinaryNodeTag:
			global_numOfOperator_++;
			bin_op_ = (binary_op_t)s.get();
			if (s.fail()) return NULL;
			left = Parse(s);
			right = Parse(s);
			if (!left || !right) {
				return NULL;
			}
//			if(read_table[id] != NULL) return read_table[id];
			read_table[id] = new BinExpr(bin_op_, left, right, size, val);
			return read_table[id];

		case kUnaryNodeTag:
			global_numOfOperator_++;
			un_op_ = (unary_op_t)s.get();
			if (s.fail()) return NULL;
			child = Parse(s);
			if (child == NULL) return NULL;
//			if(read_table[id] != NULL) return read_table[id];
			read_table[id] = new UnaryExpr(un_op_, child, size, val);
			return read_table[id];

		case kDerefNodeTag:
			global_numOfOperator_++;
			size_t managerIdx, snapshotIdx;
			s.read((char*)&managerIdx, sizeof(size_t));
			s.read((char*)&snapshotIdx, sizeof(size_t));
			addr = SymbolicExpr::Parse(s);

			if (addr == NULL) { // Read has failed in expr::Parse
				return NULL;
			}

//			bytes = new unsigned char[obj->size()];
//			s.read((char*)bytes, obj->size());

			if (s.fail()) {
				delete addr;
				return NULL;
			}
//			if(read_table[id] != NULL){
//				return read_table[id];
//			}
			read_table[id] = new DerefExpr(addr, managerIdx, snapshotIdx, size, val);
			return read_table[id];

		case kConstNodeTag:
//				if(read_table[id] != NULL) return read_table[id];
			read_table[id] = new SymbolicExpr(size, val);
			return read_table[id];

		default:
			fprintf(stderr, "Unknown type of node: '%c(%d)'....exiting\n", type_,type_);
			assert(0);
	}
}


}  // namespace crown
