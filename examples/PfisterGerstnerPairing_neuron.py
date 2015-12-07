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

syn_spec = {
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
    "nearest_spike": True,
}

n = 60 # pair of presynaptic and post synpatic spikes
dt = 10 # ms shift pre/post
start_spikes = dt + 20
rhos = np.arange(1.0, 100.0, 5.0) # hz spiking frequence
weights_plus = []
weights_minus = []
weights_plus_nearest = []
weights_minus_nearest = []

def evaluate(rho, dt, nearest):
    """Evaluate connection change of weight and returns it."""
    nest.ResetKernel()
    nest.SetKernelStatus({"local_num_threads" : 1, "resolution" : 0.1, "print_time": False})

    step = 1000.0 / rho
    simulation_duration = np.ceil(n * step)
    times_pre = np.arange(start_spikes, simulation_duration, step).round(1)
    times_post = [t + dt for t in times_pre]

    # Entities
    local_spec = syn_spec.copy()
    local_spec.update({ "nearest_spike": nearest })
    neuron_pre = nest.Create("parrot_neuron")
    neuron_post = nest.Create("parrot_neuron")
    triplet_synapse = nest.Create("stdp_triplet_neuron", params = local_spec)

    # Connections
    generateSpikes(neuron_pre, times_pre)
    generateSpikes(neuron_post, times_post)
    nest.Connect(neuron_pre, triplet_synapse)
    nest.Connect(triplet_synapse, neuron_post, syn_spec = { "receptor_type": 1 }) # do not repeat spike
    nest.Connect(neuron_post, triplet_synapse, syn_spec = { "receptor_type": 1 }) # differentiate post-synaptic feedback

    # Simulation
    current_weight = nest.GetStatus(triplet_synapse, keys = "weight")[0]
    nest.Simulate(start_spikes + simulation_duration)

    # Results
    end_weight = nest.GetStatus(triplet_synapse, keys = "weight")[0]
    return end_weight - current_weight

for rho in rhos:
    weights_plus.append(evaluate(rho, dt, False))
    weights_minus.append(evaluate(rho, -dt, False))
    weights_plus_nearest.append(evaluate(rho, dt, True))
    weights_minus_nearest.append(evaluate(rho, -dt, True))

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
