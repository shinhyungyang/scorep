#!/bin/bash

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2009-2011,
##    RWTH Aachen University, Germany
##    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
##    Technische Universitaet Dresden, Germany
##    University of Oregon, Eugene, USA
##    Forschungszentrum Juelich GmbH, Germany
##    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
##    Technische Universitaet Muenchen, Germany
##
## See the COPYING file in the package base directory for details.
##

## file       run_profile_depth_limit_test.sh

RESULT_DIR=scorep-profile-depth-limit-test-dir
rm -rf $RESULT_DIR

# Run test
SCOREP_EXPERIMENT_DIRECTORY=$RESULT_DIR SCOREP_PROFILING_MAX_CALLPATH_DEPTH=5 SCOREP_ENABLE_PROFILING=true SCOREP_ENABLE_TRACING=false ./profile_depth_limit_test

# Check output
if [ ! -e $RESULT_DIR/profile.cubex ]; then
  echo "Error: No profile generated."
  exit 1
fi

NUM_CNODES=`GREP_OPTIONS= grep -c "<cnode id=" $RESULT_DIR/profile.cubex`
if [ ! x$NUM_CNODES = x6 ]; then
  echo "Expected 6 callpath definitions, but found $NUM_CNODES"
  exit 1
fi

rm -rf $RESULT_DIR
exit 0
