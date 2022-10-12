#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

import statistics

# Use TrueType fonts instead of Type 3 fonts
# Type 3 fonts embed bitmaps and are not allowed in camera-ready submissions
# for many conferences. TrueType fonts look better and are accepted.
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42

matplotlib.rcParams['figure.figsize'] = 5, 3


def parse_data(f: str):
    with open(f, "r") as f:
        labels = []
        exectimes = []
        curr_exectimes = []
        for line in f:
            if '.' in line:
                curr_exectimes.append(float(line.strip()) * 1000)
            else:
                exectimes.append(curr_exectimes)
                curr_exectimes = []
                labels.append(int(line.strip()))
        exectimes.append(curr_exectimes)
        return labels, exectimes[1:]


labels, exectime = parse_data("../data/exec_sf.csv")

deviations = [statistics.pstdev(x) for x in exectime]
#deviations = [statistics.variance(x) for x in exectime]
exectime = [sum(x) / len(x) for x in exectime]

# Color palette
light_gray = "#cacaca"
dark_gray = "#827b7b"
light_blue = "#a6cee3"
dark_blue = "#1f78b4"
light_green = "#b2df8a"
dark_green = "#33a02c"
light_red = "#fb9a99"
dark_red = "#e31a1c"

fig, ax = plt.subplots()
ax.set_xscale('log')
ax.set_yscale('log')
plt.errorbar(labels,
             exectime,
             yerr=deviations,
             fmt='-x',
             label='Our approach',
             ecolor=dark_blue,
             color=light_blue)

y = []

for i in range(5):
    y.append(exectime[4] / 16 * pow(2, i))

plt.plot(labels, y, color=dark_gray)

labels_duckdb, exectime_duckdb = parse_data("../data/data_duckdb_single.csv")

deviations_duckdb = [statistics.pstdev(x) for x in exectime_duckdb]
#deviations = [statistics.variance(x) for x in exectime]
exectime_duckdb = [sum(x) / len(x) for x in exectime_duckdb]

plt.errorbar(labels_duckdb,
             exectime_duckdb,
             yerr=deviations_duckdb,
             fmt='-x',
             label='duckdb',
             ecolor=dark_green,
             color=light_green)

# Y-Axis Label
#
# Use a horizontal label for improved readability.
ax.set_ylabel('Runtime [ms]',
              rotation='horizontal',
              position=(1, 1.05),
              horizontalalignment='left',
              verticalalignment='bottom')

ax.set_xlabel('Scale factor')
# Add some text for labels, title and custom x-axis tick labels, etc.

ax.set_yticks([10, 100, 1000])
ax.set_xticks(labels + [16], [])

from matplotlib import ticker

ax.xaxis.set_minor_locator(ticker.NullLocator())
formatter = ticker.ScalarFormatter(useMathText=True)
formatter.set_scientific(False)
formatter.set_powerlimits((-1, 1))
ax.yaxis.set_major_formatter(formatter)
ax.xaxis.set_major_formatter(formatter)

ax.legend(ncol=100,
          frameon=False,
          loc='lower right',
          bbox_to_anchor=(0, 0.92, 1, 0))

# Hide the right and top spines
#
# This reduces the number of lines in the plot. Lines typically catch
# a readers attention and distract the reader from the actual content.
# By removing unnecessary spines, we help the reader to focus on
# the figures in the graph.
ax.spines['right'].set_visible(False)
ax.spines['top'].set_visible(False)

#def autolabel(rects):
#    """Attach a text label above each bar in *rects*, displaying its height."""
#    for rect in rects:
#        height = rect.get_height()
#        ax.annotate(
#            '{}'.format(height),
#            xy=(rect.get_x() + rect.get_width() / 2, height),
#            xytext=(0, 1),  # 1 points vertical offset
#            textcoords="offset points",
#            fontsize="smaller",
#            ha='center',
#            va='bottom')
#
#
#autolabel(rects1)

fig.tight_layout()

filename = os.path.basename(__file__).replace(".py", ".pdf")
fig.savefig(filename)
