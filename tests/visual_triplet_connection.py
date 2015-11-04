import math
import nest
import pylab as plt

nest.Install("stdpmodule")
nest.set_verbosity("M_WARNING")

def reset():
    """Reset NEST and add spike_trigger model."""
    nest.ResetKernel()
    nest.SetKernelStatus({"local_num_threads" : 1, "resolution" : 0.1, "print_time": False})

def generateSpikes(neuron, times):
    """Trigger spike to given neuron at specified times."""
    gen = nest.Create("spike_generator", 1, { "spike_times": times })
    nest.Connect(gen, neuron)

def create(model, number):
    """Allow multiple model instance to be unpack as they are created."""
    return map(lambda x: (x,), nest.Create(model, number))

# settings
reset()
neuron_model = "parrot_neuron"
synapse_model = "stdp_triplet_all_in_one_synapse"
syn_spec = {
    "model": synapse_model,
    "receptor_type": 1,     # set receptor 1 post-synaptically, to not generate extra spikes
    "weight": 5.0,          # 5.0
    "tau_plus": 16.8,       # 16.8
    "tau_x": 101.0,         # 101.0
    "tau_minus": 33.7,      # 33.7
    "tau_y": 125.0,         # 125.0
    "a2_plus": 1.0,         # 1.0
    "a2_minus": 1.0,        # 1.0
    "a3_plus": 0.0,         # 1.0
    "a3_minus": 0.0,        # 1.0
    "r1": 0.0,              # 0.0
    "r2": 0.0,              # 0.0
    "o1": 0.0,              # 0.0
    "o2": 0.0,              # 0.0
}
simulationDuration = 1000.0
preSpikeTimes = [500.0, 680.0]
postSpikeTimes = [400.0, 600.0]

# setup circuit
(neuronPost, neuronPre) = create(neuron_model, 2)
(spikeDetectorPre, spikeDetectorPost) = create("spike_detector", 2)

generateSpikes(neuronPre, preSpikeTimes)
generateSpikes(neuronPost, postSpikeTimes)

nest.Connect(neuronPre, neuronPost, syn_spec = syn_spec)
nest.Connect(neuronPre, spikeDetectorPre)
nest.Connect(neuronPost, spikeDetectorPost)

# simulation
current = 0
time = [0.0]
r1DecayRate = math.exp(- 1.0 / syn_spec["tau_plus"])
r2DecayRate = math.exp(- 1.0 / syn_spec["tau_x"])
o1DecayRate = math.exp(- 1.0 / syn_spec["tau_minus"])
o2DecayRate = math.exp(- 1.0 / syn_spec["tau_y"])

r1 = [0.0]
r2 = [0.0]
o1 = [0.0]
o2 = [0.0]
r1Sim = [0.0]
r2Sim = [0.0]
o1Sim = [0.0]
o2Sim = [0.0]

while current < simulationDuration:
    connectionStats = nest.GetConnections(neuronPre, synapse_model = synapse_model)
    vars = nest.GetStatus(connectionStats, ["r1", "r2", "o1", "o2"])[0]

    # get variables from NEST
    r1.append(vars[0])
    r2.append(vars[1])
    o1.append(vars[2])
    o2.append(vars[3])

    # simulate variable behavior
    if (current in preSpikeTimes):
        r1Sim.append(r1Sim[-1] + 1.0)
        r2Sim.append(r2Sim[-1] + 1.0)
    else:
        r1Sim.append(r1Sim[-1] * r1DecayRate)
        r2Sim.append(r2Sim[-1] * r2DecayRate)

    if (current in postSpikeTimes):
        o1Sim.append(o1Sim[-1] + 1.0)
        o2Sim.append(o2Sim[-1] + 1.0)
    else:
        o1Sim.append(o1Sim[-1] * o1DecayRate)
        o2Sim.append(o2Sim[-1] * o2DecayRate)

    time.append(current)
    current += 1
    nest.Simulate(1)

# get spikes
spikingStatsPre = nest.GetStatus(spikeDetectorPre, keys = "events")[0]
spikingStatsPost = nest.GetStatus(spikeDetectorPost, keys = "events")[0]

# plot
plt.figure()
plt.subplots_adjust(hspace = 0.3)
ylim = [-0.5, 2]

p1 = plt.subplot(211)
plt.title("Pre-synaptic spikes and variables")
plt.plot(time, r1, "b")
plt.plot(time, r2, "r")
plt.plot(time, r1Sim, "b", ls = "--")
plt.plot(time, r2Sim, "r", ls = "--")
plt.legend(["r1", "r2", "r1 simulated", "r2 simulated"], loc = "upper left", frameon = False)
plt.ylim(ylim)
plt.eventplot(spikingStatsPre["times"], orientation = "horizontal", colors = "k", linelengths = 5)

plt.subplot(212, sharex = p1)
plt.title("Post-synaptic spikes and variables")
plt.plot(time, o1, "b")
plt.plot(time, o2, "r")
plt.plot(time, o1Sim, "b", ls = "--")
plt.plot(time, o2Sim, "r", ls = "--")
plt.legend(["o1", "o2", "o1 simulated", "o2 simulated"], loc = "upper left", frameon = False)
plt.ylim(ylim)
plt.eventplot(spikingStatsPost["times"], orientation = "horizontal", colors = "k", linelengths = 5)

plt.show()
