SHELL=/bin/sh

SCOREP_OPTS=

TEST_ROOT_DIR=../

MPICC=mpicc
CFLAGS+= -fopenmp

TEST_SRC=tests/$(TEST)/test.c
SOURCES=driver.c utils.c $(TEST_SRC)
OBJECTS=$(addsuffix .o, $(notdir $(basename $(SOURCES))))

ifdef TEST
ifdef PYTHONPATH
ifdef SCOREP
all: timer-test.x
else
all:
	@echo "Please set the environment variable SCOREP to the path of the scorep executable"
	@exit 1
endif
else
all:
	@echo "Please set the environment variable PYTHONPATH to the path of otf2 python package. ex: otf2/_build/lib/python3.9/site-packages"
	@exit 1
endif
else
all:
	@echo "Define the TEST variable: Run this makefile with 'make TEST=...'"
	@exit 1
endif

timer-test.x: $(OBJECTS)
	$(SCOREP) $(SCOREP_OPTS) $(MPICC) $(CFLAGS) $(OBJECTS) -o $@

test.o: $(TEST_ROOT_DIR)$(TEST_SRC)
	$(SCOREP) $(SCOREP_OPTS) $(MPICC) $(CFLAGS) -c $< -o $@

%.o: $(TEST_ROOT_DIR)%.c
	$(SCOREP) $(SCOREP_OPTS) $(MPICC) $(CFLAGS) -c $< -o $@

clean-bin:
	@-rm -f *.o timer-test.x*

clean: clean-bin


.PHONY: clean-bin clean test all
