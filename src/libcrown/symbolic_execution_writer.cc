// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <utility>
#include <iostream>
#include <iomanip>
#include <climits>
#include <assert.h>

#include "libcrown/symbolic_execution_writer.h"

namespace crown {

SymbolicExecutionWriter::SymbolicExecutionWriter() { }

SymbolicExecutionWriter::SymbolicExecutionWriter(bool pre_allocate)
  : path_(pre_allocate) { }

SymbolicExecutionWriter::~SymbolicExecutionWriter() { }

void SymbolicExecutionWriter::Swap(SymbolicExecutionWriter& se) {
	vars_.swap(se.vars_);
	values_.swap(se.values_);
	indexSize_.swap(se.indexSize_);
	h_.swap(se.h_);
	l_.swap(se.l_);
    exprs_.swap(se.exprs_);
	inputs_.swap(se.inputs_);
	path_.Swap(se.path_);
}

void SymbolicExecutionWriter::Serialize(ostream &os, const ObjectTrackerWriter* tracker) const{
	typedef map<var_t,type_t>::const_iterator VarIt;
	// Write the inputs.
    /* comments written by Hyunwoo Kim (17.07.13)
     * variable name_len was added to temporally save the length
     * of symbolic variable name.
     * 
     * Additionally, the length of symbolic variable,
     * the name of symbolic variable,
     * linu of symbolic declaration,
     * the name of file and its length are written in <ostream &os>.
     *
     * This information is used in print_execution.
     */
	size_t len = vars_.size();
	size_t name_len;

	os.write((char*)&len, sizeof(len));
	for (VarIt i = vars_.begin(); i != vars_.end(); ++i) {
        //i->first means index of variable.
        //i->second means type of variable.
		name_len = var_names_[i->first].length();
		os.write((char *)&name_len, sizeof(name_len));

		os.write((char*)(var_names_[i->first].c_str()), name_len);

		os.write((char*) &(values_[i->first]), sizeof(long long));
		os.write((char*) &(indexSize_[i->first]), sizeof(char));
		os.write((char*) &(h_[i->first]), sizeof(char));
		os.write((char*) &(l_[i->first]), sizeof(char));
        exprs_[i->first]->Serialize(os);
		os.write((char*)&(locations_[i->first].lineno), sizeof(int));

    name_len = (locations_[i->first].fname).length();
		os.write((char*)&name_len, sizeof(name_len));

    os.write((char*)((locations_[i->first].fname).c_str()), name_len);

		char ch = static_cast<char>(i->second);
		os.write(&ch, sizeof(char));

		if(i->second == types::FLOAT){
			float tmp = (float)inputs_[i->first].floating;
			os.write((char*)&tmp, kSizeOfType[i->second]);
		}else if(i->second == types::DOUBLE){
			os.write((char*)&inputs_[i->first].floating, kSizeOfType[i->second]);
		}else{
			os.write((char*)&inputs_[i->first].integral, sizeof(value_t));
		}

	}
	// Write symbolic objects from snapshotManager_
	tracker->Serialize(os);	

	// Write the path.
	path_.Serialize(os);


}


}  // namespace crown
