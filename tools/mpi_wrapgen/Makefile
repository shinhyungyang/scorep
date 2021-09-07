#------------------------------------------------------------------------------
# General Settings
#------------------------------------------------------------------------------

CXX             = g++
CFLAGS          = -g `xml2-config --cflags` -I.
CXXFLAGS        = -g `xml2-config --cflags` -I.
LIBS            = `xml2-config --libs`

# settig sfor wrapper generator
MPI_TMPL_DIR    = skel
MPI_OUTPUTDIR   = mpi_generated
MPI_INSTALLDIR  = ../../../src/adapters/mpi/

#settings for the generated user documentation for the mpi wrappers
DOC_TMPL_DIR    = mpi_docs
DOC_OUTPUTDIR   = doc_generated
DOC_INSTALLDIR  = ../../../doc/doxygen-user/

MPI_PROTOTYPES  = $(MPI_TMPL_DIR)/SCOREP_Mpi_Prototypes.xml

WRAPGEN         = ./wrapgen
WFLAGS          = -b -p $(MPI_PROTOTYPES)
INSTALL         = /usr/bin/install

DOXYGEN         = doxygen
DOXYGEN_CONF    = dox_mpi_wrapper.cfg
DOXYGEN_OPTS    =

MODS            = SCOREP_Wrapgen_Funcparam.o SCOREP_Wrapgen_Func.o SCOREP_Wrapgen_MpiFunc.o SCOREP_Wrapgen_Util.o SCOREP_Wrapgen_Handlers.o SCOREP_Wrapgen_HandlersMpi.o

