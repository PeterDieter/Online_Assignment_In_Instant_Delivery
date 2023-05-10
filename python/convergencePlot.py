import os
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick


def smooth(scalars, weight):  # Weight between 0 and 1
    last = scalars[0]  # First value in the plot (first timestep)
    smoothed = list()
    for point in scalars:
        smoothed_val = last * weight + (1 - weight) * point  # Calculate smoothed value
        smoothed.append(smoothed_val)                        # Save it
        last = smoothed_val                                  # Anchor the last smoothed value
        
    return smoothed

lines, lambdas = [], []
for subdir, dirs, files in os.walk("data/trainingData/temporal"):
    for file in sorted(files):
        lam = file[12:17]
        print(lam)
        with open(os.path.join(subdir, file)) as fileToOpen:
            line = [int(float(line.rstrip())) for line in fileToOpen]
            lines.append(line)
            lambdas.append(lam)

x = []
for j in range(len(lines[0])):
    x.append((j+1)*100)


fig, ax = plt.subplots()
for idx, line in enumerate(lines):
    plt.plot(x, smooth(line, .8), label="$λ_t$ = " + lambdas[idx])


plt.xlabel("Iteration")
plt.ylabel("Average costs")
plt.legend()
plt.yscale("log")
ax.get_yaxis().set_major_formatter(
    mtick.FuncFormatter(lambda x, p: format(int(x), ',')))
ax.get_xaxis().set_major_formatter(
    mtick.FuncFormatter(lambda x, p: format(int(x), ',')))
plt.title('Convergence dependent on temporal lambda')
plt.savefig("convergenceTemporal.png")
plt.show()

lines = []
with open("data/trainingData/spatiotemporal/averageCosts0.9900000.850000.txt") as fileToOpen:
    line = [int(float(line.rstrip())) for line in fileToOpen]
    lines.append(line)

with open("data/trainingData/temporal/averageCosts0.9900001.000000.txt") as fileToOpen:
    line = [int(float(line.rstrip())) for line in fileToOpen]
    lines.append(line)

fig, ax = plt.subplots()
plt.plot(x, smooth(lines[1], .8), label="$λ_t$ = 0.99")
plt.plot(x, smooth(lines[0], .8), label="$λ_t$ = 0.99; $λ_s$ = 0.85")


plt.xlabel("Iteration")
plt.ylabel("Average costs")
plt.legend()
plt.yscale("log")
ax.get_yaxis().set_major_formatter(
    mtick.FuncFormatter(lambda x, p: format(int(x), ',')))
ax.get_xaxis().set_major_formatter(
    mtick.FuncFormatter(lambda x, p: format(int(x), ',')))
plt.title('Convergence dependent on spatiotemporal lambdas')
plt.savefig("convergenceSpatioTemporal.png")
plt.show()