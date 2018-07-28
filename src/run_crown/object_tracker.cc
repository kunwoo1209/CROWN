// Copyright (c) 2009, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <cstring>
#include <assert.h>

#include "base/basic_functions.h"
#include "run_crown/object_tracker.h"
#include "run_crown/symbolic_object.h"

using std::map;
using std::string;

namespace crown {

typedef map<addr_t,SymbolicObject*>::iterator EntryIt;
typedef map<addr_t,SymbolicObject*>::const_iterator ConstEntryIt;
typedef map<addr_t,unsigned char*>::iterator EntryItCh;
typedef map<addr_t,unsigned char*>::const_iterator ConstEntryItCh;
typedef map<addr_t,bool>::iterator EntryItDer;
typedef map<addr_t,bool>::const_iterator ConstEntryItDer;

ObjectTracker::~ObjectTracker() {
//printf("Delete ObjectTracker %lx\n",this);
	size_t managerSize = snapshotManager_.size();
	for(size_t i = 0; i < managerSize; i++){
		size_t objsSize = snapshotManager_[i]->size();	
		for(size_t j = 0; j < objsSize; j++){
			delete snapshotManager_[i]->at(j);
		}
		snapshotManager_[i]->clear();
	}
}

void ObjectTracker::Swap(ObjectTracker& tracker){
	snapshotManager_.swap(tracker.snapshotManager_);
	arraysManager_.swap(tracker.arraysManager_);
	isCreateAST_.swap(tracker.isCreateAST_);
}

void ObjectTracker::AppendToString(string *s) const {
	size_t managerSize = snapshotManager_.size();
	for(size_t i = 0; i < managerSize; i++){
		size_t objsSize = snapshotManager_[i]->size();	
		for(size_t j = 0; j < objsSize; j++){
			if(j==0) continue;
			snapshotManager_[i]->at(j)->AppendToString(s);
		}
	}
}

bool ObjectTracker::Parse(istream& s){
	size_t managerSize;

	s.read((char*)&managerSize, sizeof(size_t));
#if DEBUG
	printf("managerSize: %d %lx\n",managerSize, this);
#endif
	assert(snapshotManager_.size()==0);
	for(size_t i = 0; i < managerSize; i++){
		snapshotManager_.push_back(new SnapshotVector());
		arraysManager_.push_back(new ConcreteArrayVector());
		astManager_.push_back(new ASTVector());
		isCreateAST_.push_back(new BoolVector());

		size_t objsSize;
		s.read((char*)&objsSize, sizeof(size_t));
#if DEBUG
		printf("objSize: %d\n",objsSize);
#endif
		for(size_t j = 0; j < objsSize; j++){
			SymbolicObject* obj = SymbolicObject::Parse(s);
			snapshotManager_[i]->push_back(obj);
		}
	}

	return !s.fail();
}



void ObjectTracker::Dump() const {
	//TODO: implement it
}

}  // namespace crown

