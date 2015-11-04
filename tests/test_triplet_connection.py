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
            "receptor_type": 1, # set receptor 1 post-synaptically, to not generate extra spikes
            "weight": 5.0,
            "tau_plus": 16.8,
            "triplet_tau_plus": 101.0,
            "tau_minus": 33.7,
            "triplet_tau_minus": 125.0,
            "Aplus": 0.1,
            "Aminus": 0.1,
            "triplet_Aplus": 0.1,
            "triplet_Aminus": 0.1,
            "Kplus": 0.0,
            "triplet_Kplus": 0.0,
            "Kminus": 0.0,
            "triplet_Kminus": 0.0,
        }

        # setup basic circuit
        self.pre_neuron = nest.Create("parrot_neuron")
        self.post_neuron = nest.Create("parrot_neuron")
        nest.Connect(self.pre_neuron, self.post_neuron, syn_spec = self.syn_spec)

    def generateSpikes(self, neuron, times):
        """Trigger spike to given neuron at specified times."""
        gen = nest.Create("spike_generator", 1, { "spike_times": times })
        nest.Connect(gen, neuron)

    def status(self, which):
        """Get synapse parameter status."""
        stats = nest.GetConnections(self.pre_neuron, synapse_model = self.synapse_model)
        return nest.GetStatus(stats, [which])[0][0]

    def decay(self, time, Kplus, triplet_Kplus, Kminus, triplet_Kminus):
        """Decay variables."""
        Kplus *= exp(- time / self.syn_spec["tau_plus"])
        triplet_Kplus *= exp(- time / self.syn_spec["triplet_tau_plus"])
        Kminus *= exp(- time / self.syn_spec["tau_minus"])
        triplet_Kminus *= exp(- time / self.syn_spec["triplet_tau_minus"])
        return (Kplus, triplet_Kplus, Kminus, triplet_Kminus)

    def facilitate(self, w, Kplus, triplet_Kminus):
        """Facilitate weight."""
        return w + Kplus * (self.syn_spec["Aplus"] + self.syn_spec["triplet_Aplus"] * triplet_Kminus)

    def depress(self, w, Kminus, triplet_Kplus):
        """Depress weight."""
        return w - Kminus * (self.syn_spec["Aminus"] + self.syn_spec["triplet_Aminus"] * triplet_Kplus)

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

        def badPropertyWith(content, parameters):
            self.assertRaisesRegexp(nest.NESTError, "BadProperty(.+)" + content, setupProperty, parameters)

        badPropertyWith("Kplus", { "Kplus": -1.0 })
        badPropertyWith("triplet_Kplus", { "triplet_Kplus": -1.0 })
        badPropertyWith("Kminus", { "Kminus": -1.0 })
        badPropertyWith("triplet_Kminus", { "triplet_Kminus": -1.0 })
        badPropertyWith("triplet_tau_plus(.+)tau_plus", { "tau_plus": 1.0, "triplet_tau_plus": 1.0 })
        badPropertyWith("triplet_tau_minus(.+)tau_minus",  { "tau_minus": 1.0, "triplet_tau_minus": 1.0 })

    def test_varsZeroAtStart(self):
        """Check that pre and post-synaptic variables are zero at start."""
        self.assertAlmostEqualDetailed(0.0, self.status("Kplus"), "Kplus should be zero")
        self.assertAlmostEqualDetailed(0.0, self.status("triplet_Kplus"), "triplet_Kplus should be zero")
        self.assertAlmostEqualDetailed(0.0, self.status("Kminus"), "Kminus should be zero")
        self.assertAlmostEqualDetailed(0.0, self.status("triplet_Kminus"), "triplet_Kminus should be zero")

    def test_preVarsIncreaseWithPreSpike(self):
        """Check that pre-synaptic variables (Kplus, triplet_Kplus) increase after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [2.0])

        Kplus = self.status("Kplus")
        triplet_Kplus = self.status("triplet_Kplus")

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(Kplus + 1.0, self.status("Kplus"), "Kplus should have increased by 1")
        self.assertAlmostEqualDetailed(triplet_Kplus + 1.0, self.status("triplet_Kplus"),
                                       "triplet_Kplus should have increased by 1")

    def test_postVarsIncreaseWithPostSpike(self):
        """Check that post-synaptic variables (Kminus, triplet_Kminus) increase after each post-synaptic spike."""

        self.generateSpikes(self.post_neuron, [2.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.dendritic_delay]) # trigger computation

        Kminus = self.status("Kminus")
        triplet_Kminus = self.status("triplet_Kminus")

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(Kminus + 1.0, self.status("Kminus"), "Kminus should have increased by 1")
        self.assertAlmostEqualDetailed(triplet_Kminus + 1.0, self.status("triplet_Kminus"),
                                       "triplet_Kminus should have increased by 1")

    def test_preVarsDecayAfterPreSpike(self):
        """Check that pre-synaptic variables (Kplus, triplet_Kplus) decay after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.decay_duration]) # trigger computation

        (Kplus, triplet_Kplus, _, _) = self.decay(self.decay_duration, 1.0, 1.0, 0.0, 0.0)
        Kplus += 1.0
        triplet_Kplus += 1.0

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(Kplus, self.status("Kplus"), "Kplus should have decay")
        self.assertAlmostEqualDetailed(triplet_Kplus, self.status("triplet_Kplus"), "triplet_Kplus should have decay")

    def test_preVarsDecayAfterPostSpike(self):
        """Check that pre-synaptic variables (Kplus, triplet_Kplus) decay after each post-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.post_neuron, [3.0, 4.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.decay_duration]) # trigger computation

        (Kplus, triplet_Kplus, _, _) = self.decay(self.decay_duration, 1.0, 1.0, 0.0, 0.0)
        Kplus += 1.0
        triplet_Kplus += 1.0

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(Kplus, self.status("Kplus"), "Kplus should have decay")
        self.assertAlmostEqualDetailed(triplet_Kplus, self.status("triplet_Kplus"), "triplet_Kplus should have decay")

    def test_postVarsDecayAfterPreSpike(self):
        """Check that post-synaptic variables (Kminus, triplet_Kminus) decay after each pre-synaptic spike."""

        self.generateSpikes(self.post_neuron, [2.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.dendritic_delay + self.decay_duration]) # trigger computation

        (_, _, Kminus, triplet_Kminus) = self.decay(self.decay_duration, 0.0, 0.0, 1.0, 1.0)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(Kminus, self.status("Kminus"), "Kminus should have decay")
        self.assertAlmostEqualDetailed(triplet_Kminus, self.status("triplet_Kminus"),
                                       "triplet_Kminus should have decay")

    def test_postVarsDecayAfterPostSpike(self):
        """Check that post-synaptic variables (Kminus, triplet_Kminus) decay after each post-synaptic spike."""

        self.generateSpikes(self.post_neuron, [2.0, 3.0, 4.0])
        self.generateSpikes(self.pre_neuron, [2.0 + self.dendritic_delay + self.decay_duration]) # trigger computation

        (_, _, Kminus, triplet_Kminus) = self.decay(1.0, 0.0, 0.0, 1.0, 1.0)
        Kminus += 1.0
        triplet_Kminus += 1.0

        (_, _, Kminus, triplet_Kminus) = self.decay(1.0, 0.0, 0.0, Kminus, triplet_Kminus)
        Kminus += 1.0
        triplet_Kminus += 1.0

        (_, _, Kminus, triplet_Kminus) = self.decay(self.decay_duration - 2.0, 0.0, 0.0, Kminus, triplet_Kminus)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(Kminus, self.status("Kminus"), "Kminus should have decay")
        self.assertAlmostEqualDetailed(triplet_Kminus, self.status("triplet_Kminus"),
                                       "triplet_Kminus should have decay")

    def test_weightChangeWhenPrePostSpikes(self):
        """Check that weight changes whenever a pre-post spike pair happen."""

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.post_neuron, [4.0])
        self.generateSpikes(self.pre_neuron, [6.0]) # trigger computation

        Kplus = self.status("Kplus")
        triplet_Kplus = self.status("triplet_Kplus")
        Kminus = self.status("Kminus")
        triplet_Kminus = self.status("triplet_Kminus")
        weight = self.status("weight")

        (Kplus, triplet_Kplus, Kminus, triplet_Kminus) = self.decay(2.0, Kplus, triplet_Kplus, Kminus, triplet_Kminus)
        weight = self.depress(weight, Kminus, triplet_Kplus)
        Kplus += 1.0
        triplet_Kplus += 1.0

        (Kplus, triplet_Kplus, Kminus, triplet_Kminus) = self.decay(2.0 + self.dendritic_delay, Kplus, triplet_Kplus, 
                                                                    Kminus, triplet_Kminus)
        weight = self.facilitate(weight, Kplus, triplet_Kminus)
        Kminus += 1.0
        triplet_Kminus += 1.0

        (Kplus, triplet_Kplus, Kminus, triplet_Kminus) = self.decay(2.0 - self.dendritic_delay, Kplus, triplet_Kplus, 
                                                                    Kminus, triplet_Kminus)
        weight = self.depress(weight, Kminus, triplet_Kplus)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(weight, self.status("weight"), "weight should have decreased")

    def test_weightChangeWhenPrePostPreSpikes(self):
        """Check that weight changes whenever a pre-post-pre spike triplet happen."""

        self.generateSpikes(self.pre_neuron, [2.0, 6.0])
        self.generateSpikes(self.post_neuron, [4.0])
        self.generateSpikes(self.pre_neuron, [8.0]) # trigger computation

        Kplus = self.status("Kplus")
        triplet_Kplus = self.status("triplet_Kplus")
        Kminus = self.status("Kminus")
        triplet_Kminus = self.status("triplet_Kminus")
        weight = self.status("weight")

        (Kplus, triplet_Kplus, Kminus, triplet_Kminus) = self.decay(2.0, Kplus, triplet_Kplus, Kminus, triplet_Kminus)
        weight = self.depress(weight, Kminus, triplet_Kplus)
        Kplus += 1.0
        triplet_Kplus += 1.0

        (Kplus, triplet_Kplus, Kminus, triplet_Kminus) = self.decay(2.0 + self.dendritic_delay, Kplus, triplet_Kplus, 
                                                                    Kminus, triplet_Kminus)
        weight = self.facilitate(weight, Kplus, triplet_Kminus)
        Kminus += 1.0
        triplet_Kminus += 1.0

        (Kplus, triplet_Kplus, Kminus, triplet_Kminus) = self.decay(2.0 - self.dendritic_delay, Kplus, triplet_Kplus,
                                                                    Kminus, triplet_Kminus)
        weight = self.depress(weight, Kminus, triplet_Kplus)
        Kplus += 1.0
        triplet_Kplus += 1.0

        (Kplus, triplet_Kplus, Kminus, triplet_Kminus) = self.decay(2.0, Kplus, triplet_Kplus, Kminus, triplet_Kminus)
        weight = self.depress(weight, Kminus, triplet_Kplus)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(weight, self.status("weight"), "weight should have decreased")

def suite():
    suite1 = unittest.TestLoader().loadTestsFromTestCase(STDPTripletConnectionTestCase)
    return unittest.TestSuite([suite1])

def run():
    runner = unittest.TextTestRunner(verbosity = 2)
    runner.run(suite())

if __name__ == "__main__":
    run()
