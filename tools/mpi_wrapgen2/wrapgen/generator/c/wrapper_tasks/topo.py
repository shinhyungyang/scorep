#
# This file is part of the Score-P software (http://www.score-p.org)
#
# Copyright (c) 2025,
# Forschungszentrum Juelich GmbH, Germany
#
# This software may be modified and distributed under the terms of
# a BSD-style license. See the COPYING file in the package base
# directory for details.
#
__all__ = ['TaskCartCreate']

from wrapgen.generator.c.wrapper_tasks import CTask
from wrapgen.generator.scope import Scope


class TaskCartCreate(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping(comm_cart=['comm_cart', 'newcomm'])

    def name_prefix(self):
        return self.get_attribute('prefix')

    def generate_post_pmpi_call(self):
        yield f'''\
if ( *{self.ipn.comm_cart} != MPI_COMM_NULL && scorep_mpi_enable_topologies )
{{
    scorep_mpi_topo_create_cart_definition( "{self.name_prefix()}", {self.ipn.comm_cart} );
}}
'''
