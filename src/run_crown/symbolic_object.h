// Copyright (c) 2009, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

// TODO: Implement Equals.

#ifndef BASE_SYMBOLIC_OBJECT_H__
#define BASE_SYMBOLIC_OBJECT_H__

#include <utility>
#include <vector>
#include <istream>
#include <ostream>

#include "base/basic_types.h"
#include "run_crown/symbolic_expression.h"
#include "run_crown/symbolic_memory.h"


namespace crown {

class SymbolicExpr;

class SymbolicObject {
private:
	typedef std::pair<SymbolicExpr*,SymbolicExpr*> Write;
public:
	SymbolicObject(addr_t start, size_t size, 
					size_t managerIdx, size_t snapshotIdx);
	SymbolicObject(const SymbolicObject& o);
	~SymbolicObject();


	//  void concretize(SymbolicExpr* sym_addr, addr_t addr, size_t n);

	bool Equals(const SymbolicObject& o) const { return false; }

	void AppendToString(string *s) const;
	static SymbolicObject* Parse(istream &s);

	SymbolicExpr* NewConstDerefExpr(Value_t val,
						const SymbolicObject& obj,addr_t addr);

	Z3_ast ConvertToSMT(Z3_context ctx, Z3_solver sol, Z3_ast array, Z3_sort output_type) const;

	const std::vector<Write>& writes() const{ return writes_;}

	//Debuggind
	void Dump() const;

	addr_t start() const { return start_; }
	addr_t end() const { return start_ + size_; }
	size_t size() const { return size_; }
	size_t managerIdx() const { return managerIdx_; }
	size_t snapshotIdx() const { return snapshotIdx_; }

private:
	bool ParseInternal(istream &s);

	const addr_t start_;
	const size_t size_;
	const size_t managerIdx_;
	const size_t snapshotIdx_; 
	//If you don't push it on snapshotManager yet, then do not access to the vector using this variable

	SymbolicMemory mem_;
	std::vector<Write> writes_;
};

} // namespace crown

#endif // BASE_SYMBOLIC_OBJECT_H__