MPI_FILES       = $(patsubst $(MPI_TMPL_DIR)/%.tmpl,%,$(wildcard $(MPI_TMPL_DIR)/*.tmpl))
DOC_FILES       = $(patsubst $(DOC_TMPL_DIR)/%.tmpl,%,$(wildcard $(DOC_TMPL_DIR)/*.tmpl))

.PHONY: all install clean diff-files clean-diff-files

.FORCE:

all: protogen wrapgen all-files

install: all install-files

clean: clean-files clean-diff-files
	rm -f *.o wrapgen

doc: all-files $(DOXYGEN_CONF)
	$(DOXYGEN) $(DOXYGEN_OPTS) $(DOXYGEN_CONF)

wrapgen : SCOREP_Wrapgen.o $(MODS)
	$(CXX) $(CXXFLAGS) $(MODS) SCOREP_Wrapgen.o $(LIBS) -o $@

SCOREP_Wrapgen_Funcparam.o : SCOREP_Wrapgen_Funcparam.h SCOREP_Wrapgen_Funcparam.cpp
	$(CXX) $(CXXFLAGS) -Wall -c $*.cpp

SCOREP_Wrapgen_Func.o : SCOREP_Wrapgen_Func.h SCOREP_Wrapgen_Func.cpp SCOREP_Wrapgen_Util.h SCOREP_Wrapgen_Util.cpp SCOREP_Wrapgen_Funcparam.h \
        SCOREP_Wrapgen_Funcparam.cpp
	$(CXX) $(CXXFLAGS) -Wall -c $*.cpp

SCOREP_Wrapgen_MpiFunc.o : SCOREP_Wrapgen_MpiFunc.h SCOREP_Wrapgen_MpiFunc.cpp SCOREP_Wrapgen_Util.h SCOREP_Wrapgen_Util.cpp \
        SCOREP_Wrapgen_Funcparam.h SCOREP_Wrapgen_Funcparam.cpp
	$(CXX) $(CXXFLAGS) -Wall -c $*.cpp

SCOREP_Wrapgen_Util.o : SCOREP_Wrapgen_Util.cpp SCOREP_Wrapgen_Util.h
	$(CXX) $(CXXFLAGS) -Wall -c $*.cpp

SCOREP_Wrapgen_Handlers.o : SCOREP_Wrapgen_Handlers.cpp SCOREP_Wrapgen_Handlers.h SCOREP_Wrapgen_MpiFunc.h SCOREP_Wrapgen_Func.h
	$(CXX) $(CXXFLAGS) -Wall -c $*.cpp

wrapgen_handlers_mpi.o :SCOREP_Wrapgen_HandlersMpi.cppSCOREP_Wrapgen_HandlersMpi.h SCOREP_Wrapgen_MpiFunc.h SCOREP_Wrapgen_Func.hSCOREP_Wrapgen_HandlersMpi_datatypes.h
	$(CXX) $(CXXFLAGS) -Wall -c $*.cpp

SCOREP_Wrapgen.o : SCOREP_Wrapgen.cpp SCOREP_Wrapgen.h SCOREP_Wrapgen_Help.h
	$(CXX) $(CXXFLAGS) -Wall -c $*.cpp

# Targets for creating wrapper files

all-files: $(addprefix $(MPI_OUTPUTDIR)/,$(MPI_FILES)) $(addprefix $(DOC_OUTPUTDIR)/,$(DOC_FILES))

clean-files:
	@if [ -d $(MPI_OUTPUTDIR) ]; then (cd $(MPI_OUTPUTDIR) && rm -f $(MPI_FILES)) fi
	@if [ -d $(MPI_OUTPUTDIR) ] && [ -z `ls $(MPI_OUTPUTDIR)` ]; then (rmdir $(MPI_OUTPUTDIR)) fi

install-files: all-files $(addprefix install_,$(MPI_FILES))
	@(cd $(DOC_OUTPUTDIR) && $(INSTALL) -m 644 $(DOC_FILES) $(DOC_INSTALLDIR))

# Install each file only if it differs from the installed version
$(addprefix install_,$(MPI_FILES)): install_%: $(MPI_OUTPUTDIR)/% $(MPI_OUTPUTDIR)/diff_%
	@if [ "$$(find $(MPI_OUTPUTDIR) -not -empty -name diff_$*)" = "$(MPI_OUTPUTDIR)/diff_$*" ]; \
	then echo "Installing $* into $(MPI_INSTALLDIR)"; (cd $(MPI_OUTPUTDIR) && $(INSTALL) -m 644 $* $(MPI_INSTALLDIR)); \
	fi

.PHONY: $(addprefix install_,$(MPI_FILES))

diff-files: $(addprefix $(MPI_OUTPUTDIR)/diff_,$(MPI_FILES))

# Produce a diff  between generated and installed files
# Diff reports differences with exit code 1, therefore we have to make sure
# that this is not seen as an error by make
# diff -N : -N treat missing files as new
$(MPI_OUTPUTDIR)/diff_SCOREP_%: $(MPI_OUTPUTDIR)/SCOREP_% .FORCE
	@cd $(MPI_OUTPUTDIR) && diff -N SCOREP_$* $(MPI_INSTALLDIR)SCOREP_$* > diff_SCOREP_$*; \
	diff_status=$$?; \
	if [ $$diff_status -gt 1 ]; \
	then exit 1; \
	fi; \
	if [ $$diff_status -eq 1 ]; \
	then echo "$<"; \
	fi; \
	exit 0

clean-diff-files:
	-rm -f $(MPI_OUTPUTDIR)/diff_SCOREP_*


# Auto-gerenation targets for MPI wrappers

$(MPI_OUTPUTDIR)/SCOREP_%: $(MPI_PROTOTYPES) $(MPI_TMPL_DIR)/SCOREP_%.tmpl $(MPI_TMPL_DIR)/*.w wrapgen
	@if [ ! -d $(MPI_OUTPUTDIR) ]; then mkdir -p $(MPI_OUTPUTDIR); fi
	@echo "  GEN   " $@; $(WRAPGEN) $(WFLAGS) $(MPI_TMPL_DIR)/$(notdir $@).tmpl > $@
	uncrustify -c ../../common/beautifier/beautify.cfg --no-backup -q $@

$(DOC_OUTPUTDIR)/%: $(MPI_PROTOTYPES) $(DOC_TMPL_DIR)/%.tmpl $(DOC_TMPL_DIR)/*.w wrapgen
	@if [ ! -d $(DOC_OUTPUTDIR) ]; then mkdir -p $(DOC_OUTPUTDIR); fi
	@echo "  GEN   " $@; $(WRAPGEN) -p $(MPI_PROTOTYPES) $(DOC_TMPL_DIR)/$(notdir $@).tmpl > $@


# Targets for Prototype generator from header files

SCOREP_Wrapgen_HeaderParser.o: SCOREP_Wrapgen_HeaderParser.cpp
	$(CXX) $(CXXFLAGS) SCOREP_Wrapgen_HeaderParser.cpp  -c -o SCOREP_Wrapgen_HeaderParser.o

protogen: SCOREP_Wrapgen_HeaderParser.o $(MODS)
	$(CXX) $(CXXFLAGS) $(MODS) SCOREP_Wrapgen_HeaderParser.o $(LIBS) -o protogen
