//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_OBJECT_TRACKER_H__
#define BASE_OBJECT_TRACKER_H__

#include <map>
#include <cstdio>
#include <ostream>
#include <istream>
#include <vector>
#include <z3.h>

#include "base/basic_types.h"

using std::ostream;
using std::istream;
using std::string;

namespace crown {

class SymbolicObject;

class ObjectTracker {
typedef std::vector<SymbolicObject*> SnapshotVector;
typedef std::vector<unsigned char*> ConcreteArrayVector;
typedef std::vector<Z3_ast> ASTVector;
typedef std::vector<bool> BoolVector;
public:
	ObjectTracker() {
		isASTInit = false;
	}
	~ObjectTracker();
	
	bool Parse(istream& s);
	
	void AppendToString(string *s) const;
	void Swap(ObjectTracker& tracker);

	// For debugging.
	void Dump() const;

	std::vector<SnapshotVector*> snapshotManager(){ return snapshotManager_;}
	std::vector<ASTVector*> astManager(){ return astManager_;}
	std::vector<BoolVector*> isCreateAST(){ return isCreateAST_;}
	bool isASTInit;
	
private:
	std::vector<SnapshotVector*> snapshotManager_;
	std::vector<ConcreteArrayVector*> arraysManager_;
	std::vector<ASTVector*> astManager_;
	std::vector<BoolVector*> isCreateAST_;
	//FIXME pairing needs
};

extern ObjectTracker* global_tracker_;

}  // namespace crown

#endif //BASE_OBJECT_TRACKER_H__

