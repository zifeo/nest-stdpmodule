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
    """Enable all spiking-time dependent plasticity features."""

    nest.Install("stdpmodule")

    # save old functions
    nest_connect = nest.Connect

    def _connect(pre, post, conn_spec = None, syn_spec = None, model = None):
        """Wrap nest connect and allow to create fake neuron-synapse."""

        if (model == "stdp_triplet_neuron"):

            synapse = nest.Create("stdp_triplet_neuron", params = syn_spec)
            nest_connect(pre, synapse)
            nest_connect(synapse, post)
            nest_connect(post, synapse, syn_spec = { "receptor_type": 1 }) # differentiate post-synaptic feedback
            return synapse

        else:

            return nest_connect(pre, post, conn_spec, syn_spec, model)

    def _spikes(neuron, times):
        """Trigger spike to given neuron at specified times."""
        delay = 1.0
        gen = nest.Create("spike_generator", 1, { "spike_times": [t-delay for t in times] })
        nest.Connect(gen, neuron, syn_spec = { "delay": delay })

    nest.Connect = _connect
    nest.Spikes = _spikes