import random

import nest

nest.Install("stdpmodule")
nest.set_verbosity("M_WARNING")

def bench(config, order):

    simtime = 1000
    dt = 1.0
    delay = 2.0 # synaptic delay in ms

    g = 5.0 # ratio inhibitory weight/excitatory weight
    eta = 2.0 # external rate relative to threshold rate
    epsilon = 0.1 # connection probability
    NE = 4*order # number of excitatory neurons
    NI = 1*order # number of inhibitory neurons
    N_rec = 50 # record from 50 neurons
    CE = int(epsilon*NE) # number of excitatory synapses per neuron
    CI = int(epsilon*NI) # number of inhibitory synapses per neuron
    tauMem = 20.0 # time constant of membrane potential in ms
    theta = 20.0 # membrane threshold potential in mV
    neuron_params = {
        "C_m": 1.0,
        "tau_m": tauMem,
        "t_ref": 2.0,
        "E_L": 0.0,
        "V_reset": 0.0,
        "V_m": 0.0,
        "V_th": theta,
    }
    J = 0.1 # postsynaptic amplitude in mV
    J_ex = J # amplitude of excitatory postsynaptic potential
    J_in = -g*J_ex # amplitude of inhibitory postsynaptic potential
    nu_th = theta/(J*CE*tauMem)
    nu_ex = eta*nu_th
    p_rate = 1000.0*nu_ex*CE

    nest.ResetKernel()

    nest.SetKernelStatus({"resolution": dt, "print_time": True })
    nest.SetDefaults("iaf_psc_delta", neuron_params)
    nest.SetDefaults("poisson_generator",{"rate": p_rate})

    nodes_ex = nest.Create("iaf_psc_delta",NE)
    nodes_in = nest.Create("iaf_psc_delta",NI)
    noise    = nest.Create("poisson_generator")
    espikes  = nest.Create("spike_detector")
    ispikes  = nest.Create("spike_detector")

    nest.SetStatus(espikes,[{"label": "brunel-py-ex",
                             "withtime": True,
                             "withgid": True}])

    nest.SetStatus(ispikes,[{"label": "brunel-py-in",
                             "withtime": True,
                             "withgid": True}])

    nest.CopyModel("static_synapse","excitatory",{"weight":J_ex, "delay":delay})
    nest.CopyModel("static_synapse","inhibitory",{"weight":J_in, "delay":delay})

    nest.Connect(noise,nodes_ex, syn_spec="excitatory")
    nest.Connect(noise,nodes_in, syn_spec="excitatory")
    nest.Connect(nodes_ex[:N_rec], espikes, syn_spec="excitatory")
    nest.Connect(nodes_in[:N_rec], ispikes, syn_spec="excitatory")

    conn_params_ex = {'rule': 'fixed_indegree', 'indegree': CE}
    conn_params_in = {'rule': 'fixed_indegree', 'indegree': CI}

    if config == 1:
        nest.Connect(nodes_ex, nodes_ex+nodes_in, conn_params_ex, "excitatory")
        nest.Connect(nodes_in, nodes_ex+nodes_in, conn_params_in, "inhibitory")
    elif config == 2:
        stdp_synapse = {
            "model": "stdp_triplet_all_in_one_synapse",
            "tau_plus": 16.8,
            "tau_plus_triplet": 101.0,
            "tau_minus": 33.7,
            "tau_minus_triplet": 125.0,
            "Aplus": 5e-10,
            "Aminus": 7e-3,
            "Aplus_triplet": 6.2e-3,
            "Aminus_triplet": 2.3e-4,
            "Kplus": 0.0,
            "Kplus_triplet": 0.0,
            "Kminus": 0.0,
            "Kminus_triplet": 0.0,
            "delay": delay,
        }

        stdp_excitatory = stdp_synapse.copy()
        stdp_excitatory.update({
            "Wmin": J_ex,
            "weight": J_ex,
            "Wmax": J_ex,
        })
        stdp_inhibitory = stdp_synapse.copy()
        stdp_inhibitory.update({
            "Wmin": J_in,
            "weight": J_in,
            "Wmax": J_in,
        })

        nest.Connect(nodes_ex, nodes_ex+nodes_in, conn_params_ex, stdp_excitatory)
        nest.Connect(nodes_in, nodes_ex+nodes_in, conn_params_in, stdp_inhibitory)
    elif config == 3:
        stdp_neuron = {
            "tau_plus": 16.8,
            "tau_plus_triplet": 101.0,
            "tau_minus": 33.7,
            "tau_minus_triplet": 125.0,
            "Aplus": 5e-10,
            "Aminus": 7e-3,
            "Aplus_triplet": 6.2e-3,
            "Aminus_triplet": 2.3e-4,
            "Kplus": 0.0,
            "Kplus_triplet": 0.0,
            "Kminus": 0.0,
            "Kminus_triplet": 0.0,
            "nearest_spike": False,
        }

        stdp_excitatory = stdp_neuron.copy()
        stdp_excitatory.update({
            "Wmin": J_ex,
            "weight": J_ex,
            "Wmax": J_ex,
        })
        stdp_inhibitory = stdp_neuron.copy()
        stdp_inhibitory.update({
            "Wmin": J_in,
            "weight": J_in,
            "Wmax": J_in,
        })

        min_delay = nest.GetKernelStatus('resolution')
        pre_syn_spec = { "delay": min_delay }
        post_syn_spec = {
            "delay": delay - min_delay,
            "receptor_type": 1 # differentiate post-synaptic feedback
        }

        for post in nodes_ex+nodes_in:
            syn_post_spec = {
                "weight": J_ex,
                "delay": delay - min_delay
            }
            selected_ex = random.sample(filter(lambda n: n != post, nodes_ex), CE)
            synapses_ex = nest.Create("stdp_triplet_neuron", CE, params = stdp_excitatory)
            nest.Connect(selected_ex, synapses_ex, 'one_to_one', pre_syn_spec)
            nest.Connect(synapses_ex, (post,), 'all_to_all', syn_post_spec)
            nest.Connect((post,), synapses_ex, 'all_to_all', post_syn_spec)

        for post in nodes_ex+nodes_in:
            syn_post_spec = {
                "weight": J_in,
                "delay": delay - min_delay
            }
            selected_in = random.sample(filter(lambda n: n != post, nodes_in), CI)
            synapses_in = nest.Create("stdp_triplet_neuron", CI, params = stdp_inhibitory)
            nest.Connect(selected_in, synapses_in, 'one_to_one', pre_syn_spec)
            nest.Connect(synapses_in, (post,), 'all_to_all', syn_post_spec)
            nest.Connect((post,), synapses_in, 'all_to_all', post_syn_spec)

    else:
        raise Exception('unknown config')

    nest.Simulate(simtime)

    #print nest.GetStatus(espikes, "n_events")[0]
    #print nest.GetStatus(ispikes, "n_events")[0]

if __name__ == '__main__':
    import timeit

    repetition = 5
    configs = [1, 2, 3]

    results = []
    orders = [250, 500, 1000]
    orders.reverse()

    for order in orders:
        temp = []
        for config in configs:
            measure = timeit.timeit(
                    stmt = 'bench('+str(config)+','+str(order)+')',
                    setup = "from __main__ import bench",
                    number = repetition
            )
            temp.append(measure / repetition) # s
        results.append(temp)

    print
    print zip(orders, results)
