import nest
import pylab as plt
import numpy as np

"""
Reproduce result of the pairing experiment from Pfister-Gerstner (2006) with the triplet model.
"""

nest.Install("stdpmodule")
nest.set_verbosity("M_WARNING")

def generateSpikes(neuron, times):
    """Trigger spike to given neuron at specified times."""
    delay = 1.0
    gen = nest.Create("spike_generator", 1, { "spike_times": [t - delay for t in times] })
    nest.Connect(gen, neuron, syn_spec = { "delay": delay })

def create(model, number):
    """Allow multiple model instance to be unpack as they are created."""
    return map(lambda x: (x,), nest.Create(model, number))

neuron_model = "parrot_neuron"
synapse_model = "stdp_triplet_all_in_one_synapse"
syn_spec = {
    "model": synapse_model,
    "receptor_type": 1, # set receptor 1 post-synaptically, to not generate extra spikes
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
}

n = 60 # pair of presynaptic and post synpatic spikes
dt = 10 # ms shift pre/post
start_spikes = dt + 20
rhos = np.arange(1.0, 55.0, 5.0) # hz spiking frequence
weights_plus = []
weights_minus = []

def evaluate(rho, dt):
    """Evaluate connection change of weight and returns it."""
    nest.ResetKernel()
    nest.SetKernelStatus({"local_num_threads" : 1, "resolution" : 0.1, "print_time": False})

    step = 1000.0 / rho
    simulation_duration = np.ceil(n * step)
    times_pre = np.arange(start_spikes, simulation_duration, step).round(1)
    times_post = [t + dt for t in times_pre]

    # Entities
    neuron_pre = nest.Create(neuron_model)
    neuron_post = nest.Create(neuron_model)

    # Connections
    generateSpikes(neuron_pre, times_pre)
    generateSpikes(neuron_post, times_post)
    nest.Connect(neuron_pre, neuron_post, syn_spec = syn_spec)

    # Simulation
    connection_stats = nest.GetConnections(neuron_pre, synapse_model = synapse_model)
    current_weight = nest.GetStatus(connection_stats, ["weight"])[0][0]
    nest.Simulate(start_spikes + simulation_duration)

    # Results
    connection_stats = nest.GetConnections(neuron_pre, synapse_model = synapse_model)
    end_weight = nest.GetStatus(connection_stats, ["weight"])[0][0]
    return end_weight - current_weight

for rho in rhos:
    weights_plus.append(evaluate(rho, dt))
    weights_minus.append(evaluate(rho, -dt))

plt.figure()
plt.title('Pairing experiment (Pfister-Gerstner 2006)')
plt.xlabel("rho (Hz)")
plt.ylabel("weight delta")
plt.plot(rhos, weights_plus, "b")
plt.plot(rhos, weights_minus, "b", ls = "--")
plt.legend(["dt = +10 ms", "dt = -10 ms"], loc = "upper left", frameon = False)
plt.xlim([0, 50])
plt.ylim([-0.6, 0.8])
plt.show()
