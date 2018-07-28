// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef DEREF_EXPRESSION_WRITER_H__
#define DEREF_EXPRESSION_WRITER_H__

#include "base/basic_types.h"
#include "libcrown/symbolic_expression_writer.h"
#include "libcrown/symbolic_object_writer.h"

namespace crown {

class DerefExprWriter : public SymbolicExprWriter {
public:
	DerefExprWriter(SymbolicExprWriter* addr, SymbolicObjectWriter* o,
			size_t size, Value_t val);
	DerefExprWriter(const DerefExprWriter& de);
	~DerefExprWriter();

	DerefExprWriter* Clone() const;

	void AppendToString(string *s) const;

	bool IsConcrete() const { return false; }

	const DerefExprWriter* CastDerefExprWriter() const { return this; }
	void Serialize(ostream &os) const;

	//select one region from many regions (manager), and select specific obj from series of objs
	const size_t managerIdx_;
	const size_t snapshotIdx_;

private:
	// The symbolic object corresponding to the dereference.
	const SymbolicObjectWriter *object_;


	// A symbolic expression representing the symbolic address of this deref.
	const SymbolicExprWriter *addr_;

//	const unsigned char* concrete_bytes_;

};

}  // namespace crown

#endif  // DEREF_EXPRESSION_H__

