import pylab as plt
from matplotlib.ticker import ScalarFormatter

order_250 = [
    (32, [2.4849356517195695, 2.003356352448463, 26.827442486584189]),
    (16, [0.70913453499476109, 1.8591993093490602, 16.781761618455249]),
    (8, [0.25812537670135499, 1.3862906813621521, 17.341629594564438]),
    (4, [0.31028966903686522, 1.7264106869697571, 17.958530056476594]),
    (2, [0.52664642333984379, 3.3683850765228271, 28.960719966888426]),
    (1, [1.087284803390503, 6.453490591049194, 48.76197700500488]),
]

order_500 = [
    (32, [11.221941766142848, 11.736006619036196, 103.11752037853002]),
    (16, [6.615241450071335, 12.086601367592809, 73.881701308488829]),
    (8, [0.64905301928520198, 4.8547101438045503, 63.241844969987874]),
    (4, [0.70002021789550783, 5.8491949081420902, 60.284196448326114]),
    (2, [1.3279188394546508, 11.789953517913819, 95.689346814155584]),
    (1, [2.9549163818359374, 24.928628778457643, 186.61018838882447]),
]

order_1000 = [
    (32, [11.452961055934431, 17.963069204986098, 257.76139862686392]),
    (16, [1.2168006032705305, 12.57856805920601, 205.47024254500869]),
    (8, [2.8163829684257506, 23.401808100938798, 218.82744389772412]),
    (4, [2.2603758573532104, 20.737683236598969, 231.15145325660708]),
    (2, [5.1219696760177609, 42.427810001373288, 371.88863828182218]),
    (1, [10.21046223640442, 89.52677240371705, 703.3664569854736]),
]
cores = [32, 16, 8, 4, 2, 1]


plt.figure()

ax1 = plt.subplot(311)
ax1.set_yscale('log')
ax1.yaxis.set_major_formatter(ScalarFormatter())
plt.title("1s duration/1ms resolution")
plt.plot(cores, [x[1][0] for x in order_250], "b", label = 'static')
plt.plot(cores, [x[1][1] for x in order_250], "r", label = 'stdp_synapse')
plt.plot(cores, [x[1][2] for x in order_250], "g", label = 'stdp_neuron')
plt.xlabel("Cores/1'250 neurons")
plt.ylabel("Time (s)")
plt.xlim(0, 33)
plt.xticks([])
plt.legend(loc = "upper center", frameon = False, ncol = 3, bbox_to_anchor = (0.5, 1.5))

ax2 = plt.subplot(312)
ax2.set_yscale('log')
ax2.yaxis.set_major_formatter(ScalarFormatter())
plt.plot(cores, [x[1][0] for x in order_500], "b")
plt.plot(cores, [x[1][1] for x in order_500], "r")
plt.plot(cores, [x[1][2] for x in order_500], "g")
plt.xlabel("Cores/2'500 neurons")
plt.ylabel("Time (s)")
plt.xlim(0, 33)
plt.xticks([])

ax3 = plt.subplot(313)
ax3.set_yscale('log')
ax3.yaxis.set_major_formatter(ScalarFormatter())
plt.plot(cores, [x[1][0] for x in order_1000], "b", label = 'static')
plt.plot(cores, [x[1][1] for x in order_1000], "r", label = 'stdp_synapse')
plt.plot(cores, [x[1][2] for x in order_1000], "g", label = 'stdp_neuron')
plt.xlabel("Cores/5'000 neurons")
plt.ylabel("Time (s)")
plt.xlim(0, 33)
plt.xticks(cores)


plt.figure()

ax1 = plt.subplot(311)
ax1.set_yscale('log')
ax1.yaxis.set_major_formatter(ScalarFormatter())
plt.title("1s duration/1ms resolution")
plt.plot(cores, [x[1][0] for x in order_250], "b", label = "1'250 neurons")
plt.plot(cores, [x[1][0] for x in order_500], "r", label = "2'500 neurons")
plt.plot(cores, [x[1][0] for x in order_1000], "g", label = "5'000 neurons")
plt.xlabel("Cores/static")
plt.ylabel("Time (s)")
plt.xlim(0, 33)
plt.xticks([])
plt.legend(loc = "upper center", frameon = False, ncol = 3, bbox_to_anchor = (0.5, 1.5))

ax2 = plt.subplot(312)
ax2.set_yscale('log')
ax2.yaxis.set_major_formatter(ScalarFormatter())
plt.plot(cores, [x[1][1] for x in order_250], "b", label = "1'250 neurons")
plt.plot(cores, [x[1][1] for x in order_500], "r", label = "2'500 neurons")
plt.plot(cores, [x[1][1] for x in order_1000], "g", label = "5'000 neurons")
plt.xlabel("Cores/stdp_synapse")
plt.ylabel("Time (s)")
plt.xlim(0, 33)
plt.xticks([])

ax3 = plt.subplot(313)
ax3.set_yscale('log')
ax3.yaxis.set_major_formatter(ScalarFormatter())
plt.plot(cores, [x[1][2] for x in order_250], "b", label = "1'250 neurons")
plt.plot(cores, [x[1][2] for x in order_500], "r", label = "2'500 neurons")
plt.plot(cores, [x[1][2] for x in order_1000], "g", label = "5'000 neurons")
plt.xlabel("Cores/stdp_neuron")
plt.ylabel("Time (s)")
plt.xlim(0, 33)
plt.xticks(cores)

plt.show()
