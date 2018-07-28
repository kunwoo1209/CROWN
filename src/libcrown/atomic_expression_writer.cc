// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include "libcrown/atomic_expression_writer.h"
#include "base/basic_types.h"
#include <iostream>
#include <cstdio>

typedef unsigned int var_t;
namespace crown {

AtomicExprWriter::AtomicExprWriter(size_t size, Value_t val, var_t var)
	: SymbolicExprWriter(size, val), var_(var) { }

AtomicExprWriter* AtomicExprWriter::Clone() const {
	return new AtomicExprWriter(size(), value(), var_);
}

void AtomicExprWriter::AppendToString(string* s) const {
	char buff[32];
	sprintf(buff, "x%u", var_);
	s->append(buff);
}

void AtomicExprWriter::Serialize(ostream &os) const {
	SymbolicExprWriter::Serialize(os, kBasicNodeTag);
	os.write((char*)&var_, sizeof(var_t));
}

}  // namespace crown
