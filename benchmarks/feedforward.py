import nest
import numpy as np

nest.Install("stdpmodule")
nest.set_verbosity("M_WARNING")

elements = 1000
resolution = 0.1 # ms
duration = 1000 # ms
weight = 1.0
delay = 2.0
spikes = np.arange(delay, duration + delay, resolution)

def generateSpikes(neurons, times):
    """Trigger spike to given neuron at specified times."""
    delay = resolution
    gen = nest.Create("spike_generator", 1, { "spike_times": [t-delay for t in times] })
    nest.Connect(gen, neurons, syn_spec = { "delay": delay })

def bench(config):

    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": resolution, "print_time": True})

    sources = nest.Create("parrot_neuron", elements)
    target = nest.Create("iaf_psc_delta")
    detector = nest.Create("spike_detector")

    generateSpikes(sources, spikes)
    nest.Connect(target, detector)

    if config == 1:
        syn_spec = {
            "model": "static_synapse",
            "delay": delay,
            "weight": weight,
        }
        nest.Connect(sources, target, syn_spec = syn_spec)

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
            "Wmin": weight,
            "weight": weight,
            "Wmax": weight,
        }
        nest.Connect(sources, target, syn_spec = stdp_synapse)

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
            "Wmin": weight,
            "weight": weight,
            "Wmax": weight,
        }

        min_delay = nest.GetKernelStatus('resolution')
        pre_syn_spec = { "delay": min_delay }
        syn_post_spec = {
            "weight": weight,
            "delay": delay - min_delay,
        }
        post_syn_spec = {
            "delay": delay - min_delay,
            "receptor_type": 1 # differentiate post-synaptic feedback
        }

        synapses = nest.Create("stdp_triplet_neuron", len(sources), params = stdp_neuron)
        nest.Connect(sources, synapses, 'one_to_one', pre_syn_spec)
        nest.Connect(synapses, target, 'all_to_all', syn_post_spec)
        nest.Connect(target, synapses, 'all_to_all', post_syn_spec)

    else:
        raise Exception('unknown config')

    nest.Simulate(duration + 2 * delay)

    print nest.GetStatus(detector, "n_events")[0]

if __name__ == '__main__':
    import timeit

    repetition = 1
    configs = [1, 2, 3]
    results = []

    for config in configs:
        measure = timeit.timeit(
                stmt = 'bench('+str(config)+')',
                setup = "from __main__ import bench",
                number = repetition
        )
        results.append(measure / repetition) # s

    print results