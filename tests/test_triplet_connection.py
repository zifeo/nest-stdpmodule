import math
import nest
import unittest

@nest.check_stack
class STDPTripletConnectionTestCase(unittest.TestCase):
    """Check stdp_triplet_connection spike"""

    @classmethod
    def setUpClass(self):
        nest.Install("stdpmodule")

    def setUp(self):
        nest.set_verbosity(100)
        nest.ResetKernel()
        nest.CopyModel("dc_generator", "spike_trigger", params = {
            "amplitude": 3920.0,
        })

        # settings
        self.dendritic_delay = 1.0 # should be no longer than 3 in order to avoid tests mixing
        self.decay_duration = 5.0
        self.synapse_model = "stdp_triplet_synapse"
        self.syn_spec = {
            "model": self.synapse_model,
            "delay": self.dendritic_delay,
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

        # Circuit setup
        self.pre_neuron = nest.Create("iaf_neuron")
        self.post_neuron = nest.Create("iaf_neuron")
        nest.Connect(self.pre_neuron, self.post_neuron, syn_spec = self.syn_spec)

    def synapseStatus(self, which):
        stats = nest.GetConnections(self.pre_neuron, synapse_model = self.synapse_model)
        return nest.GetStatus(stats, [which])[0][0]

    def assertEqD(self, given, expected, message):
        assert abs(expected - given) < 1e-9, "%s (expected: `%s` was: `%s`" % (message, str(expected), str(given))

    def generateSpikes(self, neuron, times):
        for t in times:
            generator = nest.Create("spike_trigger", params = {
                "start": t - 2.0,
                "stop": t,
            })
            nest.Connect(generator, neuron)

    def test_badPropertiesSetupsThrowExceptions(self):
        def setupProperty(property):
            bad_syn_spec = self.syn_spec.copy()
            bad_syn_spec.update(property)
            nest.Connect(self.pre_neuron, self.post_neuron, syn_spec = bad_syn_spec)

        self.assertRaisesRegexp(nest.NESTError, "BadProperty(.+)r1", setupProperty, {"r1": -1.0})
        self.assertRaisesRegexp(nest.NESTError, "BadProperty(.+)r2", setupProperty, {"r2": -1.0})
        self.assertRaisesRegexp(nest.NESTError, "BadProperty(.+)o1", setupProperty, {"o1": -1.0})
        self.assertRaisesRegexp(nest.NESTError, "BadProperty(.+)o2", setupProperty, {"o2": -1.0})
        self.assertRaisesRegexp(nest.NESTError, "BadProperty(.+)tau_x(.+)tau_plus", setupProperty,
                                {"tau_plus": 1.0, "tau_x": 1.0})
        self.assertRaisesRegexp(nest.NESTError, "BadProperty(.+)tau_y(.+)tau_minus", setupProperty,
                                {"tau_minus": 1.0, "tau_y": 1.0})

    def test_preVarsIncreaseWithPreSpike(self):
        """Check that pre-synaptic variables (r1, r2) increase after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [10.0])

        nest.Simulate(5.0)
        self.assertEqD(self.synapseStatus("r1"), 0.0, "r1 should be zero at start")
        self.assertEqD(self.synapseStatus("r2"), 0.0, "r2 should be zero at start")

        nest.Simulate(10.0)
        self.assertEqD(self.synapseStatus("r1"), 1.0, "r1 should have increased by 1 after spike")
        self.assertEqD(self.synapseStatus("r2"), 1.0, "r2 should have increased by 1 after spike")

    def test_postVarsIncreaseWithPostSpike(self):
        """Check that post-synaptic variables (o1, o2) increase after each post-synaptic spike."""

        self.generateSpikes(self.post_neuron, [10.0])
        self.generateSpikes(self.pre_neuron, [10.0 + self.dendritic_delay])
        # last one activates vars computations

        nest.Simulate(5.0)
        self.assertEqD(self.synapseStatus("o1"), 0.0, "o1 should be zero at start")
        self.assertEqD(self.synapseStatus("o2"), 0.0, "o2 should be zero at start")

        nest.Simulate(10.0)
        self.assertEqD(self.synapseStatus("o1"), 1.0, "o1 should have increased by 1 after spike")
        self.assertEqD(self.synapseStatus("o2"), 1.0, "o2 should have increased by 1 after spike")

    def test_preVarsDecayAfterPreSpike(self):
        """Check that pre-synaptic variables (r1, r2) decay after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [10.0, 10.0 + self.decay_duration])
        # last one activates vars computations

        nest.Simulate(5.0)
        self.assertEqD(self.synapseStatus("r1"), 0.0, "r1 should be zero at start")
        self.assertEqD(self.synapseStatus("r2"), 0.0, "r2 should be zero at start")

        decayedR1 = math.exp(- self.decay_duration / self.syn_spec["tau_plus"])
        decayedR2 = math.exp(- self.decay_duration / self.syn_spec["tau_x"])

        nest.Simulate(20.0)
        self.assertEqD(self.synapseStatus("r1"), 1.0 + decayedR1, "r1 should have decay after spike")
        self.assertEqD(self.synapseStatus("r2"), 1.0 + decayedR2, "r2 should have decay after spike")

    def test_preVarsDecayAfterPostSpike(self):
        """Check that pre-synaptic variables (r1, r2) decay after each post-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [10.0, 10.0 + self.decay_duration])
        self.generateSpikes(self.post_neuron, [10.0 + self.decay_duration / 2 - self.dendritic_delay])
        # last one activates vars computations

        nest.Simulate(5.0)
        self.assertEqD(self.synapseStatus("r1"), 0.0, "r1 should be zero at start")
        self.assertEqD(self.synapseStatus("r2"), 0.0, "r2 should be zero at start")

        decayedR1 = math.exp(- self.decay_duration / self.syn_spec["tau_plus"])
        decayedR2 = math.exp(- self.decay_duration / self.syn_spec["tau_x"])

        nest.Simulate(20.0)
        self.assertEqD(self.synapseStatus("r1"), 1.0 + decayedR1, "r1 should have decay after spike")
        self.assertEqD(self.synapseStatus("r2"), 1.0 + decayedR2, "r2 should have decay after spike")

    def test_postVarsDecayAfterPreSpike(self):
        """Check that post-synaptic variables (o1, o2) decay after each pre-synaptic spike."""

        self.generateSpikes(self.post_neuron, [10.0])
        self.generateSpikes(self.pre_neuron, [10.0 + self.decay_duration + self.dendritic_delay])
        # last one activates vars computations

        nest.Simulate(5.0)
        self.assertEqD(self.synapseStatus("o1"), 0.0, "o1 should be zero at start")
        self.assertEqD(self.synapseStatus("o2"), 0.0, "o2 should be zero at start")

        decayedO1 = math.exp(- self.decay_duration / self.syn_spec["tau_minus"])
        decayedO2 = math.exp(- self.decay_duration / self.syn_spec["tau_y"])

        nest.Simulate(20.0)
        self.assertEqD(self.synapseStatus("o1"), decayedO1, "o1 should have decay after spike")
        self.assertEqD(self.synapseStatus("o2"), decayedO2, "o2 should have decay after spike")

    def test_postVarsDecayAfterPostSpike(self):
        """Check that post-synaptic variables (o1, o2) decay after each post-synaptic spike."""

        self.generateSpikes(self.post_neuron, [10.0, 10.0 + self.decay_duration / 2 - self.dendritic_delay])
        self.generateSpikes(self.pre_neuron, [10.0 + self.decay_duration + self.dendritic_delay])
        # last one activates vars computations

        nest.Simulate(5.0)
        self.assertEqD(self.synapseStatus("o1"), 0.0, "o1 should be zero at start")
        self.assertEqD(self.synapseStatus("o2"), 0.0, "o2 should be zero at start")

        decayedO1 = math.exp(- self.decay_duration / self.syn_spec["tau_minus"])
        decayedO2 = math.exp(- self.decay_duration / self.syn_spec["tau_y"])

        nest.Simulate(20.0)
        self.assertEqD(self.synapseStatus("o1"), decayedO1, "o1 should have decay after spike")
        self.assertEqD(self.synapseStatus("o2"), decayedO2, "o2 should have decay after spike")

def suite():
    suite1 = unittest.TestLoader().loadTestsFromTestCase(STDPTripletConnectionTestCase)
    return unittest.TestSuite([suite1])

def run():
    runner = unittest.TextTestRunner(verbosity = 2)
    runner.run(suite())

if __name__ == "__main__":
    run()
