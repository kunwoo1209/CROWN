// Copyright (c) 2015-2018, Software Testing & Verification Group
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_SYMBOLIC_EXPRESSION_WRITER_H__
#define BASE_SYMBOLIC_EXPRESSION_WRITER_H__

#include <istream>
#include <ostream>
#include <map>
#include <set>
#include <string>
#include "base/basic_types.h"

using std::istream;
using std::ostream;
using std::map;
using std::set;
using std::string;

namespace crown {

class UnaryExprWriter;
class BinExprWriter;
class PredExprWriter;
class AtomicExprWriter;
class DerefExprWriter;
class SymbolicObjectWriter;

class SymbolicExprWriter {
public:
	virtual ~SymbolicExprWriter();

	virtual SymbolicExprWriter* Clone() const;

	virtual void AppendToString(string* s) const;

	virtual bool IsConcrete() const { return true; }


	//Serialization: Format
	// Value | size | Node type | operator/var | children
	virtual void Serialize(ostream& os) const;

	// Virtual methods for dynamic casting.
	virtual const UnaryExprWriter* CastUnaryExprWriter() const { return NULL; }
	virtual const BinExprWriter* CastBinExprWriter() const { return NULL; }
	virtual const DerefExprWriter* CastDerefExprWriter() const { return NULL; }
	virtual const PredExprWriter* CastPredExprWriter() const { return NULL; }
	virtual const AtomicExprWriter* CastAtomicExprWriter() const { return NULL; }


	// Accessors.
	Value_t value() const { return value_; }
	size_t size() const { return size_; }


	friend class SymbolicExprWriterFactory;
protected:
	// Constructor for sub-classes.
	SymbolicExprWriter(size_t size, Value_t value)
		: value_(value), size_(size), unique_id_(++next) { }

	//Serializing with a Tag
	void Serialize(ostream &os, char c) const;

	enum kNodeTags {
		kBasicNodeTag = 0,
		kCompareNodeTag = 1,
		kBinaryNodeTag = 2,
		kUnaryNodeTag = 3,
		kDerefNodeTag = 4,
		kConstNodeTag = 5

	};

private:
	const Value_t value_;
	const size_t size_;
	const size_t unique_id_;
	static size_t next;
};

}  // namespace crown

#endif  // BASE_SYMBOLIC_EXPRESSION_WRITER_H__
