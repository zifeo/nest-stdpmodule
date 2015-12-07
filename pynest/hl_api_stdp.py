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

    def _connect(pre, post, conn_spec = None, syn_spec = None, model = None, pre_syn_spec = None, syn_post_spec = None):
        """Wrap nest connect and allow to create fake neuron-synapse."""

        if (model == "stdp_triplet_neuron"):

            pre_syn_spec = {} if pre_syn_spec is None else pre_syn_spec.copy()
            syn_post_spec = {} if syn_post_spec is None else syn_post_spec.copy()
            syn_spec = {} if syn_spec is None else syn_spec.copy()

            resolution = nest.GetKernelStatus()["resolution"]
            axonal_delay = syn_spec.pop("axonal_delay", resolution)
            dendritic_delay = syn_spec.pop("dendritic_delay", resolution)

            pre_syn_spec.update({ "delay": axonal_delay })
            syn_post_spec.update({ "delay": dendritic_delay })
            post_syn_spec = {
                "delay": dendritic_delay,
                "receptor_type": 1 # differentiate post-synaptic feedback
            }

            synapse = nest.Create("stdp_triplet_neuron", params = syn_spec)
            nest_connect(pre, synapse, syn_spec = pre_syn_spec)
            nest_connect(synapse, post, syn_spec = syn_post_spec)
            nest_connect(post, synapse, syn_spec = post_syn_spec)
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