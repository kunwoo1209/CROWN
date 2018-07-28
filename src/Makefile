# CROWN-FP
SOLVER_DIR=..

CC=$(CXX)
CFLAGS = -I. -I$(SOLVER_DIR)/include -w -Wno-deprecated -Wno-attributes -std=c++11 -g 
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L$(SOLVER_DIR)/lib
LOADLIBES = -lz3 -lrt -lpthread -fopenmp

BASE_LIBS = base/basic_types.o base/basic_functions.o base/stats.o 

MIDDLE_LIBS = libcrown/symbolic_execution_writer.o \
	libcrown/object_tracker_writer.o libcrown/symbolic_object_writer.o \
	libcrown/deref_expression_writer.o libcrown/symbolic_path_writer.o \
	libcrown/atomic_expression_writer.o libcrown/unary_expression_writer.o \
	libcrown/bin_expression_writer.o libcrown/pred_expression_writer.o \
	libcrown/symbolic_expression_writer.o libcrown/symbolic_expression_factory.o \
	libcrown/symbolic_memory_writer.o libcrown/logger.o

MIDDLE_FP_LIBS = libcrown/crown.o libcrown/symbolic_interpreter-noderef.o 
MIDDLE_BV_LIBS = libcrown/crown-nofp.o libcrown/symbolic_interpreter-noderef.o 

BACK_LIBS = run_crown/z3_solver.o \
	run_crown/symbolic_object.o run_crown/symbolic_memory.o \
	run_crown/deref_expression.o run_crown/object_tracker.o \
	run_crown/symbolic_execution.o run_crown/symbolic_path.o \
	run_crown/atomic_expression.o run_crown/unary_expression.o \
	run_crown/bin_expression.o run_crown/pred_expression.o \
	run_crown/symbolic_expression.o run_crown/symbolic_expression_factory.o 


all: 	libcrown/libcrown-fp.a \
	libcrown/libcrown-bv.a \
	replay/libcrown-replay.a \
	run_crown/run_crown \
    process_cfg/process_cfg tools/print_execution \
    install

libcrown/libcrown-fp.a: $(BASE_LIBS) $(MIDDLE_LIBS) $(MIDDLE_FP_LIBS)
	$(AR) rsv $@ $^

libcrown/libcrown-bv.a: $(BASE_LIBS) $(MIDDLE_LIBS) $(MIDDLE_BV_LIBS)
	$(AR) rsv $@ $^

libcrown/crown-nofp.o: libcrown/crown.cc
	$(CC) $(CXXFLAGS) -DDISABLE_FP -c -o libcrown/crown-nofp.o libcrown/crown.cc

libcrown/symbolic_interpreter-noderef.o: libcrown/symbolic_interpreter.cc
	$(CC) $(CXXFLAGS) -DDISABLE_DEREF -c -o libcrown/symbolic_interpreter-noderef.o libcrown/symbolic_interpreter.cc

replay/libcrown-replay.a: $(BASE_LIBS) replay/crown_replay.o
	$(AR) rsv $@ $^


run_crown/run_crown: run_crown/concolic_search.o  $(BASE_LIBS) $(BACK_LIBS)

tools/print_execution: $(BASE_LIBS) $(BACK_LIBS)


install:
	cp libcrown/libcrown-fp.a ../lib
	cp libcrown/libcrown-bv.a ../lib
	cp run_crown/run_crown ../bin
	cp process_cfg/process_cfg ../bin
	cp tools/print_execution ../bin
	cp libcrown/crown.h ../include
	cp replay/libcrown-replay.a ../lib

clean:
	rm -f libcrown/libcrown-fp.a libcrown/libcrown-bv.a replay/libcrown-replay.a
	rm -f run_crown/run_crown
	rm -f process_cfg/process_cfg tools/print_execution
	rm -f */*.o */*~ *~
