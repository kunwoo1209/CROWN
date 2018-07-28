// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef SYMBOLIC_PATH_WRITER_H__
#define SYMBOLIC_PATH_WRITER_H__

#include <algorithm>
#include <istream>
#include <ostream>
#include <vector>

#include "base/basic_types.h"
#include "libcrown/symbolic_expression_writer.h"

using std::istream;
using std::ostream;
using std::swap;
using std::vector;

namespace crown {

class SymbolicPathWriter {
public:
	SymbolicPathWriter();
	SymbolicPathWriter(bool pre_allocate);
	SymbolicPathWriter(const SymbolicPathWriter &p);
	~SymbolicPathWriter();

	void Swap(SymbolicPathWriter& sp);

	void Push(branch_id_t bid);

    /*
     * comments written by Hyunwoo Kim (17.07.13)
     * line number and file name of branch were added as parameters.
     * the variable line_ and fname_ to save them were added.
     */
	void Push(branch_id_t bid, SymbolicExprWriter* constraint, unsigned int lineno, const char *filename);
	void Push(branch_id_t bid, SymbolicExprWriter* constraint, bool pred_value, 
			unsigned int lineno, const char *filename, const char *exp);
	void Serialize(ostream &os) const;

	const vector<branch_id_t>& branches() const { return branches_; }
	const vector<SymbolicExprWriter*>& constraints() const { return constraints_; }
	const vector<size_t>& constraints_idx() const { return constraints_idx_; }

    const vector<Loc_t>& locations() const { return locations_; }
    vector<Loc_t>* mutable_locations() { return &locations_; }

private:
    vector<Loc_t> locations_;

	vector<branch_id_t> branches_;
	vector<size_t> constraints_idx_;
	vector<SymbolicExprWriter*> constraints_;
};

}  // namespace crown

#endif  // BASE_SYMBOLIC_PATH_H__
