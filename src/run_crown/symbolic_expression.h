// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_SYMBOLIC_EXPRESSION_H__
#define BASE_SYMBOLIC_EXPRESSION_H__

#include <istream>
#include <map>
#include <set>
#include <string>
#include <z3.h>
#include "base/basic_types.h"
#include "run_crown/object_tracker.h"

using std::istream;
using std::map;
using std::set;
using std::string;

namespace crown {

extern long long int global_numOfExpr_;
extern long long int global_numOfOperator_;
extern long long int global_numOfVar_;

class UnaryExpr;
class BinExpr;
class PredExpr;
class AtomicExpr;
class DerefExpr;
class SymbolicObject;

class SymbolicExpr {
public:
	virtual ~SymbolicExpr();

	virtual SymbolicExpr* Clone() const;

	virtual void AppendToString(string* s) const;

	virtual bool IsConcrete() const { return true; }

	virtual void AppendVars(set<var_t>* vars) const{return;} 
	virtual bool DependsOn(const map<var_t,type_t>& vars) const{ return false; }

	// Convert to Z3
	virtual Z3_ast ConvertToSMT(Z3_context ctx, Z3_solver sol) const;

	// Parsing
	static SymbolicExpr* Parse(istream& s);

	// Virtual methods for dynamic casting.
	virtual const UnaryExpr* CastUnaryExpr() const { return NULL; }
	virtual const BinExpr* CastBinExpr() const { return NULL; }
	virtual const DerefExpr* CastDerefExpr() const { return NULL; }
	virtual const PredExpr* CastPredExpr() const { return NULL; }
	virtual const AtomicExpr* CastAtomicExpr() const { return NULL; }

	// Equals.
	virtual bool Equals(const SymbolicExpr &e) const;

	// Accessors.
	Value_t value() const { return value_; }
	size_t size() const { return size_; }

	static void ReadTableClear() { read_table.clear(); }

	friend class SymbolicExprFactory;
protected:
	// Constructor for sub-classes.
	SymbolicExpr(size_t size, Value_t value)
		: unique_id_(++next),value_(value), size_(size) { }


	enum kNodeTags {
		kBasicNodeTag = 0,
		kCompareNodeTag = 1,
		kBinaryNodeTag = 2,
		kUnaryNodeTag = 3,
		kDerefNodeTag = 4,
		kConstNodeTag = 5

	};

	const size_t unique_id_;
private:
	const Value_t value_;
	const size_t size_;
	static size_t next;
	static map<size_t, SymbolicExpr *> read_table;
};

}  // namespace crown

#endif  // BASE_SYMBOLIC_EXPRESSION_H__
