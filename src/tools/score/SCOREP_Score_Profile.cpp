/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2013,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file       SCOREP_Score_Profile.cpp
 *
 * @brief      Implements a class which represents a flat profile in the
 *             scorep-score tool.
 */

#include <config.h>
#include "SCOREP_Score_Profile.hpp"
#include "SCOREP_Score_EventList.hpp"
#include <Cube.h>
#include <CubeTypes.h>
#include <assert.h>

using namespace std;
using namespace cube;

SCOREP_Score_Profile::SCOREP_Score_Profile( string cubeFile )
{
    m_cube = new Cube();
    m_cube->openCubeReport( cubeFile );

    m_visits = m_cube->get_met( "visits" );
    m_time   = m_cube->get_met( "time" );

    m_processes = m_cube->get_procv();
    m_regions   = m_cube->get_regv();

    // Make sure the id of the region definitions match their position in the vector
    for ( uint32_t i = 0; i < GetNumberOfRegions(); i++ )
    {
        m_regions[ i ]->set_id( i );
    }

    // Analyze region types
    m_region_types = ( SCOREP_Score_Type* )
                     malloc( sizeof( SCOREP_Score_Type ) * GetNumberOfRegions() );
    for ( uint32_t i = 0; i < GetNumberOfRegions(); i++ )
    {
        m_region_types[ i ] = get_definition_type( i );
    }
    std::vector<Cnode*> roots =  m_cube->get_root_cnodev();
    for ( uint64_t i = 0; i < roots.size(); i++ )
    {
        calculate_calltree_types( &m_cube->get_cnodev(), roots[ i ] );
    }
}

SCOREP_Score_Profile::~SCOREP_Score_Profile()
{
    delete ( m_cube );
}

bool
SCOREP_Score_Profile::calculate_calltree_types( const vector<Cnode*>* cnodes,
                                                Cnode*                node )
{
    bool is_on_path = false;
    for ( uint32_t i = 0; i < node->num_children(); i++ )
    {
        is_on_path = calculate_calltree_types( cnodes, node->get_child( i ) ) ?
                     true : is_on_path;
    }

    uint32_t          region = node->get_callee()->get_id();
    SCOREP_Score_Type type   = GetGroup( region );
    if ( is_on_path && type == SCOREP_SCORE_TYPE_USR )
    {
        m_region_types[ region ] = SCOREP_SCORE_TYPE_COM;
    }

    if ( type == SCOREP_SCORE_TYPE_OMP ||
         type == SCOREP_SCORE_TYPE_MPI )
    {
        is_on_path = true;
    }
    return is_on_path;
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

string
SCOREP_Score_Profile::GetFileName( uint64_t region )
{
    return m_regions[ region ]->get_mod();
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

SCOREP_Score_Type
SCOREP_Score_Profile::GetGroup( uint64_t region )
{
    assert( region < GetNumberOfRegions() );
    return m_region_types[ region ];
}

SCOREP_Score_Type
SCOREP_Score_Profile::get_definition_type( uint64_t region )
{
    string name = GetRegionName( region );
    if ( name.substr( 0, 4 ) == "MPI_" )
    {
        return SCOREP_SCORE_TYPE_MPI;
    }
    if ( name.substr( 0, 6 ) == "!$omp " )
    {
        return SCOREP_SCORE_TYPE_OMP;
    }
    else
    {
        return SCOREP_SCORE_TYPE_USR;
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

// Requires exact name matching
#define SCOREP_SCORE_EVENT( name ) if ( name == GetRegionName( region ) ) { return true; }

bool
SCOREP_Score_Profile::HasSend( uint64_t region )
{
    SCOREP_SCORE_EVENT_SEND
    return false;
}

bool
SCOREP_Score_Profile::HasIsend( uint64_t region )
{
    SCOREP_SCORE_EVENT_ISEND
    return false;
}

bool
SCOREP_Score_Profile::HasIsendComplete( uint64_t region )
{
    SCOREP_SCORE_EVENT_ISENDCOMPLETE
    return false;
}

bool
SCOREP_Score_Profile::HasIrecvRequest( uint64_t region )
{
    SCOREP_SCORE_EVENT_IRECVREQUEST
    return false;
}

bool
SCOREP_Score_Profile::HasRecv( uint64_t region )
{
    SCOREP_SCORE_EVENT_RECV
    return false;
}

bool
SCOREP_Score_Profile::HasIrecv( uint64_t region )
{
    SCOREP_SCORE_EVENT_IRECV
    return false;
}

bool
SCOREP_Score_Profile::HasCollective( uint64_t region )
{
    SCOREP_SCORE_EVENT_COLLECTIVE
    return false;
}

bool
SCOREP_Score_Profile::HasAcquireLock( uint64_t region )
{
    SCOREP_SCORE_EVENT_ACQUIRELOCK
    return false;
}

bool
SCOREP_Score_Profile::HasReleaseLock( uint64_t region )
{
    SCOREP_SCORE_EVENT_RELEASELOCK
    return false;
}

#undef SCOREP_SCORE_EVENT

// Requires prefix matching
#define SCOREP_SCORE_EVENT( name )                                     \
    if ( name == GetRegionName( region ).substr( 0, strlen( name ) ) ) \
    { return true; }

bool
SCOREP_Score_Profile::HasFork( uint64_t region )
{
    SCOREP_SCORE_EVENT_FORK
    return false;
}

bool
SCOREP_Score_Profile::HasJoin( uint64_t region )
{
    SCOREP_SCORE_EVENT_JOIN
    return false;
}

bool
SCOREP_Score_Profile::HasThreadTeam( uint64_t region )
{
    SCOREP_SCORE_EVENT_THREAD_TEAM
    return false;
}

bool
SCOREP_Score_Profile::HasTaskCreateComplete( uint64_t region )
{
    SCOREP_SCORE_EVENT_TASK_CREATE
    return false;
}

bool
SCOREP_Score_Profile::HasTaskSwitch( uint64_t region )
{
    SCOREP_SCORE_EVENT_TASK_SWITCH
    return false;
}

bool
SCOREP_Score_Profile::HasCudaMemcpy( uint64_t region )
{
    SCOREP_SCORE_EVENT_CUDAMEMCPY
    return false;
}

bool
SCOREP_Score_Profile::HasCudaStreamCreate( uint64_t region )
{
    SCOREP_SCORE_EVENT_CUDASTREAMCREATE
    return false;
}
