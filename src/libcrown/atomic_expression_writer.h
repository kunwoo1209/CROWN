// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef ATOMIC_TYPE_WRITER_H__
#define ATOMIC_TYPE_WRITER_H__

#include <map>
#include <set>
#include <string>

#include "base/basic_types.h"
#include "libcrown/symbolic_expression_writer.h"
using std::map;
using std::set;
using std::string;


namespace crown {

class AtomicExprWriter : public SymbolicExprWriter {
public:
	AtomicExprWriter(size_t size, Value_t val, var_t var);
	~AtomicExprWriter() { }


	AtomicExprWriter* Clone() const;

	void AppendToString(string* s) const;
	void Serialize(ostream &os) const;

	bool IsConcrete() const { return false; }


	const AtomicExprWriter* CastAtomicExprWriter() const { return this; }


	// Accessor.
	var_t variable() const { return var_; }

private:
	const var_t var_;
};

}  // namespace crown

#endif  // ATOMIC_TYPE_H__
