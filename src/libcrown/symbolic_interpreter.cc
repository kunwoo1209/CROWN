// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <algorithm>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <utility>
#include <vector>
#include <string>

#include "base/basic_functions.h"
#include "libcrown/symbolic_interpreter.h"
#include "libcrown/atomic_expression_writer.h"
#include "libcrown/symbolic_expression_factory.h"
#include "libcrown/symbolic_object_writer.h"
#include "libcrown/object_tracker_writer.h"
#include "libcrown/logger.h"

using std::make_pair;
using std::swap;
using std::vector;
using std::string;

#ifdef DEBUG
#define IFDEBUG(x) x
#define IFDEBUG2(x) //IFDEBUG2 has buggy behavior
#else
#define IFDEBUG(x)
#define IFDEBUG2(x)
#endif


namespace crown {

typedef map<addr_t,SymbolicExprWriter*>::const_iterator ConstMemIt;

SymbolicInterpreter::SymbolicInterpreter()
	: ex_(true), num_inputs_(0) {
	  stack_.reserve(16);
}

SymbolicInterpreter::SymbolicInterpreter(
		const vector<Value_t>& input) : ex_(true) {
	stack_.reserve(16);
	num_inputs_ = 0;
	ex_.mutable_inputs()->assign(input.begin(), input.end());
}


void SymbolicInterpreter::ClearStack(id_t id) {
	IFDEBUG(fprintf(stderr, "clear\n"));
	vector<StackElem>::const_iterator it;
	for (it = stack_.begin(); it != stack_.end(); ++it) {
		//delete it->expr;
	}
	stack_.clear();
	return_value_ = false;
	IFDEBUG(Logger::DumpMemoryAndStack(mem_, stack_));
}


void SymbolicInterpreter::Load(id_t id, addr_t addr, Value_t value) {
	IFDEBUG(fprintf(stderr, "load 0x%lx %lld\n", addr, value.integral));

	SymbolicObjectWriter* obj = obj_tracker_.find(addr);
	SymbolicExprWriter* e = NULL;
	if(obj == NULL){
		// Load from main memory.
		e = mem_.read(addr, value);
	}else{
		e = obj->read(addr, value);
		obj_tracker_.updateDereferredStateOfRegion(*obj, addr, true);
	}

	IFDEBUG2({
	if (e) {
		string s;
		e->AppendToString(&s);
		fprintf(stderr, "load 0x%lx %lld : %s\n", addr, value.integral, s.c_str());
	}})
	PushSymbolic(e, value);

	IFDEBUG(Logger::DumpMemoryAndStack(mem_, stack_));
}

void SymbolicInterpreter::Deref(id_t id, addr_t addr, Value_t value) {
	IFDEBUG(fprintf(stderr, "deref %lu %lld\n", addr, value.integral));
	assert(stack_.size() > 0);

	SymbolicExprWriter* e = NULL;
	// Find right sym_obj from obj_tracker (set of sym_obj) 
	SymbolicObjectWriter* obj = obj_tracker_.find(addr);

	const StackElem& se = stack_.back();
	// Is this a symbolic dereference?
	if (obj && se.expr && !se.expr->IsConcrete()) {	
		e = SymbolicExprWriterFactory::NewDerefExprWriter(value, *obj, se.expr);
		obj_tracker_.updateDereferredStateOfRegion(*obj, addr, true);
		//Set the state of valid region(obj) as dereferenced
	} else {
		//delete se.expr;
	}
	stack_.pop_back();

	// If this is not a symbolic dereference, do a normal load.
	if (e == NULL) {
		if (obj == NULL) {
			// Load from main memory.
			e = mem_.read(addr, value);
		} else {
			// Load from a symbolic object.
			e = obj->read(addr, value);
			obj_tracker_.updateDereferredStateOfRegion(*obj, addr, true);
			//Set the state of valid region(obj) as dereferenced
		}
	}

	IFDEBUG2({
	if (e) {
		string s;
		e->AppendToString(&s);
		fprintf(stderr, "deref %lu %lld : %s\n", addr, value.integral, s.c_str());
	}})

	PushSymbolic(e, value);
}


void SymbolicInterpreter::Store(id_t id, addr_t addr) {
	IFDEBUG(fprintf(stderr, "store 0x%lx id %d\n", addr,id));
//	fprintf(stderr, "store 0x%lx id %d\n", addr,id);
	assert(stack_.size() > 0);

	const StackElem& se = stack_.back();

	IFDEBUG2({
		if (se.expr) {
			string s;
			se.expr->AppendToString(&s);
			fprintf(stderr, "store 0x%lx : %s\n", addr, s.c_str());
		}})

	// Is this a write to an object?
	SymbolicObjectWriter* obj = obj_tracker_.find(addr);

	if(obj == NULL){
		// Write to untracked region/object.
		if (se.expr && !se.expr->IsConcrete()) {
			//If the expr is symbolic
			mem_.write(addr, se.expr);
		} else {
			//delete se.expr;

			mem_.concretize(addr, sizeOfType(se.ty, se.concrete));
		}
	}else{
		bool isRegionDereferred = obj_tracker_.getDereferredStateOfRegion(addr);
		SymbolicObjectWriter* newObj;
		if(isRegionDereferred){
			newObj = obj_tracker_.storeAndGetNewObj(*obj, addr);
		}else{
			newObj = obj; //Use existing object(memory region)
		}

		// Write to tracked valid region/object.
		if (se.expr && !se.expr->IsConcrete()) {
			// Transfers ownership of se.expr.
			newObj->write(NULL, addr, se.expr);
		} else {
			SymbolicExprWriter *expr = SymbolicExprWriterFactory::NewConcreteExpr(
									Value_t(se.concrete, se.fp_concrete, se.ty));
      		newObj->write(NULL, addr, expr);
    	}
	}

	stack_.pop_back();
	IFDEBUG(Logger::DumpMemoryAndStack(mem_, stack_));

}

void SymbolicInterpreter::Write(id_t id, addr_t addr) {
	IFDEBUG(fprintf(stderr, "write 0x%lx id %d\n", addr,id));
	assert(stack_.size() > 1);

	const StackElem& dest = *(stack_.rbegin()+1);
	const StackElem& val = stack_.back();

	IFDEBUG2({
		if (val.expr) {
		string s;
		val.expr->AppendToString(&s);
		fprintf(stderr, "store %lu : %s\n", addr, s.c_str());
		}})

	// Is this a write to an object.
	SymbolicObjectWriter* obj = obj_tracker_.find(addr);

	if (obj == NULL) {
		// Normal store -- may be concretizing a symbolic write to an
		// untracked region/object.
		if (val.expr && !val.expr->IsConcrete()) {
			mem_.write(addr, val.expr);
		} else {
			mem_.concretize(addr, sizeOfType(val.ty, val.concrete));
			delete val.expr;
		}
	} else {
		bool isRegionDereferred = obj_tracker_.getDereferredStateOfRegion(addr);
		//Get the state of region. 
		//If the region is derefed, then store the obj and create new obj
		SymbolicObjectWriter* newObj;
		if(isRegionDereferred){
			newObj = obj_tracker_.storeAndGetNewObj(*obj, addr);
			//If the object is already dereferenced, 
			//then create new obj for updating
		}else{
			newObj = obj;
		}

		if (val.expr && !val.expr->IsConcrete()) {
			// Transfers ownership of dest.expr and val.expr.
			newObj->write(dest.expr, addr, val.expr);
		} else {
			SymbolicExprWriter *expr = SymbolicExprWriterFactory::NewConcreteExpr(
							Value_t(val.concrete, val.fp_concrete, val.ty));
			newObj->write(dest.expr, addr, expr);
		}
	}

	stack_.pop_back();
	stack_.pop_back();
}


void SymbolicInterpreter::ApplyUnaryOp(id_t id, unary_op_t op, Value_t value) {
	IFDEBUG(fprintf(stderr, "apply1 %d %lld\n", op, value.integral));
	assert(stack_.size() >= 1);
	StackElem& se = stack_.back();

	if (se.expr)
		se.expr = SymbolicExprWriterFactory::NewUnaryExprWriter(value, op, se.expr);

	se.ty = value.type;
	se.concrete = value.integral;
	se.fp_concrete = value.floating;
	IFDEBUG(Logger::DumpMemoryAndStack(mem_, stack_));
}


void SymbolicInterpreter::ApplyBinaryOp(id_t id, binary_op_t op,
		Value_t value) {
	IFDEBUG(fprintf(stderr, "apply2 %d %lld\n", op, value.integral));
	assert(stack_.size() >= 2);
	StackElem& a = *(stack_.rbegin()+1);
	StackElem& b = stack_.back();

	if (a.expr) {
		if (b.expr == NULL) {
			b.expr = SymbolicExprWriterFactory::NewConcreteExpr(Value_t(b.concrete, b.fp_concrete, b.ty));
		}
		a.expr = SymbolicExprWriterFactory::NewBinExprWriter(value, op, a.expr, b.expr);
	} else if (b.expr) {
		a.expr = SymbolicExprWriterFactory::NewConcreteExpr(Value_t(a.concrete, a.fp_concrete, a.ty));
		a.expr = SymbolicExprWriterFactory::NewBinExprWriter(value, op, a.expr, b.expr);
	}
	if (op == ops::CONCRETE){
		delete a.expr;
		a.expr = NULL;
	}

	a.concrete = value.integral;
	a.fp_concrete = value.floating;
	a.ty = value.type;

	stack_.pop_back();
	IFDEBUG(Logger::DumpMemoryAndStack(mem_, stack_));
}

/* 
 * For scaling up the integer number to right index addr
 * e.g. int a[1] means *cast<int *>(cast<long>(a) + sizeof(int)*1)
 */
void SymbolicInterpreter::ScaleUpBy(bool isSigned, size_t size) {
	assert(stack_.size() >= 1);
	StackElem& b = stack_.back();

	unary_op_t cast = isSigned ? ops::SIGNED_CAST : ops::UNSIGNED_CAST;
	type_t ty = isSigned ? types::LONG : types::U_LONG;

	// If necessary, adjust the symbolic value on the stack.
	if (b.expr) {
		// Cast b to be the same size as a signed/unsigned long.
	if (b.expr->size() != kSizeOfType[ty]) {
		b.expr = SymbolicExprWriterFactory::NewUnaryExprWriter(Value_t(b.concrete, 0, ty), cast, b.expr);
	}

	// Multiply b by size
	b.expr = SymbolicExprWriterFactory::NewBinExprWriter(Value_t(b.concrete * size, 0, ty),
							ops::MULTIPLY, b.expr, Value_t(size, 0, ty));
	}

	// Adjust the concrete value/type on the stack.
	b.concrete *= size;
	b.ty = ty;
}

void SymbolicInterpreter::ApplyBinPtrOp(id_t id, pointer_op_t op,
										size_t size, value_t value) {
	IFDEBUG(fprintf(stderr, "apply2ptr %d(%zu) %lld\n", op, size, value));
	assert(stack_.size() >= 2);
	StackElem& a = *(stack_.rbegin()+1);
	StackElem& b = stack_.back();

	type_t ty = (op == ops::SUBTRACT_PP) ? types::LONG : types::U_LONG;

	if (a.expr || b.expr) {
		// If operation is a pointer-int op, then scale b.
		if ((op != ops::SUBTRACT_PP) && (size > 1)) {
			bool isSigned = ((op == ops::S_ADD_PI) || (op == ops::S_SUBTRACT_PI));
			ScaleUpBy(isSigned, size);
		}

		// Apply the corresponding binary operation.
		if ((op == ops::ADD_PI) || (op == ops::S_ADD_PI)) {
		  ApplyBinaryOp(-1, ops::ADD, Value_t(
				static_cast<unsigned long>(a.concrete - b.concrete), 0, ty));
		} else {
		  ApplyBinaryOp(-1, ops::SUBTRACT, Value_t(
				static_cast<unsigned long>(a.concrete - b.concrete), 0, ty));
		}

		// If the operation is pointer-pointer subtraction, scale the result.
		if ((op == ops::SUBTRACT_PP) && (size > 1)) {
			// Currently only works for powers of 2.
			size_t log2 = 0;
			while ((size & 1) == 0) {
			log2++;
			size >>= 1;
		  }
		  assert(size == 1);

		  // TODO: Invert the remainder (module the correct power of two),
		  // so that we can use multiplication instead of division.

		  a.expr = SymbolicExprWriterFactory::NewBinExprWriter(Value_t(value, 0, ty), ops::S_SHIFT_R, a.expr, Value_t(log2, 0, ty));
		}

	} else {
		// The operation is concrete, so just pop the stack.
		stack_.pop_back();
	}

	a.concrete = value;
	a.fp_concrete = 0;
	a.ty = ty; //FIXME Could I have to distinguish long and pointer?
	// Stack has already been popped.
}


void SymbolicInterpreter::ApplyCompareOp(id_t id, compare_op_t op,
		Value_t value) {
	IFDEBUG(fprintf(stderr, "compare2 %d %lld\n", op, value.integral));
	assert(stack_.size() >= 2);
	StackElem& a = *(stack_.rbegin()+1);
	StackElem& b = stack_.back();

	if (a.expr) {
		if (b.expr == NULL) {
			b.expr = SymbolicExprWriterFactory::NewConcreteExpr(Value_t(b.concrete, b.fp_concrete, b.ty));
		}
		a.expr = SymbolicExprWriterFactory::NewPredExprWriter(value, op, a.expr, b.expr);
	} else if (b.expr) {
		a.expr = SymbolicExprWriterFactory::NewConcreteExpr(Value_t(a.concrete, a.fp_concrete, a.ty));
		a.expr = SymbolicExprWriterFactory::NewPredExprWriter(value, op, a.expr, b.expr);
	}
	a.concrete = value.integral;
	a.fp_concrete = value.floating;
	a.ty = value.type;

	stack_.pop_back();
	IFDEBUG(Logger::DumpMemoryAndStack(mem_, stack_));
}


void SymbolicInterpreter::Call(id_t id, function_id_t fid) {
	ex_.mutable_path()->Push(kCallId);
}


void SymbolicInterpreter::Return(id_t id) {
	ex_.mutable_path()->Push(kReturnId);

	// There is either exactly one value on the stack -- the current function's
	// return value -- or the stack is empty.
	assert(stack_.size() <= 1);

	return_value_ = (stack_.size() == 1);
}


void SymbolicInterpreter::HandleReturn(id_t id, Value_t value) {
	if (return_value_) {
		// We just returned from an instrumented function, so the stack
		// contains a single element -- the (possibly symbolic) return value.
		assert(stack_.size() == 1);
		return_value_ = false;
	} else {
		// We just returned from an uninstrumented function, so the stack
		// still contains the arguments to that function.  Thus, we clear
		// the stack and push the concrete value that was returned.
		ClearStack(-1);
		PushConcrete(value);
	}
}


void SymbolicInterpreter::Branch(id_t id, branch_id_t bid, bool pred_value,
			unsigned int lineno, const char *filename, const char *exp) {
	IFDEBUG(fprintf(stderr, "branch %d %d\n", bid, pred_value));
	assert(stack_.size() == 1);
	StackElem& se = stack_.back();

	if (se.expr) {
		if (se.expr->CastPredExprWriter()) {
			// If necessary, negate the expression.
			Value_t value = Value_t(!pred_value,(double)!pred_value,types::INT);
			if (!pred_value) {
				se.expr = SymbolicExprWriterFactory::NewUnaryExprWriter(value,
						ops::LOGICAL_NOT, se.expr);
			}

		} else {
			// Need to create a comparison.
			Value_t value = Value_t(1,1.0,types::INT);
			if (pred_value) {
				SymbolicExprWriter* zero = SymbolicExprWriterFactory::NewConcreteExpr(se.expr->size(), Value_t());
				se.expr = SymbolicExprWriterFactory::NewPredExprWriter(value,
						ops::NEQ, se.expr, zero);
			} else {
				SymbolicExprWriter* zero = SymbolicExprWriterFactory::NewConcreteExpr(se.expr->size(), Value_t());
				se.expr = SymbolicExprWriterFactory::NewPredExprWriter(value,
						ops::EQ, se.expr, zero);
			}
		}
	}

	ex_.mutable_path()->Push(bid, se.expr, pred_value, lineno, filename, exp);

	stack_.pop_back();
	IFDEBUG(Logger::DumpMemoryAndStack(mem_, stack_));
}

void SymbolicInterpreter::Alloc(id_t id, addr_t addr, size_t size) {
	//printf("id: %d\n",(int)id);
#ifndef DISABLE_DEREF
	obj_tracker_.addRegion(addr, size);
#endif
}

void SymbolicInterpreter::Free(id_t id, addr_t addr) {
#ifndef DISABLE_DEREF
	obj_tracker_.storeObj(addr);
	obj_tracker_.removeTrackingObj(addr);
#endif
}

void SymbolicInterpreter::Exit() {
#ifndef DISABLE_DEREF
	obj_tracker_.storeAllObjAndRemove();
#endif
}

/*
 * comments written by Hyunwoo Kim (17.07.13)
 * symbolic variable name, its declared line and filename are added as parameters.
 * received information is saved in symbolic execution writer. 
 */

Value_t SymbolicInterpreter::NewInput(type_t ty, addr_t addr, char* var_name, int line, char* fname, unsigned long long oldValue, unsigned char h, unsigned char l, unsigned char indexSize) {
	assert(ty != types::STRUCT);
	ex_.mutable_vars()->insert(make_pair(num_inputs_, ty));
  ex_.mutable_var_names()->push_back(string(var_name));
  ex_.mutable_locations()->push_back(Loc_t(string(fname),line));

	// Size and initial, concrete value.
	size_t size = kSizeOfType[ty];
	Value_t val = Value_t();

	//const addrs = ex_.addrs();

//	if((ty >= 15) && (std::find(addrs.begin(), addrs.end(), addr) != v.end()) ){		//bitfield, duplicated
	
	 if(num_inputs_ < ex_.values().size()) {
		ex_.mutable_values() -> at(num_inputs_) = oldValue;
		ex_.mutable_h() -> at(num_inputs_) = h;
		ex_.mutable_l() -> at(num_inputs_) = l;
		ex_.mutable_indexSize() -> at(num_inputs_) = indexSize;
	} else {
		ex_.mutable_values() -> push_back(oldValue);
		ex_.mutable_h() -> push_back(h);
		ex_.mutable_l() -> push_back(l);
		ex_.mutable_indexSize() -> push_back(indexSize);
	}

	if (num_inputs_ < ex_.inputs().size()) {
		if (ty < 15){
			val = ex_.inputs()[num_inputs_];
		} else {
			val.type = ty;
			val.integral = ((((ex_.inputs()[num_inputs_].integral) >> l ) & 1 ) << l) | (((oldValue >> h) << h) | (oldValue & ((1 << l) - 1)));
			ex_.mutable_inputs() -> at(num_inputs_) = val;
		}
	} else {
		// New inputs are initially zero.  (Could randomize instead.)
		val.integral = oldValue;
		val.type = ty;
		ex_.mutable_inputs()->push_back(val);
	}
	//assert(val.type==ty && "The type of input (file) is not match with symbolic variable incode");
	IFDEBUG(std::cerr<<"NewInput: "<<val.integral<<" "<<val.floating<<" ty: "<<val.type<<" "<<num_inputs_<<" "<<ex_.inputs().size()<<"\n");

    // To handle old value of bitfield inputs

    SymbolicObjectWriter* obj = obj_tracker_.find(addr);
    SymbolicExprWriter* e = NULL;
    if(obj == NULL){
        // Load from main memory.
        e = mem_.read(addr, val);
    }else{
        e = obj->read(addr, val);
        obj_tracker_.updateDereferredStateOfRegion(*obj, addr, true);
    }
    if (e == NULL){
        e = new AtomicExprWriter(size, val, num_inputs_);
    }
     if(num_inputs_ < ex_.exprs().size()) {
        ex_.mutable_exprs() -> at(num_inputs_) = e->Clone();
    } else {
        ex_.mutable_exprs() -> push_back(e->Clone());
    }

    if (ty < 15){
        // if the input is not bitfield or the input is the first encountered
    	mem_.write(addr, new AtomicExprWriter(size, val, num_inputs_));
    } else {
        //mem_.write(addr, e);
        mem_.write(addr, new AtomicExprWriter(size, val, num_inputs_));
	}
    
	num_inputs_++;
	return val;
}


Value_t SymbolicInterpreter::NewInput2(type_t ty, addr_t addr, Value_t init_val, char* var_name, int line, char* fname, unsigned long long oldValue, unsigned char h, unsigned char l, unsigned char indexSize) {
    assert(ty != types::STRUCT);
    ex_.mutable_vars()->insert(make_pair(num_inputs_, ty));
  ex_.mutable_var_names()->push_back(string(var_name));
  ex_.mutable_locations()->push_back(Loc_t(string(fname),line));

    // Size and initial, concrete value.
    size_t size = kSizeOfType[ty];
    Value_t val = Value_t();

    //const addrs = ex_.addrs();

//  if((ty >= 15) && (std::find(addrs.begin(), addrs.end(), addr) != v.end()) ){        //bitfield, duplicated

     if(num_inputs_ < ex_.values().size()) {
        ex_.mutable_values() -> at(num_inputs_) = oldValue;
        ex_.mutable_h() -> at(num_inputs_) = h;
        ex_.mutable_l() -> at(num_inputs_) = l;
        ex_.mutable_indexSize() -> at(num_inputs_) = indexSize;
    } else {
        ex_.mutable_values() -> push_back(oldValue);
        ex_.mutable_h() -> push_back(h);
        ex_.mutable_l() -> push_back(l);
        ex_.mutable_indexSize() -> push_back(indexSize);
    }

    if (num_inputs_ < ex_.inputs().size()) {
        if (ty < 15){
            val = ex_.inputs()[num_inputs_];
        } else {
            val.type = ty;
            val.integral = ((((ex_.inputs()[num_inputs_].integral) >> l ) & 1 ) << l) | (((oldValue >> h) << h) | (oldValue & ((1 << l) - 1)));
            ex_.mutable_inputs() -> at(num_inputs_) = val;
        }
    } else {
        // New input value is given 
        val = init_val;
        ex_.mutable_inputs()->push_back(val);
    }
    //assert(val.type==ty && "The type of input (file) is not match with symbolic variable incode");
    IFDEBUG(std::cerr<<"NewInput: "<<val.integral<<" "<<val.floating<<" ty: "<<val.type<<" "<<num_inputs_<<" "<<ex_.inputs().size()<<"\n");

    // To handle old value of bitfield inputs

    SymbolicObjectWriter* obj = obj_tracker_.find(addr);
    SymbolicExprWriter* e = NULL;
    if(obj == NULL){
        // Load from main memory.
        e = mem_.read(addr, val);
    }else{
        e = obj->read(addr, val);
        obj_tracker_.updateDereferredStateOfRegion(*obj, addr, true);
    }
    if (e == NULL){
        e = new AtomicExprWriter(size, val, num_inputs_);
    }
     if(num_inputs_ < ex_.exprs().size()) {
        ex_.mutable_exprs() -> at(num_inputs_) = e->Clone();
    } else {
        ex_.mutable_exprs() -> push_back(e->Clone());
    }

    if (ty < 15){
        // if the input is not bitfield or the input is the first encountered
        mem_.write(addr, new AtomicExprWriter(size, val, num_inputs_));
    } else {
        //mem_.write(addr, e);
        mem_.write(addr, new AtomicExprWriter(size, val, num_inputs_));
    }

    num_inputs_++;
    return val;
}


void SymbolicInterpreter::PushConcrete(Value_t value) {
	PushSymbolic(NULL, value);
}


void SymbolicInterpreter::PushSymbolic(SymbolicExprWriter* expr,
		Value_t value) {
	IFDEBUG(std::cerr<<"PushSymbolic: "<<value.integral<<" "<<value.floating<<"\n");
	stack_.push_back(StackElem());
	StackElem& se = stack_.back();
	se.expr = expr;
	se.ty = value.type;
	se.concrete = value.integral;
	se.fp_concrete = value.floating;
}


size_t SymbolicInterpreter::sizeOfType(type_t ty, value_t val) {
	if (ty != types::STRUCT) {
		return kSizeOfType[ty];
	} else {
		// For structs, the "concrete value" is the size of the struct.
		if (val != 0){
			return static_cast<size_t>(val);
		}else{
			return sizeof(int);
		}
	}
}

}  // namespace crown
