// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.
 
#include <assert.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>

#include "libcrown/symbolic_expression_writer.h"
#include "libcrown/unary_expression_writer.h"
#include "libcrown/bin_expression_writer.h"
#include "libcrown/pred_expression_writer.h"
#include "libcrown/atomic_expression_writer.h"
#include "libcrown/symbolic_object_writer.h"

#ifdef DEBUG
#define IFDEBUG(x) x
#else
#define IFDEBUG(x)
#endif

namespace crown {

typedef map<var_t,value_t>::iterator It;
typedef map<var_t,value_t>::const_iterator ConstIt;

size_t SymbolicExprWriter::next = 0;

SymbolicExprWriter::~SymbolicExprWriter() { }

SymbolicExprWriter* SymbolicExprWriter::Clone() const {
	return new SymbolicExprWriter(size_, value_);
}

void SymbolicExprWriter::AppendToString(string* s) const {
	assert(IsConcrete());

	char buff[92];
	if(value().type == types::FLOAT ||value().type ==types::DOUBLE){
		std::ostringstream fp_out;
		fp_out<<value().floating;
		strcpy(buff, fp_out.str().c_str());
		s->append(buff);
	}else{
		sprintf(buff, "%lld", value().integral);
		s->append(buff);
	}
}


void SymbolicExprWriter::Serialize(ostream &os) const {
	SymbolicExprWriter::Serialize(os, kConstNodeTag);
}

void SymbolicExprWriter::Serialize(ostream &os, char c) const {
	os.write((char*)&value_, sizeof(Value_t));
	os.write((char*)&size_, sizeof(size_t));
	os.write((char*)&unique_id_, sizeof(size_t));
	IFDEBUG(std::cerr<<"SerializeInSymExpr: "<<value_.type<<" "<<value_.integral<<" "<<value_.floating<<" size:"<<(size_t)size_<<" nodeTy:"<<(int)c<<" id:"<<unique_id_<<std::endl);
	os.write(&c, sizeof(char));
}


}  // namespace crown
