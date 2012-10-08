/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
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


#ifndef SCOREP_INTERNAL_ALLOCATOR_H
#define SCOREP_INTERNAL_ALLOCATOR_H


/**
 * @file       scorep_allocator.h
 * @maintainer Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 *
 * @status alpha
 *
 */


#include <SCOREP_Allocator.h>

#include <stdbool.h>
#include <stdint.h>


typedef union SCOREP_Allocator_Object SCOREP_Allocator_Object;


struct SCOREP_Allocator_Page
{
    SCOREP_Allocator_Allocator* allocator;

    char*                       memory_start_address;
    char*                       memory_end_address;     // use until end address
                                                        // but not beyond
    char*                       memory_current_address; // internal use only!

    // SCOREP_Allocator_PageManager and SCOREP_Allocator_Allocator store pages in lists.
    // For internal use only
    SCOREP_Allocator_Page* next;
};


struct SCOREP_Allocator_Allocator
{
    uint32_t page_shift;
    uint32_t n_pages;
    //uint32_t union_size;
    //uint32_t reserved;

    /** free objects */
    SCOREP_Allocator_Object*     free_objects;

    SCOREP_Allocator_Guard       lock;
    SCOREP_Allocator_Guard       unlock;
    SCOREP_Allocator_GuardObject lock_object;
};


struct SCOREP_Allocator_PageManager
{
    SCOREP_Allocator_Allocator* allocator;         // fetch new pages from this one
    SCOREP_Allocator_Page*      pages_in_use_list;

    uint32_t*                   moved_page_id_mapping;

    /* sentinal which allocation could be rolled back */
    /* unly movable allocations curently */
    SCOREP_Allocator_MovableMemory last_allocation;
};


struct SCOREP_Allocator_ObjectManager
{
    SCOREP_Allocator_Allocator* allocator;         // fetch new pages from this one
    SCOREP_Allocator_Page*      pages_in_use_list;
    /* list of free objects */
    void*                       free_list;
    /* size of each object, was rounded-up to meet the required alligment */
    uint32_t                    object_size;
};


union SCOREP_Allocator_Object
{
    union SCOREP_Allocator_Object*        next;
    /* 32: 24, 64: 40 */
    struct SCOREP_Allocator_Page          page;
    /* 32: 16, 64: 32 */
    struct SCOREP_Allocator_PageManager   page_manager;
    /* 32: 16, 64: 32 */
    struct SCOREP_Allocator_ObjectManager object_manager;
};


#endif /* SCOREP_INTERNAL_ALLOCATOR_H */
