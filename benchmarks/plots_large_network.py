import numpy as np
import pylab as plt

plt.subplot(111)

runtimes = (691.059984167, 786.641655185)
index = np.arange(len(runtimes))
bar_width = 0.35
plt.bar(index, runtimes, bar_width)

plt.xlabel('Synapse type')
plt.ylabel('Runtime (s)')
plt.title('125\'000 neurons/1s duration/0.1ms resolution/400vcpus/1.28To RAM')
plt.xticks(index + bar_width / 2, ('static', 'stdp_triplet'))
plt.xlim(- bar_width / 2, 1 + 1.5 * bar_width)
plt.ylim(500, 900)

plt.show()
