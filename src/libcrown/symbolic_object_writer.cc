// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <assert.h>
#include <z3.h>
#include "libcrown/symbolic_object_writer.h"
#include "libcrown/symbolic_expression_factory.h"

#define OPT2METHOD

using namespace std;

namespace crown {

SymbolicObjectWriter::SymbolicObjectWriter(addr_t start, size_t size, size_t managerIdx)
	: start_(start), size_(size), managerIdx_(managerIdx),
		snapshotIdx_(0),  writes_() {  }

SymbolicObjectWriter::SymbolicObjectWriter(const SymbolicObjectWriter &obj)
	: start_(obj.start_), size_(obj.size_), managerIdx_(obj.managerIdx_),
#ifdef OPT2METHOD
	snapshotIdx_(obj.snapshotIdx_+1), mem_(obj.mem_), writes_(){
#else
	snapshotIdx_(obj.snapshotIdx_+1), mem_(obj.mem_), writes_(obj.writes_){
	for (vector<Write>::iterator it = writes_.begin(); 
									it != writes_.end(); ++it) {
		it->first = it->first->Clone();
		it->second = it->second->Clone();
	}
#endif
}


SymbolicObjectWriter::~SymbolicObjectWriter() {
	for (vector<Write>::iterator it = writes_.begin(); 
									it != writes_.end(); ++it) {
	//	delete it->first;
	//	delete it->second;
	}
}


SymbolicExprWriter* SymbolicObjectWriter::read(addr_t addr, Value_t val) {
	if (writes_.size() == 0) {
		// No symbolic writes yes, so normal read.
		return mem_.read(addr, val);
	} else {
		// There have been symbolic writes, so return a deref.
		return SymbolicExprWriterFactory::NewConstDerefExprWriter(val, *this, addr);
	}
}


void SymbolicObjectWriter::write(SymbolicExprWriter* sym_addr, addr_t addr,
		SymbolicExprWriter* e) {
#if 0
	if ((writes_.size() == 0) && ((sym_addr == NULL) ||
		 sym_addr->IsConcrete())) {
		// Normal write.
		mem_.write(addr, e);
	} else {
#endif

	std::vector<Write>::iterator it = find_if(writes_.begin(), writes_.end(), WriteAddrIs(addr));
	// There have been symbolic writes, so record this write.
	if (sym_addr == NULL) {
		sym_addr = SymbolicExprWriterFactory::NewConcreteExpr(Value_t(addr, 0, types::U_LONG));
	}
/*
	if(it != writes_.end()){
		//replace it second element (assigned symbolic expr)
		it->first = sym_addr;
		it->second = e;
	}else{
*/
		//Insert new expr which the addr does not used yet.
		writes_.push_back(make_pair(sym_addr, e));
//	}
}

#if 0
void SymbolicObjectWriter::concretize(SymbolicExprWriter* sym_addr, addr_t addr, size_t n) {
	if ((writes_.size() == 0) && ((sym_addr == NULL) || sym_addr->IsConcrete())) {
		// Normal write.
		mem_.concretize(addr, n);
	} else {
		// There have been symbolic writes, so record this write.
		// TODO: Don't know how to do this yet.
		assert(false);
		//mem_.concretize(addr, n);
	}
}
#endif


void SymbolicObjectWriter::Serialize(ostream &os) const {
	// Format is:
	// start_ | size_ | mem_ | writes_.size() | writes[0] | ... |writes[n-1]
	// writes[n] == (SymExpr1, SymExpr2)
#ifdef DEBUG
	printf("Serialize obj: start %d, size %d, writes_size %d\n",start_, size_, writes_.size());
#endif
	// Not keeping tracks of symbolic writes
	//assert(writes_.size() == 0);

	os.write((char*)&start_, sizeof(start_));
	os.write((char*)&size_, sizeof(size_));
	os.write((char*)&managerIdx_, sizeof(managerIdx_));
	os.write((char*)&snapshotIdx_, sizeof(snapshotIdx_));
	mem_.Serialize(os);
	size_t size = writes_.size();
	os.write((char*)&size, sizeof(size));
	for (size_t i=0; i<writes_.size(); i++){
		writes_[i].first->Serialize(os);
		writes_[i].second->Serialize(os);
	}

}

void SymbolicObjectWriter::Dump() const {
	mem_.Dump();
}
}  // namespace crown

