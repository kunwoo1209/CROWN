// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_SYMBOLIC_EXECUTION_H__
#define BASE_SYMBOLIC_EXECUTION_H__

#include <istream>
#include <ostream>
#include <utility>
#include <vector>
#include <string>

#include "base/basic_types.h"
#include "libcrown/symbolic_path_writer.h"
#include "libcrown/object_tracker_writer.h"

using std::istream;
using std::make_pair;
using std::ostream;
using std::vector;
using std::string;

namespace crown {

/* comments written by Hyunwoo Kim (17.07.14)
 * New variable and method for reference are added to
 * record nameof symbolic variable and its location.
 */
class SymbolicExecutionWriter {
public:
	SymbolicExecutionWriter();
	explicit SymbolicExecutionWriter(bool pre_allocate);
	~SymbolicExecutionWriter();

	void Swap(SymbolicExecutionWriter& se);

	void Serialize(ostream &os, const ObjectTrackerWriter* tracker) const;
  const vector<string>& var_names() const { return var_names_; }
  const vector<Loc_t>& locations() const { return locations_; }

	const map<var_t,type_t>& vars() const { return vars_; }
	const vector<unsigned long long>& values() const{ return values_;}
	const vector<unsigned char>& indexSize() const {return indexSize_;}
	const vector<unsigned char>& h() const{ return h_;}
	const vector<unsigned char>& l() const {return l_;}
    vector<SymbolicExprWriter *> exprs() {return exprs_;}
	const vector<Value_t>& inputs() const { return inputs_; }
	const SymbolicPathWriter& path() const      { return path_; }

  vector<string>* mutable_var_names() { return &var_names_; }
  vector<Loc_t>* mutable_locations() { return &locations_; }

	map<var_t,type_t>* mutable_vars() { return &vars_; }
	vector<unsigned long long> * mutable_values() {return & values_;}
	vector<unsigned char> * mutable_indexSize() {return & indexSize_;}
	vector<unsigned char> * mutable_h() {return & h_;}
	vector<unsigned char> * mutable_l() {return & l_;}
    vector<SymbolicExprWriter *> * mutable_exprs() {return & exprs_;}
	vector<Value_t>* mutable_inputs() { return &inputs_; }
	SymbolicPathWriter* mutable_path() { return &path_; }

private:
  vector<string> var_names_;
  vector<Loc_t> locations_;

	map<var_t,type_t>  vars_;
	vector<unsigned long long> values_;
	vector<unsigned char> indexSize_;
	vector<unsigned char> h_;
	vector<unsigned char> l_;
    vector<SymbolicExprWriter *> exprs_;
	vector<Value_t> inputs_;
	SymbolicPathWriter path_;  
};

}  // namespace crown

#endif  // BASE_SYMBOLIC_EXECUTION_H__
