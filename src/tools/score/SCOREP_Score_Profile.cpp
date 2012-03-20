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

    m_threads = m_cube->get_thrdv();
    m_regions = m_cube->get_regv();
}

SCOREP_Score_Profile::~SCOREP_Score_Profile()
{
    delete ( m_cube );
}

double
SCOREP_Score_Profile::GetTotalTime( uint64_t region )
{
    return m_cube->get_sev( m_time, CUBE_CALCULATE_EXCLUSIVE,
                            m_regions[ region ], CUBE_CALCULATE_EXCLUSIVE );
}

uint64_t
SCOREP_Score_Profile::GetVisits( uint64_t region, uint64_t thread )
{
    return m_cube->get_sev( m_visits, CUBE_CALCULATE_EXCLUSIVE,
                            m_regions[ region ], CUBE_CALCULATE_EXCLUSIVE,
                            m_threads[ thread ], CUBE_CALCULATE_INCLUSIVE ) + 0.5;
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
    uint64_t thread;
    uint64_t value;

    for ( thread = 0; thread < GetNumberOfThreads(); thread++ )
    {
        value = GetVisits( region, thread );
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
SCOREP_Score_Profile::GetNumberOfThreads()
{
    return m_threads.size();
}

uint64_t
SCOREP_Score_Profile::GetNumberOfMetrics()
{
    return m_cube->get_metv().size();
}

void
SCOREP_Score_Profile::Print()
{
    uint64_t region, thread;

    cout << "Region \t max visits \t total visits \t total time" << endl;
    for ( region = 0; region < GetNumberOfRegions(); region++ )
    {
        cout << GetRegionName( region );
        cout << "\t" << GetMaxVisits( region );
        cout << "\t" << GetTotalVisits( region );
        cout << "\t" << GetTotalTime( region );
        cout << endl;
    }
}
