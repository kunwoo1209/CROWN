// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_SYMBOLIC_INTERPRETER_H__
#define BASE_SYMBOLIC_INTERPRETER_H__

#include <stdio.h>

#include <ext/hash_map>
#include <map>
#include <vector>

#include "base/basic_types.h"
#include "libcrown/symbolic_execution_writer.h"
#include "libcrown/symbolic_expression_writer.h"
#include "libcrown/symbolic_path_writer.h"
#include "libcrown/symbolic_memory_writer.h"
#include "libcrown/object_tracker_writer.h"
using std::map;
using std::vector;
using __gnu_cxx::hash_map;

namespace crown {

class SymbolicInterpreter {
	friend class Logger;
public:
	SymbolicInterpreter();
	explicit SymbolicInterpreter(const vector<Value_t>& input);

	void ClearStack(id_t id);
	void Load(id_t id, addr_t addr, Value_t value);
	void Deref(id_t id, addr_t addr, Value_t value);
	void Store(id_t id, addr_t addr);
	void Write(id_t id, addr_t addr);

	void ApplyUnaryOp(id_t id, unary_op_t op, Value_t value);
	void ApplyBinaryOp(id_t id, binary_op_t op, Value_t value);
	void ApplyBinPtrOp(id_t id, pointer_op_t op, size_t size, value_t addr);
	void ApplyCompareOp(id_t id, compare_op_t op, Value_t value);

	void Call(id_t id, function_id_t fid);
	void Return(id_t id);
	void HandleReturn(id_t id, Value_t value);

	void Branch(id_t id, branch_id_t bid, bool pred_value, unsigned int, const char *, const char*);
	void Alloc(id_t id, addr_t addr, size_t size);
	void Free(id_t id, addr_t addr);
	void Exit();

    Value_t NewInput(type_t ty, addr_t addr, char* var_name, int line, char* fname, unsigned long long oldValue = 0, unsigned char h = 0, unsigned char l = 0, unsigned char indexSize = 0);
    Value_t NewInput2(type_t ty, addr_t addr, Value_t val, char* var_name, int line, char* fname, unsigned long long oldValue = 0, unsigned char h = 0, unsigned char l = 0, unsigned char indexSize = 0);

    /*
     * comments written by Hyunwoo Kim (17.07.13)
     * symbolic variable name, its declared line and filename are added as parameters.
     * received information is saved in symbolic execution writer.
     */

	// Accessor for symbolic execution so far.
	const SymbolicExecutionWriter& execution() const { return ex_; }
	const ObjectTrackerWriter* tracker() const { return &obj_tracker_; }

private:
	struct StackElem {
		value_t concrete;
		fp_value_t fp_concrete;
		SymbolicExprWriter* expr;  // NULL to indicate concrete.
		type_t ty;
	};

	//Symbolic object
	ObjectTrackerWriter obj_tracker_;

	// Stack.
	vector<StackElem> stack_;

	// Symbolic Memory.
	SymbolicMemoryWriter mem_;

	// Is the top of the stack a function return value?
	bool return_value_;

	// The symbolic execution (program path and inputs).
	SymbolicExecutionWriter ex_;

	// Number of symbolic inputs so far.
	unsigned int num_inputs_;

	// Helper functions.
	inline void PushConcrete(Value_t value);
	inline void PushSymbolic(SymbolicExprWriter* expr, Value_t value);
	inline void ClearPredicateRegister();
	inline size_t sizeOfType(type_t ty, value_t val);
	inline void ScaleUpBy(bool isSigned, size_t size);
};

}  // namespace crown

#endif  // BASE_SYMBOLIC_INTERPRETER_H__
