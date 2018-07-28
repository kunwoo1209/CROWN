// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.
#include <algorithm>
#include <utility>
#include <assert.h>
#include <cstdio>
#include <ext/hash_map>
using std::make_pair;
using std::min;
using __gnu_cxx::hash_map;

#include "libcrown/logger.h"
#include "libcrown/symbolic_memory_writer.h"
#include "libcrown/symbolic_interpreter.h"
namespace crown{
void Logger::DumpMemElem(const SymbolicMemoryWriter::MemElem &elem, const addr_t addr){
	string s,fp_s;

	for (size_t i = 0; i < elem.kMemElemCapacity; i++) {
		if (elem.slots_[i] != NULL) {
			s.clear();
			fp_s.clear();
			elem.slots_[i]->AppendToString(&s);
			fprintf(stderr, "*%lu (%zu): %lld [ %s ]\n",
					addr + i, elem.slots_[i]->size(), elem.slots_[i]->value().integral, s.c_str());
		}
	}
}
void Logger::DumpSymbolicMemoryWriter(const SymbolicMemoryWriter &sym_mem) {
	hash_map<addr_t,SymbolicMemoryWriter::MemElem>::const_iterator it;
	for (it = sym_mem.mem_.begin(); it != sym_mem.mem_.end(); ++it) {
		DumpMemElem(it->second, it->first);
	}
}

void Logger::DumpMemoryAndStack(const SymbolicMemoryWriter &mem_, 
		const vector<SymbolicInterpreter::StackElem> &stack_) {
	fprintf(stderr, "\n");
	DumpSymbolicMemoryWriter(mem_);

	for (size_t i = 0; i < stack_.size(); i++) {
		string s,fp_s;
		if (stack_[i].expr) {
			stack_[i].expr->AppendToString(&s);
		}
		fprintf(stderr, "s%zu (%d): %lld [ %s ]\n", i,
				stack_[i].ty, stack_[i].concrete, s.c_str());
	}

	fprintf(stderr, "\n");
}

} // namespace crown
