## -*- mode: awk -*-

## 
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2009-2011, 
##    RWTH Aachen, Germany
##    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
##    Technische Universitaet Dresden, Germany
##    University of Oregon, Eugene, USA
##    Forschungszentrum Juelich GmbH, Germany
##    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
##    Technische Universitaet Muenchen, Germany
##
## See the COPYING file in the package base directory for details.
##

## file       definitions_write_to_otf2.awk
## maintainer Christian Roessel <c.roessel@fz-juelich.de>

{
    print "static void"
    print "scorep_write_" $3 "_definitions_to_otf2( OTF2_DefWriter* definitionWriter )"
    print "{"
    print "    " $2 "_Definition* definition ="
    print "        &( scorep_definition_manager." $3 "_definitions_head_dummy );"
    print "    while ( !SCOREP_MEMORY_MOVABLE_IS_NULL( definition->next ) )"
    print "    {"
    print "        definition = SCOREP_MEMORY_DEREF_MOVABLE( &( definition->next ),"
    print "                                                " $2 "_Definition* );"
    print "        //SCOREP_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);"
    print "        //if ( status != SCOREP_SUCCESS )"
    print "        //{"
    print "        //    scorep_handle_definition_writing_error( status, \"" $2 "_Definition\" );"
    print "        //}"
    print "        assert( false ); // implement me"
    print "    }"
    print "}\n\n"
}
