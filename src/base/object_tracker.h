// Copyright (c) 2009, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CREST, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef BASE_OBJECT_TRACKER_H__
#define BASE_OBJECT_TRACKER_H__

#include <map>

#include "base/basic_types.h"

namespace crown {

class SymbolicObject;

class ObjectTracker {
public:
	 ObjectTracker() { }
	 ~ObjectTracker();

	 void add(addr_t addr, size_t size);
	 void remove(addr_t addr);

	 SymbolicObject* find(addr_t addr) const;

	 // For debugging.
	 void Dump() const;

private:
	 std::map<addr_t,SymbolicObject*> objs_;
};

}  // namespace crown

#endif //BASE_OBJECT_TRACKER_H__
