// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_SYMBOLIC_MEMORY_H__
#define BASE_SYMBOLIC_MEMORY_H__

#include <ext/hash_map>
#include "base/basic_types.h"
#include <z3.h>
#include <ostream>

using std::ostream;
using std::string;
using std::istream;

namespace crown {

class SymbolicExpr;

class SymbolicMemory {
	friend class Logger;
public:
	SymbolicMemory();
	SymbolicMemory(const SymbolicMemory& m);
	~SymbolicMemory();

	SymbolicExpr* read(addr_t addr, Value_t val) const;

	bool Parse(istream &s);
	Z3_ast ConvertToSMT(Z3_context ctx, addr_t addr);

	// For debugging.
	void Dump() const;

private:
	class MemElem {
		friend class Logger;
		public:
		MemElem();
		MemElem(const MemElem& elem);
		MemElem(SymbolicExpr **slot);
		~MemElem();
		inline SymbolicExpr* read(addr_t addr, size_t n, Value_t val, const MemElem *next) const;

		void Dump(addr_t addr) const;
		bool Parse(istream &s);
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
		SymbolicExpr* slots_[kMemElemCapacity];
	};

	__gnu_cxx::hash_map<addr_t, MemElem> mem_;
};

}

#endif // BASE_SYMBOLIC_MEMORY_H__
