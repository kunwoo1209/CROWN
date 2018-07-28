// Copyright (c) 2009, Jacob Burnim (jburnim@cs.berkeley.edu)
//
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
#include "run_crown/symbolic_memory.h"
#include "run_crown/symbolic_expression.h"
#include "run_crown/symbolic_expression_factory.h"

using std::make_pair;
using std::min;
using __gnu_cxx::hash_map;

namespace crown {

const size_t SymbolicMemory::MemElem::kMemElemCapacity;
const size_t SymbolicMemory::MemElem::kOffsetMask;
const size_t SymbolicMemory::MemElem::kAddrMask;

SymbolicMemory::MemElem::MemElem() {
	for (size_t i = 0; i < kMemElemCapacity; i++) {
		slots_[i] = NULL;
	}
}


SymbolicMemory::MemElem::MemElem(const MemElem& elem) {
	for (size_t i = 0; i < kMemElemCapacity; i++) {
		if (elem.slots_[i] != NULL) {
			slots_[i] = elem.slots_[i]->Clone();
		} else {
			slots_[i] = NULL;
		}
	}
}

SymbolicMemory::MemElem::MemElem(SymbolicExpr** slots) {
	for(size_t i = 0; i < kMemElemCapacity; i++)
		slots_[i] = slots[i];
}

SymbolicMemory::MemElem::~MemElem() {
	for (size_t i = 0; i < kMemElemCapacity; i++) {
		delete slots_[i];
	}
}


SymbolicExpr* SymbolicMemory::MemElem::read(addr_t addr,
		size_t n,
		Value_t val, const MemElem *next_elem) const {
	size_t i = addr & kOffsetMask;

{
	size_t j = i;
	while ((j != 0) && (slots_[j] == NULL)) {
		//j = j & (j - 1);  
		j--;
	}

	if ((slots_[j] != NULL)                // found an expression,
			&& (slots_[j]->size() > n)         // the expression is larger,
			&& (j + slots_[j]->size() > i)) {  // and our read overlaps

		return SymbolicExprFactory::ExtractBytes(slots_[j]->Clone(), i - j, n);
	}
}
{
	SymbolicExpr* ret = NULL;

	size_t j = 0;
	do {
		if (i + j < kMemElemCapacity){
			if (slots_[i + j] != NULL) {
				if (ret == NULL) {
					ret = slots_[i + j]->Clone();
				} else {
					ret = SymbolicExprFactory::Concatenate(ret, slots_[i + j]->Clone());
				}

			} else {
				while ((++j < n) && (slots_[i + j] == NULL)) ;
				if (ret == NULL) {
					if (j == n) return NULL;  // Whole read is concrete.
					ret = SymbolicExprFactory::ExtractBytes(n, val, 0, j);
				} else {
					SymbolicExpr* tmp =
						SymbolicExprFactory::ExtractBytes(n, val, ret->size(), j - ret->size());
					ret = SymbolicExprFactory::Concatenate(ret, tmp);
				}
			}

			j = ret->size();
		}else{
			if (next_elem->slots_[i + j - kMemElemCapacity] != NULL){
				if (ret == NULL){
					ret = next_elem->slots_[i + j - kMemElemCapacity]->Clone();
				}else{
					ret = SymbolicExprFactory::Concatenate(ret, 
							next_elem->slots_[i + j - kMemElemCapacity]->Clone());               }
			}else{
				while ((++j < n) && (next_elem->slots_[i + j - kMemElemCapacity] == NULL));
				if (ret == NULL){
					if (j == n) return NULL;
					ret = SymbolicExprFactory::ExtractBytes(n, val, 0, j);
				} else {
					SymbolicExpr* tmp = 
						SymbolicExprFactory::ExtractBytes(n, val, ret->size(), j - ret->size());
					ret = SymbolicExprFactory::Concatenate(ret, tmp);
				}
			}
			j = ret->size();
		}
	} while (j < n);

	return ret;
  }

}

bool SymbolicMemory::MemElem::Parse(istream& in) {
	// Assumption: slab is empty, so we don't have to clear it out.

	unsigned int c; //= static_cast<unsigned int>(in.get());
	in.read((char*)&c, sizeof(c));
	for (size_t i = 0; i < kMemElemCapacity; i++) {
		//c <<= 1;
		//if (!(c & 0x100))
		//	continue;
		if( ((c>>i) & 0x0001) != 1) continue;

		slots_[i] = SymbolicExpr::Parse(in);
		if (slots_[i] == NULL) {
			return false;
		}
	}

	return true;
}


void SymbolicMemory::MemElem::Dump(addr_t addr) const {
	string s;

	for (size_t i = 0; i < kMemElemCapacity; i++) {
		if (slots_[i] != NULL) {
			s.clear();
			slots_[i]->AppendToString(&s);
			fprintf(stderr, "*%lu (%zu): %lld [ %s ]\n",
					addr + i, slots_[i]->size(), slots_[i]->value().integral, s.c_str());
		}
	}
}


SymbolicMemory::SymbolicMemory() { }


SymbolicMemory::SymbolicMemory(const SymbolicMemory& m)
	: mem_(m.mem_) { }


SymbolicMemory::~SymbolicMemory() { }

void SymbolicMemory::Dump() const {
	hash_map<addr_t,MemElem>::const_iterator it;
	for (it = mem_.begin(); it != mem_.end(); ++it) {
		it->second.Dump(it->first);
	}
}

SymbolicExpr* SymbolicMemory::read(addr_t addr, Value_t val) const {
	if (val.type == types::STRUCT)
		return NULL;

	hash_map<addr_t,MemElem>::const_iterator it = mem_.find(addr & MemElem::kAddrMask);
	if (it == mem_.end())
		return NULL;

	MemElem default_elem;
	const MemElem *next = &default_elem;
	hash_map<addr_t,MemElem>::const_iterator it2 = mem_.find((addr + MemElem::kMemElemCapacity)
			& MemElem::kAddrMask);
	if (it2 != mem_.end()){
		next = &(it2->second);
	}

	size_t n = kSizeOfType[val.type];
	return it->second.read(addr, n, val, next);
}


bool SymbolicMemory::Parse(istream &s) {
	// Assumption: This object is empty, so we don't have to clear it out.

	size_t size;
	addr_t addr;

	s.read((char*)&size, sizeof(size));
	if (s.fail())
		return false;
	for(size_t i = 0; i < size; i++) {
		s.read((char*)&addr, sizeof(addr));
		if (s.fail())
			return false;

		if (!mem_[addr].Parse(s))
			return false;
	}

	return true;
}


Z3_ast SymbolicMemory::ConvertToSMT(Z3_context ctx, addr_t addr) {
	/*
	   SymbolicExpr* expr = mem_[addr];
	   return expr->BitBlast(ctx);
	 */
	return NULL;
}


}  // namespace crown
