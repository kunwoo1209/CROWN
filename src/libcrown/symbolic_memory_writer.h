// This file is part of CREST, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef SYMBOLIC_MEMORY_WRITER_H__
#define SYMBOLIC_MEMORY_WRITER_H__

#include <ext/hash_map>
#include "base/basic_types.h"
#include <ostream>

using std::ostream;
using std::string;
using std::istream;

namespace crown {

class SymbolicExprWriter;

class SymbolicMemoryWriter {
	friend class Logger;
public:
	SymbolicMemoryWriter();
	SymbolicMemoryWriter(const SymbolicMemoryWriter& m);
	~SymbolicMemoryWriter();

	SymbolicExprWriter* read(addr_t addr, Value_t val) const;

	void write(addr_t addr, SymbolicExprWriter* e);

	void concretize(addr_t addr, size_t n);

	void Serialize(ostream &os) const;

	// For debugging.
	void Dump() const;

private:
	class MemElem {
		friend class Logger;
		public:
		MemElem();
		MemElem(const MemElem& elem);
		MemElem(SymbolicExprWriter **slot);
		~MemElem();
		inline SymbolicExprWriter* read(addr_t addr, size_t n, Value_t val, const MemElem *) const;
		inline void write(addr_t addr, size_t n, SymbolicExprWriter* e);

		void Dump(addr_t addr) const;
		void Serialize(ostream &os) const;

#if 1
		static const size_t kMemElemCapacity = 32;
		static const size_t kOffsetMask = kMemElemCapacity - 1;
		static const size_t kAddrMask = ~kOffsetMask;
#else
	static const size_t kMemElemCapacity = 4096;
	static const size_t kOffsetMask = kMemElemCapacity - 1;
	static const size_t kAddrMask = ~kOffsetMask;    
#endif

		private:
		SymbolicExprWriter* slots_[kMemElemCapacity];
	};

	__gnu_cxx::hash_map<addr_t, MemElem> mem_;
};

}

#endif //SYMBOLIC_MEMORY_WRITER_H__
