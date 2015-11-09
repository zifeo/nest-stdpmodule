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

def evaluate(spikePairs, rho, dt):
    """Evaluate connection change of weight and returns it."""
    nest.ResetKernel()
    nest.SetKernelStatus({"local_num_threads" : 1, "resolution" : 0.1, "print_time": False})

    step = round(1000.0 / rho)
    simulationDuration = spikePairs * step
    preSpikeTimes = np.arange(startSpikes, simulationDuration, step)
    postSpikeTimes = map(lambda t: t + dt, preSpikeTimes)

    # Entities
    (neuronPre, neuronPost) = create(neuron_model, 2)

    # Connections
    generateSpikes(neuronPre, preSpikeTimes)
    generateSpikes(neuronPost, postSpikeTimes)
    nest.Connect(neuronPre, neuronPost, syn_spec = syn_spec)

    # Simulation
    connectionStats = nest.GetConnections(neuronPre, synapse_model = synapse_model)
    currentWeight = nest.GetStatus(connectionStats, ["weight"])[0]
    nest.Simulate(startSpikes + simulationDuration)

    # Results
    connectionStats = nest.GetConnections(neuronPre, synapse_model = synapse_model)
    endWeight = nest.GetStatus(connectionStats, ["weight"])[0]
    return endWeight[0] - currentWeight[0]

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
startSpikes = dt + 20
rhos = range(1, 51, 2) # hz spiking frequence
weights_plus = []
weights_minus = []

for rho in rhos:
    weights_plus.append(evaluate(n, rho, dt))
    weights_minus.append(evaluate(n, rho, -dt))

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
