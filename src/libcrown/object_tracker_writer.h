//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef OBJECT_TRACKER_WRTIER_H__
#define OBJECT_TRACKER_WRTIER_H__

#include <map>
#include <cstdio>
#include <ostream>
#include <vector>

#include "base/basic_types.h"

using std::ostream;

namespace crown {

class SymbolicObjectWriter;

class ObjectTrackerWriter {
public:
	ObjectTrackerWriter() { 
		addRegionBarrior = 0;
	}
	~ObjectTrackerWriter();

	void addRegion(addr_t addr, size_t size);
	void removeTrackingObj(addr_t addr);
	void updateDereferredStateOfRegion(SymbolicObjectWriter &obj, addr_t addr, bool truth);

	SymbolicObjectWriter* storeAndGetNewObj(
			SymbolicObjectWriter &obj, addr_t addr);
	void storeAllObjAndRemove();
	void storeObj(addr_t addr);
	
	void Serialize(ostream &os) const;

	SymbolicObjectWriter* find(addr_t addr) const;
	bool getDereferredStateOfRegion(addr_t addr) const;

	// For debugging.
	void Dump() const;

private:
	typedef std::vector<SymbolicObjectWriter*> SnapshotVector;
	std::map<addr_t,SymbolicObjectWriter*> memRegions_;
	std::map<addr_t,bool> isRegionDereferred_;
	std::vector<SnapshotVector*> snapshotManager_;
	//FIXME pairing needs
	
	size_t addRegionBarrior;
};

}  // namespace crown

#endif //BASE_OBJECT_TRACKER_H__

