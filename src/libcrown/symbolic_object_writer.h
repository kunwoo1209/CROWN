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

#ifndef BASE_SYMBOLIC_OBJECT_WRITER_H__
#define BASE_SYMBOLIC_OBJECT_WRITER_H__

#include <utility>
#include <vector>
#include <istream>
#include <ostream>

#include "base/basic_types.h"
#include "libcrown/symbolic_expression_writer.h"
#include "libcrown/symbolic_memory_writer.h"


namespace crown {
typedef std::pair<SymbolicExprWriter*,SymbolicExprWriter*> Write;

class SymbolicExprWriter;

class SymbolicObjectWriter {
public:
	SymbolicObjectWriter(addr_t start, size_t size, size_t managerIdx);
	SymbolicObjectWriter(const SymbolicObjectWriter& o);
	~SymbolicObjectWriter();

	SymbolicExprWriter* read(addr_t addr, Value_t val);

	void write(SymbolicExprWriter* sym_addr, addr_t addr, SymbolicExprWriter* e);

	//  void concretize(SymbolicExprWriter* sym_addr, addr_t addr, size_t n);

	bool Equals(const SymbolicObjectWriter& o) const { return false; }

	void Serialize(ostream &os) const;

	const std::vector<Write>& writes() const{ return writes_;}

	//Debuggind
	void Dump() const;

	addr_t start() const { return start_; }
	addr_t end() const { return start_ + size_; }
	size_t size() const { return size_; }
	size_t managerIdx() const { return managerIdx_; }
	size_t snapshotIdx() const { return snapshotIdx_; }

private:

	const addr_t start_;
	const size_t size_;
	const size_t managerIdx_;
	const size_t snapshotIdx_; //If you don't push it on snapshotManager yet, then do not access to the vector using this variable

	SymbolicMemoryWriter mem_;
	std::vector<Write> writes_;
};

struct WriteAddrIs {
	WriteAddrIs(addr_t addr) : addr_(addr) { }
	bool operator() (const Write write) {
		return write.first->value().integral == addr_;
	}
	addr_t addr_;
};

} // namespace crown

#endif // BASE_SYMBOLIC_OBJECT_H__

