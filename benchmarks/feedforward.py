import nest
import numpy as np

nest.set_verbosity(10000)

def bench():

    def generateSpikes(neurons, times):
        """Trigger spike to given neuron at specified times."""
        delay = 1.0
        gen = nest.Create("spike_generator", 1, { "spike_times": [t-delay for t in times] })
        nest.Connect(gen, neurons, syn_spec = { "delay": delay })

    elements = 1000
    resolution = 0.1 # ms
    duration = 100 # ms
    spikes = np.arange(1.1, 200, 0.1)

    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": resolution, "print_time": False})

    syn_spec = {
        "model": "static_synapse",
    }

    sources = nest.Create("parrot_neuron", elements)
    target = nest.Create("parrot_neuron")
    detector = nest.Create("spike_detector")

    generateSpikes(sources, spikes)

    nest.Connect(sources, target, 'all_to_all', syn_spec = syn_spec)
    nest.Connect(target, detector)

    nest.Simulate(duration)

    print len(nest.GetStatus(detector, keys = "events")[0]["times"])

if __name__ == '__main__':
    import timeit
    print timeit.timeit("bench()", setup = "from __main__ import bench", number = 20) / 20 # s
