import nest
import unittest
from math import exp


@nest.check_stack
class STDPTripletConnectionTestCase(unittest.TestCase):
    """Check stdp_triplet_connection model properties."""

    @classmethod
    def setUpClass(self):
        nest.Install("stdpmodule")

    def setUp(self):
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()

        # settings
        self.dendritic_delay = 1.0
        self.decay_duration = 5.0
        self.synapse_model = "stdp_triplet_all_in_one_synapse"
        self.syn_spec = {
            "model": self.synapse_model,
            "delay": self.dendritic_delay,
            "receptor_type": 1,     # set receptor 1 post-synaptically, to not generate extra spikes
            "weight": 5.0,          # 5.0
            "tau_plus": 16.8,       # 16.8
            "tau_x": 101.0,         # 101.0
            "tau_minus": 33.7,      # 33.7
            "tau_y": 125.0,         # 125.0
            "a2_plus": 1.0,         # 1.0
            "a2_minus": 1.0,        # 1.0
            "a3_plus": 1.0,         # 1.0
            "a3_minus": 1.0,        # 1.0
            "r1": 0.0,              # 0.0
            "r2": 0.0,              # 0.0
            "o1": 0.0,              # 0.0
            "o2": 0.0,              # 0.0
        }

        # setup basic circuit
        self.pre_neuron = nest.Create("parrot_neuron")
        self.post_neuron = nest.Create("parrot_neuron")
        nest.Connect(self.pre_neuron, self.post_neuron, syn_spec = self.syn_spec)

    def generateSpikes(self, neuron, times):
        """Trigger spike to given neuron at specified times."""
        gen = nest.Create("spike_generator", 1, { "spike_times": times })
        nest.Connect(gen, neuron)

    def synapseStatus(self, which):
        """Get parameter status."""
        stats = nest.GetConnections(self.pre_neuron, synapse_model = self.synapse_model)
        return nest.GetStatus(stats, [which])[0][0]

    def decay(self, time, r1, r2, o1, o2):
        """Decay variables."""
        r1 *= exp(- time / self.syn_spec["tau_plus"])
        r2 *= exp(- time / self.syn_spec["tau_x"])
        o1 *= exp(- time / self.syn_spec["tau_minus"])
        o2 *= exp(- time / self.syn_spec["tau_y"])
        return (r1, r2, o1, o2)

    def facilitate(self, w, r1, o2):
        """Facilitate weight."""
        return w + r1 * (self.syn_spec["a2_plus"] + self.syn_spec["a3_plus"] * o2)

    def depress(self, w, o1, r2):
        """Depress weight."""
        return w - o1 * (self.syn_spec["a2_minus"] + self.syn_spec["a3_minus"] * r2)

    def assertAlmostEqualDetailed(self, expected, given, message):
        """Improve assetAlmostEqual with detailed message."""
        messageWithValues = "%s (expected: `%s` was: `%s`" % (message, str(expected), str(given))
        self.assertAlmostEqual(given, expected, msg = messageWithValues)


    def test_badPropertiesSetupsThrowExceptions(self):
        """Check that exceptions are thrown when setting bad parameters."""
        def setupProperty(property):
            bad_syn_spec = self.syn_spec.copy()
            bad_syn_spec.update(property)
            nest.Connect(self.pre_neuron, self.post_neuron, syn_spec = bad_syn_spec)

        exceptionName = "BadProperty"
        self.assertRaisesRegexp(nest.NESTError, exceptionName + "(.+)r1", setupProperty, {"r1": -1.0})
        self.assertRaisesRegexp(nest.NESTError, exceptionName + "(.+)r2", setupProperty, {"r2": -1.0})
        self.assertRaisesRegexp(nest.NESTError, exceptionName + "(.+)o1", setupProperty, {"o1": -1.0})
        self.assertRaisesRegexp(nest.NESTError, exceptionName + "(.+)o2", setupProperty, {"o2": -1.0})
        self.assertRaisesRegexp(nest.NESTError, exceptionName + "(.+)tau_x(.+)tau_plus", setupProperty,
                                {"tau_plus": 1.0, "tau_x": 1.0})
        self.assertRaisesRegexp(nest.NESTError, exceptionName + "(.+)tau_y(.+)tau_minus", setupProperty,
                                {"tau_minus": 1.0, "tau_y": 1.0})

    def test_varsZeroAtStart(self):
        """Check that pre and post-synaptic variables (r1, r2, o1, o2) are zero at start."""
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("r1"), "r1 should be 0 at start")
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("r2"), "r2 should be 0 at start")
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("o1"), "o1 should be 0 at start")
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("o2"), "o2 should be 0 at start")

    def test_preVarsIncreaseWithPreSpike(self):
        """Check that pre-synaptic variables (r1, r2) increase after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [2.0])

        r1 = self.synapseStatus("r1")
        r2 = self.synapseStatus("r2")

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(r1 + 1.0, self.synapseStatus("r1"), "r1 should have increased by 1 after spike")
        self.assertAlmostEqualDetailed(r2 + 1.0, self.synapseStatus("r2"), "r2 should have increased by 1 after spike")

    def test_postVarsIncreaseWithPostSpike(self):
        """Check that post-synaptic variables (o1, o2) increase after each post-synaptic spike."""

        self.generateSpikes(self.post_neuron, [2.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.dendritic_delay]) # trigger computation

        o1 = self.synapseStatus("o1")
        o2 = self.synapseStatus("o2")

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(o1 + 1.0, self.synapseStatus("o1"), "o1 should have increased by 1 after spike")
        self.assertAlmostEqualDetailed(o2 + 1.0, self.synapseStatus("o2"), "o2 should have increased by 1 after spike")

    def test_preVarsDecayAfterPreSpike(self):
        """Check that pre-synaptic variables (r1, r2) decay after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.decay_duration]) # trigger computation

        (r1, r2, _, _) = self.decay(self.decay_duration, 1.0, 1.0, 0.0, 0.0)
        r1 += 1.0
        r2 += 1.0

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(r1, self.synapseStatus("r1"), "r1 should have decay after spike")
        self.assertAlmostEqualDetailed(r2, self.synapseStatus("r2"), "r2 should have decay after spike")

    def test_preVarsDecayAfterPostSpike(self):
        """Check that pre-synaptic variables (r1, r2) decay after each post-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.post_neuron, [3.0, 4.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.decay_duration]) # trigger computation

        (r1, r2, _, _) = self.decay(self.decay_duration, 1.0, 1.0, 0.0, 0.0)
        r1 += 1.0
        r2 += 1.0

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(r1, self.synapseStatus("r1"), "r1 should have decay after spike")
        self.assertAlmostEqualDetailed(r2, self.synapseStatus("r2"), "r2 should have decay after spike")

    def test_postVarsDecayAfterPreSpike(self):
        """Check that post-synaptic variables (o1, o2) decay after each pre-synaptic spike."""

        self.generateSpikes(self.post_neuron, [2.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.dendritic_delay + self.decay_duration]) # trigger computation

        (_, _, o1, o2) = self.decay(self.decay_duration, 0.0, 0.0, 1.0, 1.0)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(o1, self.synapseStatus("o1"), "o1 should have decay after spike")
        self.assertAlmostEqualDetailed(o2, self.synapseStatus("o2"), "o2 should have decay after spike")

    def test_postVarsDecayAfterPostSpike(self):
        """Check that post-synaptic variables (o1, o2) decay after each post-synaptic spike."""

        self.generateSpikes(self.post_neuron, [2.0, 3.0, 4.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.dendritic_delay + self.decay_duration]) # trigger computation

        (_, _, o1, o2) = self.decay(1.0, 0.0, 0.0, 1.0, 1.0)
        o1 += 1.0
        o2 += 1.0

        (_, _, o1, o2) = self.decay(1.0, 0.0, 0.0, o1, o2)
        o1 += 1.0
        o2 += 1.0

        (_, _, o1, o2) = self.decay(self.decay_duration - 2.0, 0.0, 0.0, o1, o2)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(o1, self.synapseStatus("o1"), "o1 should have decay after spike")
        self.assertAlmostEqualDetailed(o2, self.synapseStatus("o2"), "o2 should have decay after spike")

    def test_weightChangeWhenPrePostSpikes(self):
        """Check that weight changes whenever a pre-post spike pair happen."""

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.post_neuron, [4.0])
        self.generateSpikes(self.pre_neuron, [6.0]) # trigger computation

        r1 = self.synapseStatus("r1")
        r2 = self.synapseStatus("r2")
        o1 = self.synapseStatus("o1")
        o2 = self.synapseStatus("o2")
        weight = self.synapseStatus("weight")

        (r1, r2, o1, o2) = self.decay(2.0, r1, r2, o1, o2)
        weight = self.depress(weight, o1, r2)
        r1 += 1.0
        r2 += 1.0

        (r1, r2, o1, o2) = self.decay(2.0 + self.dendritic_delay, r1, r2, o1, o2)
        weight = self.facilitate(weight, r1, o2)
        o1 += 1.0
        o2 += 1.0

        (r1, r2, o1, o2) = self.decay(2.0 - self.dendritic_delay, r1, r2, o1, o2)
        weight = self.depress(weight, o1, r2)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(weight, self.synapseStatus("weight"), "weight should have decreased")

    def test_weightChangeWhenPrePostPreSpikes(self):
        """Check that weight changes whenever a pre-post-pre spike triplet happen."""

        self.generateSpikes(self.pre_neuron, [2.0, 6.0])
        self.generateSpikes(self.post_neuron, [4.0])
        self.generateSpikes(self.pre_neuron, [8.0]) # trigger computation

        r1 = self.synapseStatus("r1")
        r2 = self.synapseStatus("r2")
        o1 = self.synapseStatus("o1")
        o2 = self.synapseStatus("o2")
        weight = self.synapseStatus("weight")

        (r1, r2, o1, o2) = self.decay(2.0, r1, r2, o1, o2)
        weight = self.depress(weight, o1, r2)
        r1 += 1.0
        r2 += 1.0

        (r1, r2, o1, o2) = self.decay(2.0 + self.dendritic_delay, r1, r2, o1, o2)
        weight = self.facilitate(weight, r1, o2)
        o1 += 1.0
        o2 += 1.0

        (r1, r2, o1, o2) = self.decay(2.0 - self.dendritic_delay, r1, r2, o1, o2)
        weight = self.depress(weight, o1, r2)
        r1 += 1.0
        r2 += 1.0

        (r1, r2, o1, o2) = self.decay(2.0, r1, r2, o1, o2)
        weight = self.depress(weight, o1, r2)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(weight, self.synapseStatus("weight"), "weight should have decreased")

def suite():
    suite1 = unittest.TestLoader().loadTestsFromTestCase(STDPTripletConnectionTestCase)
    return unittest.TestSuite([suite1])

def run():
    runner = unittest.TextTestRunner(verbosity = 2)
    runner.run(suite())

if __name__ == "__main__":
    run()
