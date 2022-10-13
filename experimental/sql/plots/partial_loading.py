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
    counts = []
    curr_count = 0
    for line in f:
      l = line.strip()
      if l.isnumeric():
        counts.append(curr_count)
        curr_count = 0
      else:
        curr_count += 1
    counts.append(curr_count)
    return counts[1:]


column_counts = {
    "customer": 8,
    "orders": 9,
    "partsupp": 5,
    "supplier": 7,
    "part": 9,
    "nation": 4,
    "region": 3,
    "lineitem": 16
}

counts = parse_data("../data/partial_loading.txt")

used_tables = [["lineitem"], ["customer", "orders", "lineitem"],
               [
                   "customer", "orders", "lineitem", "supplier", "nation",
                   "region"
               ], ["lineitem"], ["customer", "orders", "lineitem", "nation"]]

mentioned_columns = [7, 10, 16, 4, 16]

all_columns = [sum(map(lambda x: column_counts[x], l)) for l in used_tables]

# Color palette
light_gray = "#cacaca"
dark_gray = "#827b7b"
light_blue = "#a6cee3"
dark_blue = "#1f78b4"
light_green = "#b2df8a"
dark_green = "#33a02c"
light_red = "#fb9a99"
dark_red = "#e31a1c"

labels = ["Q1", "Q3", "Q5", "Q6", "Q10"]

width = 0.3
x = np.arange(len(labels))

fig, ax = plt.subplots()
rects3 = ax.bar(x,
                all_columns,
                2 * width,
                color='w',
                edgecolor='black',
                linestyle='dashed',
                label='Basetable columns')
rects = ax.bar(x - width / 2,
               counts,
               width,
               color=light_blue,
               label='Accessed by us')
rects2 = ax.bar(x + width / 2,
                mentioned_columns,
                width,
                color=dark_blue,
                label='Used in the query')

ax.set_ylim(0, 50)
ax.set_xticks(x)
ax.set_xticklabels(labels)
# Y-Axis Label
#
# Use a horizontal label for improved readability.
#ax.set_ylabel('Amount of input columns',
#              rotation='horizontal',
#              position=(1, 1.05),
#              horizontalalignment='left',
#              verticalalignment='bottom')

# Add some text for labels, title and custom x-axis tick labels, etc.

# Hide the right and top spines
#
# This reduces the number of lines in the plot. Lines typically catch
# a readers attention and distract the reader from the actual content.
# By removing unnecessary spines, we help the reader to focus on
# the figures in the graph.
ax.spines['right'].set_visible(False)
ax.spines['top'].set_visible(False)


def autolabel(rects):
  """Attach a text label above each bar in *rects*, displaying its height."""
  for rect in rects:
    height = rect.get_height()
    ax.annotate(
        '{}'.format(height),
        xy=(rect.get_x() + rect.get_width() / 2, height),
        xytext=(0, 1),  # 1 points vertical offset
        textcoords="offset points",
        fontsize="smaller",
        ha='center',
        va='bottom')


ax.set_xlabel('Query of the TPC-H benchmark')

autolabel(rects)
autolabel(rects2)
autolabel(rects3)

ax.legend(ncol=2,
          frameon=False,
          loc='lower right',
          bbox_to_anchor=(0, 0.9, 0.9, 0))

fig.tight_layout()

filename = os.path.basename(__file__).replace(".py", ".pdf")
fig.savefig(filename)
