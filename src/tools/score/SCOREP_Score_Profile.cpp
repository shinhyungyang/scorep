/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 *    RWTH Aachen University, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene, USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */

/**
 * @status     alpha
 * @file       SCOREP_Score_Profile.cxx
 * @maintainer Daniel Lorenz  <d.lorenz@fz-juelich.de>
 *
 * @brief      Implements a class which represents a flat profile in the
 *             scorep-score tool.
 */

#include <config.h>
#include <SCOREP_Score_Profile.hpp>
#include <Cube.h>
#include <CubeTypes.h>

using namespace std;
using namespace cube;

SCOREP_Score_Profile::SCOREP_Score_Profile( string cube_file )
{
    m_cube = new Cube();
    m_cube->openCubeReport( cube_file );

    m_visits = m_cube->get_met( "visits" );
    m_time   = m_cube->get_met( "time" );

    m_processes = m_cube->get_procv();
    m_regions   = m_cube->get_regv();
}

SCOREP_Score_Profile::~SCOREP_Score_Profile()
{
    delete ( m_cube );
}

double
SCOREP_Score_Profile::GetTime( uint64_t region, uint64_t process )
{
    return m_cube->get_sev( m_time, CUBE_CALCULATE_EXCLUSIVE,
                            m_regions[ region ], CUBE_CALCULATE_EXCLUSIVE,
                            m_processes[ process ], CUBE_CALCULATE_INCLUSIVE  );
}

double
SCOREP_Score_Profile::GetTotalTime( uint64_t region )
{
    return m_cube->get_sev( m_time, CUBE_CALCULATE_EXCLUSIVE,
                            m_regions[ region ], CUBE_CALCULATE_EXCLUSIVE );
}

uint64_t
SCOREP_Score_Profile::GetVisits( uint64_t region, uint64_t process )
{
    return m_cube->get_sev( m_visits, CUBE_CALCULATE_EXCLUSIVE,
                            m_regions[ region ], CUBE_CALCULATE_EXCLUSIVE,
                            m_processes[ process ], CUBE_CALCULATE_INCLUSIVE ) + 0.5;
}

uint64_t
SCOREP_Score_Profile::GetTotalVisits( uint64_t region )
{
    return m_cube->get_sev( m_visits, CUBE_CALCULATE_EXCLUSIVE,
                            m_regions[ region ], CUBE_CALCULATE_EXCLUSIVE ) + 0.5;
}

uint64_t
SCOREP_Score_Profile::GetMaxVisits( uint64_t region )
{
    uint64_t max = 0;
    uint64_t process;
    uint64_t value;

    for ( process = 0; process < GetNumberOfProcesses(); process++ )
    {
        value = GetVisits( region, process );
        max   = value > max ? value : max;
    }
    return max;
}

string
SCOREP_Score_Profile::GetRegionName( uint64_t region )
{
    return m_regions[ region ]->get_name();
}

uint64_t
SCOREP_Score_Profile::GetNumberOfRegions()
{
    return m_regions.size();
}

uint64_t
SCOREP_Score_Profile::GetNumberOfProcesses()
{
    return m_processes.size();
}

uint64_t
SCOREP_Score_Profile::GetNumberOfMetrics()
{
    return m_cube->get_metv().size();
}

void
SCOREP_Score_Profile::Print()
{
    uint64_t region, process;

    cout << "group \t max visits \t total visits \t total time \t region" << endl;
    for ( region = 0; region < GetNumberOfRegions(); region++ )
    {
        cout << GetGroup( region );
        cout << "\t" << GetMaxVisits( region );
        cout << "\t" << GetTotalVisits( region );
        cout << "\t" << GetTotalTime( region );
        cout << "\t" << GetRegionName( region );
        cout << endl;
    }
}

SCOREP_Score_GroupId
SCOREP_Score_Profile::GetGroup( uint64_t region )
{
    string name = GetRegionName( region );
    if ( name.substr( 0, 4 ) == "MPI_" )
    {
        return SCOREP_SCORE_GROUP_MPI;
    }
    if ( name.substr( 0, 6 ) == "!$omp " )
    {
        return SCOREP_SCORE_GROUP_OMP;
    }
    else
    {
        return SCOREP_SCORE_GROUP_USR;
    }
}

bool
SCOREP_Score_Profile::HasEnterExit( uint64_t region )
{
    string name = GetRegionName( region );
    if ( name.find( '=', 0 ) == string::npos )
    {
        return true;
    }
    return false; // Is a parameter region which has no enter/exit
}

bool
SCOREP_Score_Profile::HasParameter( uint64_t region )
{
    string name = GetRegionName( region );
    if ( name.find( '=', 0 ) == string::npos )
    {
        return false;
    }
    if ( name.substr( 0, 9 ) == "instance=" )
    {
        return false;                                        // Dynamic region
    }
    return true;
}
