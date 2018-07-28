// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_SYMBOLIC_PATH_H__
#define BASE_SYMBOLIC_PATH_H__

#include <algorithm>
#include <istream>
#include <ostream>
#include <vector>

#include "base/basic_types.h"
#include "run_crown/symbolic_expression.h"
#include "run_crown/object_tracker.h"

using std::istream;
using std::ostream;
using std::swap;
using std::vector;

namespace crown {

class SymbolicPath {
public:
	SymbolicPath();
	SymbolicPath(bool pre_allocate);
	SymbolicPath(const SymbolicPath &p);
	SymbolicPath & operator= (const SymbolicPath &p);
	~SymbolicPath();

	void Swap(SymbolicPath& sp);

	bool Parse(istream& s);

	const vector<branch_id_t>& branches() const { return branches_; }
	const vector<SymbolicExpr*>& constraints() const { return constraints_; }
	const vector<size_t>& constraints_idx() const { return constraints_idx_; }

    const vector<Loc_t>& locations() const { return locations_; }
    vector<Loc_t>* mutable_locations() { return &locations_; }

private:
    vector<Loc_t> locations_;

	vector<branch_id_t> branches_;
	vector<size_t> constraints_idx_;
	vector<SymbolicExpr*> constraints_;
};

}  // namespace crown

#endif  // BASE_SYMBOLIC_PATH_H__
