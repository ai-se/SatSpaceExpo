import matplotlib.pyplot as plt
from pylab import *
import pdb
import numpy as np
from scipy.ndimage.filters import gaussian_filter1d
from matplotlib.pyplot import figure

info = list()
with open("delta_CNT.info", 'r') as f:
    for line in f.readlines():
        tmpL = line.split(' ')[:-1]
        tmpL = [int(i) for i in tmpL]
        tmpL = sorted(tmpL, reverse=True)
        info.append(tmpL)
figure(figsize=(8, 5))

for i, series in enumerate(info):
    if not len(series):
        continue
    dx = 1 / len(series)
    X = np.arange(0, 1, dx)
    Y = np.array(series, dtype='f')
    Y /= (dx * Y).sum()
    CY = np.cumsum(Y * dx)
    if len(X) != len(CY):  # precision errors
        X = X[:-1]
    # ysmoothed = gaussian_filter1d(CY, sigma=2)  # smooth the lines a little bit
    plt.plot(X, CY, color='gray',
             linewidth=1.7, linestyle=':')

plt.plot(X, X, color='red', linewidth=1)
plt.xlabel("Deltas sorted by decreasing frequency", fontsize=18)
plt.ylabel(r"$cdf$ of delta weights", fontsize=18)
plt.tick_params(axis='x', which='both', bottom=False,
                top=False, labelbottom=False)

# plt.show()
plt.tick_params(axis='y', which='major', labelsize=16)
plt.savefig("delta_cdf.eps", bbox_inches='tight',)

# matplotlib2tikz.save("delta_cdf.tex")
