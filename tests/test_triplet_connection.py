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

    def synapseStatus(self, which):
        """Get parameter value."""
        stats = nest.GetConnections(self.pre_neuron, synapse_model = self.synapse_model)
        return nest.GetStatus(stats, [which])[0][0]

    def assertAlmostEqualDetailed(self, expected, given, message):
        """Improve assetAlmostEqual with detailed message."""
        messageWithValues = "%s (expected: `%s` was: `%s`" % (message, str(expected), str(given))
        self.assertAlmostEqual(given, expected, msg = messageWithValues)

    def generateSpikes(self, neuron, times):
        """Trigger spike to given neuron at specified times."""
        gen = nest.Create("spike_generator", 1, { "spike_times": times })
        nest.Connect(gen, neuron)

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

    def test_preVarsIncreaseWithPreSpike(self):
        """Check that pre-synaptic variables (r1, r2) increase after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [10.0])

        nest.Simulate(5.0)
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("r1"), "r1 should be zero at start")
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("r2"), "r2 should be zero at start")

        nest.Simulate(10.0)
        self.assertAlmostEqualDetailed(1.0, self.synapseStatus("r1"), "r1 should have increased by 1 after spike")
        self.assertAlmostEqualDetailed(1.0, self.synapseStatus("r2"), "r2 should have increased by 1 after spike")

    def test_postVarsIncreaseWithPostSpike(self):
        """Check that post-synaptic variables (o1, o2) increase after each post-synaptic spike."""

        self.generateSpikes(self.post_neuron, [10.0])
        self.generateSpikes(self.pre_neuron, [10.0 + self.dendritic_delay]) # trigger computation

        nest.Simulate(5.0)
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("o1"), "o1 should be zero at start")
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("o2"), "o2 should be zero at start")

        nest.Simulate(10.0)
        self.assertAlmostEqualDetailed(1.0, self.synapseStatus("o1"), "o1 should have increased by 1 after spike")
        self.assertAlmostEqualDetailed(1.0, self.synapseStatus("o2"), "o2 should have increased by 1 after spike")

    def test_preVarsDecayAfterPreSpike(self):
        """Check that pre-synaptic variables (r1, r2) decay after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [10.0])
        self.generateSpikes(self.pre_neuron, [10.0 + self.decay_duration])  # trigger computation

        nest.Simulate(5.0)
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("r1"), "r1 should be zero at start")
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("r2"), "r2 should be zero at start")

        decayedR1 = exp(- self.decay_duration / self.syn_spec["tau_plus"])
        decayedR2 = exp(- self.decay_duration / self.syn_spec["tau_x"])

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(1.0 + decayedR1, self.synapseStatus("r1"), "r1 should have decay after spike")
        self.assertAlmostEqualDetailed(1.0 + decayedR2, self.synapseStatus("r2"), "r2 should have decay after spike")

    def test_preVarsDecayAfterPostSpike(self):
        """Check that pre-synaptic variables (r1, r2) decay after each post-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [10.0])
        self.generateSpikes(self.post_neuron, [11.0, 12.0])
        self.generateSpikes(self.pre_neuron, [10.0 + self.decay_duration])  # trigger computation

        nest.Simulate(5.0)
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("r1"), "r1 should be zero at start")
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("r2"), "r2 should be zero at start")

        decayedR1 = exp(- self.decay_duration / self.syn_spec["tau_plus"])
        decayedR2 = exp(- self.decay_duration / self.syn_spec["tau_x"])

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(1.0 + decayedR1, self.synapseStatus("r1"), "r1 should have decay after spike")
        self.assertAlmostEqualDetailed(1.0 + decayedR2, self.synapseStatus("r2"), "r2 should have decay after spike")

    def test_postVarsDecayAfterPreSpike(self):
        """Check that post-synaptic variables (o1, o2) decay after each pre-synaptic spike."""

        self.generateSpikes(self.post_neuron, [10.0])
        self.generateSpikes(self.pre_neuron, [10.0 + self.dendritic_delay + self.decay_duration]) # trigger computation

        nest.Simulate(5.0)
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("o1"), "o1 should be zero at start")
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("o2"), "o2 should be zero at start")

        decayedO1 = exp(- self.decay_duration / self.syn_spec["tau_minus"])
        decayedO2 = exp(- self.decay_duration / self.syn_spec["tau_y"])

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(decayedO1, self.synapseStatus("o1"), "o1 should have decay after spike")
        self.assertAlmostEqualDetailed(decayedO2, self.synapseStatus("o2"), "o2 should have decay after spike")

    def test_postVarsDecayAfterPostSpike(self):
        """Check that post-synaptic variables (o1, o2) decay after each post-synaptic spike."""

        self.generateSpikes(self.post_neuron, [10.0, 11.0, 12.0])
        self.generateSpikes(self.pre_neuron, [10.0 + self.dendritic_delay + self.decay_duration]) # trigger computation

        nest.Simulate(5.0)
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("o1"), "o1 should be zero at start")
        self.assertAlmostEqualDetailed(0.0, self.synapseStatus("o2"), "o2 should be zero at start")

        decayedO1 = exp(- 1.0 / self.syn_spec["tau_minus"]) + 1.0
        decayedO2 = exp(- 1.0 / self.syn_spec["tau_y"]) + 1.0
        decayedO1 = decayedO1 * exp(- 1.0 / self.syn_spec["tau_minus"]) + 1.0
        decayedO2 = decayedO2 * exp(- 1.0 / self.syn_spec["tau_y"]) + 1.0
        decayedO1 *= exp(- (self.decay_duration - 2.0) / self.syn_spec["tau_minus"])
        decayedO2 *= exp(- (self.decay_duration - 2.0) / self.syn_spec["tau_y"])

        print decayedO1

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(decayedO1, self.synapseStatus("o1"), "o1 should have decay after spike")
        self.assertAlmostEqualDetailed(decayedO2, self.synapseStatus("o2"), "o2 should have decay after spike")

    def test_weightChangeWhenPreSpikeBeforePostSpike(self):
        """Check that weight changes whenever a pre-post spike pairs happen."""

        def facilitate(w, r1, o2):
            return w + r1 * (self.syn_spec["a2_plus"] + self.syn_spec["a3_plus"] * o2)

        def depress(w, o1, r2):
            return w - o1 * (self.syn_spec["a2_minus"] + self.syn_spec["a3_minus"] * r2)

        self.generateSpikes(self.pre_neuron, [10.0])
        self.generateSpikes(self.post_neuron, [12.0])
        self.generateSpikes(self.pre_neuron, [14.0]) # trigger computation

        nest.Simulate(5.0)
        r1 = self.synapseStatus("r1")
        r2 = self.synapseStatus("r2")
        o1 = self.synapseStatus("o1")
        o2 = self.synapseStatus("o2")
        weight = self.synapseStatus("weight")

        nest.Simulate(5.0)
        r1 *= exp(- 5.0 / self.syn_spec["tau_plus"])
        r2 *= exp(- 5.0 / self.syn_spec["tau_x"])
        o1 *= exp(- 5.0 / self.syn_spec["tau_minus"])
        o2 *= exp(- 5.0 / self.syn_spec["tau_y"])
        weight = depress(weight, o1, r2)
        r1 += 1.0
        r2 += 1.0

        nest.Simulate(2.0)
        r1 *= exp(- 2.0 / self.syn_spec["tau_plus"])
        r2 *= exp(- 2.0 / self.syn_spec["tau_x"])
        o1 *= exp(- 2.0 / self.syn_spec["tau_minus"])
        o2 *= exp(- 2.0 / self.syn_spec["tau_y"])
        weight = facilitate(weight, r1, o2)
        o1 += 1.0
        o2 += 1.0

        nest.Simulate(2.0)
        r1 *= exp(- 2.0 / self.syn_spec["tau_plus"])
        r2 *= exp(- 2.0 / self.syn_spec["tau_x"])
        o1 *= exp(- 2.0 / self.syn_spec["tau_minus"])
        o2 *= exp(- 2.0 / self.syn_spec["tau_y"])
        weight = depress(weight, o1, r2)
        r1 += 1.0
        r2 += 1.0

        nest.Simulate(2.0) # finish computation
        self.assertAlmostEqualDetailed(weight, self.synapseStatus("weight"), "weight should have decreased")

    def test_weightChangeWhenPostSpikeBeforePreSpike(self):
        """Check that weight changes whenever a post-pre spike pair happen."""

        self.generateSpikes(self.post_neuron, [11.0])
        self.generateSpikes(self.post_neuron, [10.0])
        self.generateSpikes(self.pre_neuron, [10.0 + self.decay_duration])  # trigger computation

        nest.Simulate(5.0)
        weightBefore = self.synapseStatus("weight")

        nest.Simulate(20.0)
        weightAfter = self.synapseStatus("weight")

        r2Before = self.synapseStatus("r2")
        o2Before = self.synapseStatus("o2")
        r1After = self.synapseStatus("r1")
        o1After = self.synapseStatus("o1")

        a2_minus = self.synapseStatus("a2_minus")
        a3_minus = self.synapseStatus("a3_minus")
        a2_plus = self.synapseStatus("a2_plus")
        a3_plus = self.synapseStatus("a3_plus")

        increase = - r1After * (a2_plus + a3_plus * o2Before)
        decrease = - o1After * (a2_minus + a3_minus * r2Before)

        #print weightBefore
        #print weightAfter
        #print increase
        #print decrease

        #self.assertAlmostEqualDetailed(0.0, weightBefore - weightAfter, "weight should have increased")

def suite():
    suite1 = unittest.TestLoader().loadTestsFromTestCase(STDPTripletConnectionTestCase)
    return unittest.TestSuite([suite1])

def run():
    runner = unittest.TextTestRunner(verbosity = 2)
    runner.run(suite())

if __name__ == "__main__":
    run()
