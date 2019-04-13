
HOSTNAME = $(shell hostname)
USERNAME = $(shell whoami)

EXEC = ./solver

DEBUG = 1

CPLEX_VERSION = 12.9
CPLEX_DIR = /Applications/CPLEX_Studio129/

ARCH = x86-64
GPP = g++

CPPFLAGS = -DIL_STD \
	-isystem $(CPLEX_DIR)/cplex/include \
	-isystem $(CPLEX_DIR)/concert/include

CXXFLAGS += -Wall -Wno-non-virtual-dtor -pipe

LDFLAGS = -L$(CPLEX_DIR)/cplex/lib/$(ARCH)_osx/static_pic \
	-L$(CPLEX_DIR)/concert/lib/$(ARCH)_osx/static_pic

LDFLAGS += -lilocplex -lcplex -lconcert -lpthread -ldl

ifeq ($(DEBUG), 1)
  CXXFLAGS += -g
else
  CXXFLAGS += -O3
endif

STARTUP_SOURCE = solver.cpp

CPP_SOURCES = \
	src/Instance.cpp \
	src/kMST_ILP.cpp \
	src/Tools.cpp \
	src/CutCallback.cpp \
	src/Maxflow.cpp \

# $< the name of the related file that caused the action.
# $* the prefix shared by target and dependent files.
# $? is the names of the changed dependents.
# $@ is the name of the file to be made.
# $^ dependencies above


# ----- object files ---------------------------------------------------------------


all: solver

%.o: %.cpp
	@echo
	@echo "compiling $<"
	$(GPP) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $< 

# ----- linking --------------------------------------------------------------------

solver: solver.o
	@echo 
	@echo "linking ..."
	@echo
	$(GPP) $(CPPFLAGS) $(CXXFLAGS) -o solver solver.o $(LDFLAGS)

# ----- debugging and profiling ----------------------------------------------------

gdb: all
	gdb --args $(EXEC)

test: all
	$(EXEC)

clean:
	rm -rf obj/*.o kmst gmon.out

