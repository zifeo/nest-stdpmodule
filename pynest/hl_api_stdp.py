# -*- coding: utf-8 -*-
#
# hl_api_stdp.py

"""
Functions for spike-timing dependent plasticity.
"""

"""
@check_stack
def Install(module_name):

    return sr("(%s) Install" % module_name)
"""

import nest

def helloSTDP():
    nest.Install("stdpmodule")

    nest_create = nest.Create

    def _create(name):
        print "create"
        return nest_create(name)

    nest.Create = _create
