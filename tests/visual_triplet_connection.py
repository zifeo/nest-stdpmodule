import math
import nest
import pylab as plt
import numpy as np

nest.Install("stdpmodule")
nest.set_verbosity("M_WARNING")

def reset():
    nest.ResetKernel()
    nest.SetKernelStatus({"local_num_threads" : 1, "resolution" : 0.1, "print_time": False})
    nest.CopyModel("dc_generator", "spike_trigger", params = {
        "amplitude": 3920.0,
    })

def generateSpikes(neuron, times):
    for t in times:
        generator = nest.Create("spike_trigger", params = {
            "start": t - 2.0,
            "stop": t,
        })
        nest.Connect(generator, neuron)

def create(model, number):
    return map(lambda x: (x,), nest.Create(model, number))


# Settings
neuron_model = "iaf_neuron"
synapse_model = "stdp_triplet_synapse"
syn_spec = {
    "model": synapse_model,
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
multi_params = {
    "withtime": True,
    "record_from": ["V_m"],
}

reset()
simulationDuration = 1000.0
preSpikeTimes = [500.0]
postSpikeTimes = [400.0, 600.0]

# Circuit setup
multi = nest.Create("multimeter")
(neuronPost, neuronPre) = create(neuron_model, 2)
(spikeDetectorPre, spikeDetectorPost) = create("spike_detector", 2)

nest.SetStatus(multi, params = multi_params)

nest.Connect(multi, neuronPre)
nest.Connect(neuronPre, neuronPost, syn_spec = syn_spec)
nest.Connect(neuronPre, spikeDetectorPre)
nest.Connect(neuronPost, spikeDetectorPost)

generateSpikes(neuronPre, preSpikeTimes)
generateSpikes(neuronPost, postSpikeTimes)

# Simulation
current = 0
dt = 1
time = [0.0]

r1Sim = [0.0]
r2Sim = [0.0]
o1Sim = [0.0]
o2Sim = [0.0]
r1DecayRate = math.exp(- dt / syn_spec["tau_plus"])
r2DecayRate = math.exp(- dt / syn_spec["tau_x"])
o1DecayRate = math.exp(- dt / syn_spec["tau_minus"])
o2DecayRate = math.exp(- dt / syn_spec["tau_y"])

r1 = [0.0]
r2 = [0.0]
o1 = [0.0]
o2 = [0.0]
weight = [5.0]

while current < simulationDuration:
    connectionStats = nest.GetConnections(neuronPre, synapse_model = synapse_model)
    vars = nest.GetStatus(connectionStats, ["r1", "r2", "o1", "o2"])[0]

    r1.append(vars[0])
    r2.append(vars[1])
    o1.append(vars[2])
    o2.append(vars[3])

    if (r1[-2] == r1[-1]): r1Sim.append(r1Sim[-1] * r1DecayRate)
    else: r1Sim.append(r1[-1])

    if (r2[-2] == r2[-1]): r2Sim.append(r2Sim[-1] * r2DecayRate)
    else: r2Sim.append(r2[-1])

    if (o1[-2] == o1[-1]): o1Sim.append(o1Sim[-1] * o1DecayRate)
    else: o1Sim.append(o1[-1])

    if (o2[-2] == o2[-1]): o2Sim.append(o2Sim[-1] * o2DecayRate)
    else: o2Sim.append(o2[-1])

    weight.append(nest.GetStatus(connectionStats, ["weight"])[0])
    time.append(current)

    current += dt
    nest.Simulate(dt)

spikingStatsPre = nest.GetStatus(spikeDetectorPre, keys = "events")[0]
spikingStatsPost = nest.GetStatus(spikeDetectorPost, keys = "events")[0]

# Plot

plt.figure()
plt.subplots_adjust(hspace = 0.3)
ylim = [-0.5, 2]

p1 = plt.subplot(211)
plt.title("Pre-synaptic spikes and variables")
plt.plot(time, r1, "b")
plt.plot(time, r2, "r")
plt.plot(time, r1Sim, "b")
plt.plot(time, r2Sim, "r")
plt.legend(["r1", "r2"], loc = "center left", bbox_to_anchor = (1, 0.5))
plt.ylim(ylim)
plt.eventplot(spikingStatsPre["times"], orientation = "horizontal", colors = "k", linelengths = 5)

plt.subplot(212, sharex = p1)
plt.title("Post-synaptic spikes and variables")
plt.plot(time, o1, "b")
plt.plot(time, o2, "r")
plt.plot(time, o1Sim, "b")
plt.plot(time, o2Sim, "r")
plt.legend(["o1", "o2"], loc = "center left", bbox_to_anchor = (1, 0.5))
plt.ylim(ylim)
plt.eventplot(spikingStatsPost["times"], orientation = "horizontal", colors = "k", linelengths = 5)

plt.show()
