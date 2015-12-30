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

            if conn_spec != None and conn_spec != 'all_to_all' and conn_spec != 'one_to_one':
                raise nest.NESTError('Unsupported conn_spec for stdp dsl: %s' % conn_spec)

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

            connectionCount = len(post) if conn_spec == 'one_to_one' else len(post)**2
            synapse = nest.Create("stdp_triplet_neuron", connectionCount, params = syn_spec)

            nest_connect(pre, synapse, conn_spec, pre_syn_spec)
            nest_connect(synapse, post, 'one_to_one', syn_post_spec)
            nest_connect(post, synapse, 'one_to_one', post_syn_spec)
            return synapse

        else:

            return nest_connect(pre, post, conn_spec, syn_spec, model)

    def _spikes(neurons, times):
        """Trigger spike to given neurons at specified times."""
        delay = nest.GetKernelStatus('resolution')
        gen = nest.Create("spike_generator", 1, { "spike_times": [t-delay for t in times] })
        nest.Connect(gen, neurons, syn_spec = { "delay": delay })

    nest.Connect = _connect
    nest.Spikes = _spikes