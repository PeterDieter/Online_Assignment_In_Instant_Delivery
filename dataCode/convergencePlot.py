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
for subdir, dirs, files in os.walk("data/trainingData"):
    for file in sorted(files):
        lam = file[12:17]
        print(lam)
        if (lam[0] != "1" or lam[-1] != "9"):
            with open(os.path.join(subdir, file)) as fileToOpen:
                line = [int(float(line.rstrip())) for line in fileToOpen]
                lines.append(line)
                lambdas.append(lam)

x = []
for j in range(len(lines[0])):
    x.append((j+1)*100)


fig, ax = plt.subplots()
for idx, line in enumerate(lines):
    plt.plot(x, smooth(line, .8), label=lambdas[idx])


plt.xlabel("Iteration")
plt.ylabel("Average costs")
plt.legend()
plt.yscale("log")
ax.get_yaxis().set_major_formatter(
    mtick.FuncFormatter(lambda x, p: format(int(x), ',')))
ax.get_xaxis().set_major_formatter(
    mtick.FuncFormatter(lambda x, p: format(int(x), ',')))
plt.title('Convergence depend on lambda')
plt.savefig("convergence.png")
plt.show()
