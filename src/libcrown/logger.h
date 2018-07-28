// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef LOGGER_H_
#define LOGGER_H_
#include <vector>
#include "libcrown/symbolic_interpreter.h"
#include "libcrown/symbolic_memory_writer.h"
namespace crown{
//class SymbolicMemoryWriter;

class Logger{
public:
	static void DumpMemoryAndStack(const SymbolicMemoryWriter &,
			const vector<SymbolicInterpreter::StackElem>&);
	static void DumpSymbolicMemoryWriter(const SymbolicMemoryWriter&);
	static void DumpMemElem(const SymbolicMemoryWriter::MemElem&, const addr_t);


};

} // namespace crown
#endif // LOGGER_H_
