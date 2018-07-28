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

#include "run_crown/symbolic_execution.h"

/* comments written by Hyunwoo Kim (17.07.14)
 * global variable g_var_names is used in atomic_expression_writer.cc
 * this variable is used to refer vector including variable name.
 */

vector<string> g_var_names;
namespace crown {

ObjectTracker* global_tracker_ = NULL;

SymbolicExecution::SymbolicExecution() { }

SymbolicExecution::SymbolicExecution(bool pre_allocate)
  : path_(pre_allocate) { }

SymbolicExecution::~SymbolicExecution() { }

void SymbolicExecution::Swap(SymbolicExecution& se) {
	vars_.swap(se.vars_);
	values_.swap(se.values_);
	inputs_.swap(se.inputs_);
	h_.swap(se.h_);
	l_.swap(se.l_);
    exprs_.swap(se.exprs_);
	indexSize_.swap(se.indexSize_);
	path_.Swap(se.path_);
	ObjectTracker objTemp = object_tracker_;
	object_tracker_ = se.object_tracker_;
	se.object_tracker_ = objTemp;
//	object_tracker_.Swap(se.object_tracker_);
}

bool SymbolicExecution::Parse(istream& s) {
	// Read the inputs.
  char *tmp_str;
	size_t len;

    /*
     * comments written by Hyunwoo Kim (17.07.13)
     * read symbolic variable name and its length, filename and its length from <ifstream& s>
     * variable name_len were used to save string temporally.
     */

	size_t name_len;
	s.read((char*)&len, sizeof(len));
	if (s.fail()) return false;
	assert(len >= 0);

  tmp_str = (char *)malloc(sizeof(char) * 256);

	vars_.clear();
	values_.resize(len);
	indexSize_.resize(len);
	h_.resize(len);
	l_.resize(len);
    exprs_.resize(len);
	inputs_.resize(len);
  var_names_.resize(len);
  locations_.resize(len);

	for (size_t i = 0; i < len; i++) {
		s.read((char*)&name_len, sizeof(name_len));
		s.read((char*)tmp_str, name_len);
    tmp_str[name_len]='\0';
    var_names_[i]=std::string(tmp_str);

		s.read((char*) &(values_[i]), sizeof(long long));
		s.read((char*) &(indexSize_[i]), sizeof(char));
		s.read((char*) &(h_[i]), sizeof(char));
		s.read((char*) &(l_[i]), sizeof(char));
        exprs_[i] = SymbolicExpr::Parse(s);
		s.read((char*)&(locations_[i].lineno), sizeof(int));

		s.read((char*)&name_len, sizeof(name_len));
		s.read((char*)tmp_str, name_len);
		tmp_str[name_len]='\0';
    locations_[i].fname=std::string(tmp_str);

		vars_[i] = static_cast<type_t>(s.get());
		inputs_[i].type = vars_[i];
		if(vars_[i] == types::FLOAT){
			float tmp;
			s.read((char*)&tmp, kSizeOfType[vars_[i]]);
			inputs_[i].floating = (double) tmp;
			//		inputs_[i].integral = inputs_[i].floating;
		}else if(vars_[i] == types::DOUBLE){
			s.read((char*)&inputs_[i].floating, kSizeOfType[vars_[i]]);
			//		inputs_[i].integral = inputs_[i].floating;
		}else {
			s.read((char*)&inputs_[i].integral, sizeof(value_t));
			//		inputs_[i].floating = inputs_[i].integral;
		}
#ifdef DEBUG
		std::cerr<<"Parse: "<<inputs_[i].integral<<" "<<inputs_[i].floating<<" "<<vars_[i]<<std::endl;
#endif
	}
    free(tmp_str);

	//Read symbolicObjects
	global_tracker_ = &object_tracker_;
	object_tracker_.Parse(s);

    g_var_names = var_names_;
	// Write the path.
	return (path_.Parse(s) && !s.fail());
}

}  // namespace crown
