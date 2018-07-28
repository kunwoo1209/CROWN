//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

/***
 * Author: Sudeep juvekar (sjuvekar@eecs.berkeley.edu)
 * 4/17/09
 */

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "base/basic_functions.h"
#include "libcrown/deref_expression_writer.h"
#include "libcrown/symbolic_object_writer.h"
#include "libcrown/symbolic_expression_factory.h"

namespace crown {

DerefExprWriter::DerefExprWriter(SymbolicExprWriter *c, SymbolicObjectWriter *o,
                     size_t s, Value_t v)
  : SymbolicExprWriter(s,v), managerIdx_(o->managerIdx()), 
	snapshotIdx_(o->snapshotIdx()), object_(o), addr_(c){ }


DerefExprWriter::DerefExprWriter(const DerefExprWriter& de)
  : SymbolicExprWriter(de.size(), de.value()),
	managerIdx_(de.managerIdx_), snapshotIdx_(de.snapshotIdx_),
    object_(de.object_), addr_(de.addr_->Clone()) { }

DerefExprWriter::~DerefExprWriter() {
//  delete object_;
	delete addr_;
}

DerefExprWriter* DerefExprWriter::Clone() const {
  return new DerefExprWriter(*this);
}

void DerefExprWriter::AppendToString(string *s) const {
  s->append(" (*?)");
}

void DerefExprWriter::Serialize(ostream &os) const {
	SymbolicExprWriter::Serialize(os, kDerefNodeTag);

	assert(object_->managerIdx() == managerIdx_);
	assert(object_->snapshotIdx() == snapshotIdx_ && "There are some bugs for assigning snapshotIdx");

	/*
	for(size_t i = 0; i < object_->writes().size(); i++){
		SymbolicExprWriter *exp = object_->writes()[i].second;
	}
	*/

	//Store the indexes to find object on run_crown
	size_t managerIdx = object_->managerIdx();
	size_t snapshotIdx = object_->snapshotIdx();
	os.write((char*)&managerIdx, sizeof(size_t));
	os.write((char*)&snapshotIdx, sizeof(size_t));

//	object_->Serialize(os);
	addr_->Serialize(os);
}


}  // namespace crown

