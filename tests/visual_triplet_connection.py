import math
import nest
import pylab as plt

nest.Install("stdpmodule")
nest.set_verbosity("M_WARNING")

def generateSpikes(neuron, times):
    """Trigger spike to given neuron at specified times."""
    delay = 1.0
    gen = nest.Create("spike_generator", 1, { "spike_times": [t-delay for t in times] })
    nest.Connect(gen, neuron, syn_spec = { "delay": delay })

nest.set_verbosity('M_WARNING')
nest.ResetKernel()

# settings
synapse_model = "stdp_triplet_all_in_one_synapse"
syn_spec = {
    "model": synapse_model,
    "receptor_type": 1, # set receptor 1 post-synaptically, to not generate extra spikes
    "weight": 5.0,
    "tau_plus": 16.8,
    "tau_plus_triplet": 101.0,
    "tau_minus": 33.7,
    "tau_minus_triplet": 125.0,
    "Aplus": 0.1,
    "Aminus": 0.1,
    "Aplus_triplet": 0.1,
    "Aminus_triplet": 0.1,
    "Kplus": 0.0,
    "Kplus_triplet": 0.0,
    "Kminus": 0.0,
    "Kminus_triplet": 0.0,
}
simulation_duration = 1000.0
times_pre = [500.0, 680.0]
times_post = [400.0, 600.0]

# setup circuit
neuron_pre = nest.Create("parrot_neuron")
neuron_post = nest.Create("parrot_neuron")
detector_pre = nest.Create("spike_detector")
detector_post = nest.Create("spike_detector")

generateSpikes(neuron_pre, times_pre)
generateSpikes(neuron_post, times_post)

nest.Connect(neuron_pre, neuron_post, syn_spec = syn_spec)
nest.Connect(neuron_pre, detector_pre)
nest.Connect(neuron_post, detector_post)

# simulation
current = 0
time = [0.0]
r1_decay_rate = math.exp(- 1.0 / syn_spec["tau_plus"])
r2_decay_rate = math.exp(- 1.0 / syn_spec["tau_plus_triplet"])
o1_decay_rate = math.exp(- 1.0 / syn_spec["tau_minus"])
o2_decay_rate = math.exp(- 1.0 / syn_spec["tau_minus_triplet"])

r1 = [0.0]
r2 = [0.0]
o1 = [0.0]
o2 = [0.0]
r1_sim = [0.0]
r2_sim = [0.0]
o1_sim = [0.0]
o2_sim = [0.0]

while current < simulation_duration:
    connection_stats = nest.GetConnections(neuron_pre, synapse_model = synapse_model)
    vars = nest.GetStatus(connection_stats, ["Kplus", "Kplus_triplet", "Kminus", "Kminus_triplet"])[0]

    # get variables from NEST
    r1.append(vars[0])
    r2.append(vars[1])
    o1.append(vars[2])
    o2.append(vars[3])

    # simulate variable behavior, -1 is for dendritic delay
    if (current - 1 in times_pre):
        r1_sim.append(r1_sim[-1] + 1.0)
        r2_sim.append(r2_sim[-1] + 1.0)
    else:
        r1_sim.append(r1_sim[-1] * r1_decay_rate)
        r2_sim.append(r2_sim[-1] * r2_decay_rate)

    if (current - 1 in times_post):
        o1_sim.append(o1_sim[-1] + 1.0)
        o2_sim.append(o2_sim[-1] + 1.0)
    else:
        o1_sim.append(o1_sim[-1] * o1_decay_rate)
        o2_sim.append(o2_sim[-1] * o2_decay_rate)

    time.append(current)
    current += 1
    nest.Simulate(1)

# get spikes
spiking_stats_pre = nest.GetStatus(detector_pre, keys ="events")[0]
spiking_stats_post = nest.GetStatus(detector_post, keys ="events")[0]

# plot
plt.figure()
plt.subplots_adjust(hspace = 0.3)
ylim = [-0.5, 2]

p1 = plt.subplot(211)
plt.title("Pre-synaptic spikes and variables")
plt.plot(time, r1, "b")
plt.plot(time, r2, "r")
plt.plot(time, r1_sim, "b", ls ="--")
plt.plot(time, r2_sim, "r", ls ="--")
plt.legend(["r1", "r2", "r1 simulated", "r2 simulated"], loc = "upper left", frameon = False)
plt.ylim(ylim)
plt.eventplot(spiking_stats_pre["times"], orientation ="horizontal", colors ="k", linelengths = 5)

plt.subplot(212, sharex = p1)
plt.title("Post-synaptic spikes and variables")
plt.plot(time, o1, "b")
plt.plot(time, o2, "r")
plt.plot(time, o1_sim, "b", ls ="--")
plt.plot(time, o2_sim, "r", ls ="--")
plt.legend(["o1", "o2", "o1 simulated", "o2 simulated"], loc = "upper left", frameon = False)
plt.ylim(ylim)
plt.eventplot(spiking_stats_post["times"], orientation ="horizontal", colors ="k", linelengths = 5)

plt.show()
