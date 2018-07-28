// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include "run_crown/symbolic_path.h"
#include<assert.h>
#include<iostream>

namespace crown {

SymbolicPath::SymbolicPath() { }

SymbolicPath::SymbolicPath(bool pre_allocate) {
	if (pre_allocate) {
		// To cut down on re-allocation.
		branches_.reserve(4000000);
		constraints_idx_.reserve(50000);
		constraints_.reserve(50000);
	}
}

SymbolicPath::SymbolicPath(const SymbolicPath &p)
		: branches_(p.branches_),
		constraints_idx_(p.constraints_idx_),
		constraints_(p.constraints_) {
	for(size_t i = 0; i < p.constraints_.size(); i++)
		constraints_[i] = p.constraints_[i]->Clone();
}

SymbolicPath & SymbolicPath::operator= (const SymbolicPath &p){
	if (this == &p) return *this;
	branches_ = p.branches_;
	//branch_info_ = p.branch_info_;
	constraints_idx_ = p.constraints_idx_;
	for (size_t i = 0; i < constraints_.size(); i++){
		delete constraints_[i];
	}
	constraints_.clear();
	constraints_.reserve(p.constraints_.size());
	for (size_t i = 0; i < p.constraints_.size(); i++){
		constraints_.push_back(p.constraints_[i]->Clone());
	}
	return *this;
}

SymbolicPath::~SymbolicPath() {
	for (size_t i = 0; i < constraints_.size(); i++)
		delete constraints_[i];
}

void SymbolicPath::Swap(SymbolicPath& sp) {
	branches_.swap(sp.branches_);
	constraints_idx_.swap(sp.constraints_idx_);
	constraints_.swap(sp.constraints_);
}

bool SymbolicPath::Parse(istream& s) {
	typedef vector<SymbolicExpr*>::iterator ConIt;
	size_t len;
    char *tmp_str;

	SymbolicExpr::ReadTableClear();
	// Read the path.
	s.read((char*)&len, sizeof(size_t));
	if (s.fail()) return false;
	assert(len >= 0);

	branches_.resize(len);
	s.read((char*)&branches_.front(), len * sizeof(branch_id_t));
	if (s.fail())
		return false;

	// Clean up any existing path constraints.
	for (size_t i = 0; i < constraints_.size(); i++){
		delete constraints_[i];
	}

	// Read the path constraints.
	s.read((char*)&len, sizeof(size_t));
	if (s.fail()) return false;
	assert(len >= 0);

	constraints_idx_.resize(len);
	constraints_.resize(len);
    locations_.resize(len);
	s.read((char*)&constraints_idx_.front(), len * sizeof(size_t));

    /* comments written by Hyunwoo Kim (17.07.14)
     * location information (filename, line no.) is added in <istream& s>.
     */
    tmp_str = (char *)malloc(sizeof(char)*256);
    vector<Loc_t>::iterator j = locations_.begin();
	for (ConIt i = constraints_.begin(); i != constraints_.end(); ++i, ++j) {
		s.read((char*)&(j->lineno), sizeof(int));
		s.read((char*)&len, sizeof(size_t));
		s.read((char*)(tmp_str), len);
		tmp_str[len] = '\0';
        j->fname=std::string(tmp_str);

		SymbolicExpr *temp = SymbolicExpr::Parse(s);
		if(temp == NULL)
			return false;
		*i = temp;

	}
    free(tmp_str);

	return !s.fail();
}

}  // namespace crown
