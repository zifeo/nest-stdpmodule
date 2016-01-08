import nest
import pylab as plt
import numpy as np

"""
Reproduce result of the pairing experiment from Pfister-Gerstner (2006) with the triplet model.
"""

nest.HelloSTDP()
nest.set_verbosity("M_WARNING")

all_to_all_syn_spec = {
    "weight": 1.0,
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
    "axonal_delay": 0.1,
    "dendritic_delay": 0.9,
    "nearest_spike": False,
}

nearest_syn_spec = {
    "weight": 1.0,
    "tau_plus": 16.8,
    "tau_plus_triplet": 714.0,
    "tau_minus": 33.7,
    "tau_minus_triplet": 40.0,
    "Aplus": 8.8e-11,
    "Aminus": 6.6e-3,
    "Aplus_triplet": 5.3e-2,
    "Aminus_triplet": 3.1e-3,
    "Kplus": 0.0,
    "Kplus_triplet": 0.0,
    "Kminus": 0.0,
    "Kminus_triplet": 0.0,
    "axonal_delay": 0.1,
    "dendritic_delay": 0.9,
    "nearest_spike": True,
}

n = 60 # pair of presynaptic and post synpatic spikes
dt = 10 # ms shift pre/post
start_spikes = dt + 20
rhos = np.arange(1.0, 55.0, 5.0) # hz spiking frequence
weights_plus = []
weights_minus = []
weights_plus_nearest = []
weights_minus_nearest = []

def evaluate(rho, dt, syn_spec):
    """Evaluate connection change of weight and returns it."""
    nest.ResetKernel()
    nest.SetKernelStatus({"local_num_threads" : 1, "resolution" : 0.1, "print_time": False})

    step = 1000.0 / rho
    simulation_duration = np.ceil(n * step)
    times_pre = np.arange(start_spikes, simulation_duration, step).round(1)
    times_post = [t + dt for t in times_pre]

    # Entities
    neuron_pre = nest.Create("parrot_neuron")
    neuron_post = nest.Create("parrot_neuron")

    # Connections
    nest.Spikes(neuron_pre, times_pre)
    nest.Spikes(neuron_post, times_post)
    triplet_synapse = nest.Connect(neuron_pre, neuron_post, model = "stdp_triplet_node", syn_spec = syn_spec,
                                   syn_post_spec = { "receptor_type": 1 }) # avoid repeating spike on post parrot neuron

    # Simulation
    current_weight = nest.GetStatus(triplet_synapse, keys = "weight")[0]
    nest.Simulate(start_spikes + simulation_duration)

    # Results
    end_weight = nest.GetStatus(triplet_synapse, keys = "weight")[0]
    return end_weight - current_weight

for rho in rhos:
    weights_plus.append(evaluate(rho, dt, all_to_all_syn_spec))
    weights_minus.append(evaluate(rho, -dt, all_to_all_syn_spec))
    weights_plus_nearest.append(evaluate(rho, dt, nearest_syn_spec))
    weights_minus_nearest.append(evaluate(rho, -dt, nearest_syn_spec))

plt.figure()
plt.title('Pairing experiment (Pfister-Gerstner 2006)')
plt.xlabel("rho (Hz)")
plt.ylabel("weight delta")
plt.plot(rhos, weights_plus, "b")
plt.plot(rhos, weights_minus, "b", ls = "--")
plt.plot(rhos, weights_plus_nearest, "r")
plt.plot(rhos, weights_minus_nearest, "r", ls = "--")
plt.legend(["dt +10 ms", "dt -10 ms", "dt +10 ms nearest", "dt -10 ms nearest"], loc = "upper left", frameon = False)
plt.xlim([0, 50])
plt.ylim([-0.6, 0.8])
plt.show()
