/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

#ifndef SCOREP_INSTRUMENTER_THREAD_HPP
#define SCOREP_INSTRUMENTER_THREAD_HPP

/**
 * @file
 *
 * Defines the threading paradigms.
 */

#include "scorep_instrumenter_selector.hpp"

/* **************************************************************************************
 * class SCOREP_Instrumenter_SingleThreaded
 * *************************************************************************************/

class SCOREP_Instrumenter_SingleThreaded : public SCOREP_Instrumenter_Paradigm
{
public:
    SCOREP_Instrumenter_SingleThreaded( SCOREP_Instrumenter_Selector* selector );
};

/* **************************************************************************************
 * class SCOREP_Instrumenter_OmpTpd
 * *************************************************************************************/
class SCOREP_Instrumenter_OmpTpd : public SCOREP_Instrumenter_Paradigm
{
public:
    SCOREP_Instrumenter_OmpTpd( SCOREP_Instrumenter_Selector* selector );

    virtual bool
    checkCommand( const std::string& current,
                  const std::string& next );
    virtual void
    setConfigValue( const std::string& key,
                    const std::string& value );

private:
    std::string m_openmp_cflag;
};

/* **************************************************************************************
 * class SCOREP_Instrumenter_OmpAncestry
 * *************************************************************************************/
class SCOREP_Instrumenter_OmpAncestry : public SCOREP_Instrumenter_Paradigm
{
public:
    SCOREP_Instrumenter_OmpAncestry( SCOREP_Instrumenter_Selector* selector );

    virtual void
    setConfigValue( const std::string& key,
                    const std::string& value );

    virtual void
    checkDependencies( void );

private:
    std::string m_openmp_cflag;
};

/* **************************************************************************************
 * class SCOREP_Instrumenter_Thread
 * *************************************************************************************/
class SCOREP_Instrumenter_Thread : public SCOREP_Instrumenter_Selector
{
public:
    SCOREP_Instrumenter_Thread();
};

#endif
