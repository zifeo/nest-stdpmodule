import pylab as plt

plt.figure()
plt.subplots_adjust(hspace = 0.5)

plt.subplot(211)
plt.title("Brunnel 1s duration/1ms resolution (single core)")

neurons = [1250.0, 2500.0, 6250.0, 12500.0]
plt.plot(neurons, [1.18, 3.05, 11.33, 33.47], "b")
plt.plot(neurons, [11.0, 27.0, 85.55, 170.0], "r")

plt.xlabel("# neurons")
plt.ylabel("Time (s)")
plt.xlim(neurons[0], neurons[-1])
plt.ylim(0, 200)
plt.legend(["nest", "taranis"], loc = "upper left", frameon = False)


plt.subplot(212)
plt.title("Brunnel 1s duration/250 neurons (single core)")

resolutions = [0.1, 0.5, 1.0]
plt.plot(resolutions, [2.00, 1.13, 1.18], "b")
plt.plot(resolutions, [50.0, 20.0, 11.0], "r")

plt.xlabel("Resolutions (ms)")
plt.ylabel("Time (s)")
plt.xlim(resolutions[0], resolutions[-1])
plt.ylim(0, 60)
plt.legend(["nest", "taranis"], loc = "upper right", frameon = False)

plt.show()
