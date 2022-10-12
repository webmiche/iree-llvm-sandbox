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

matplotlib.rcParams['figure.figsize'] = 6, 2.5
matplotlib.rc('xtick', labelsize=7)

hardness = {
    'Select': "done",
    'Aggregate': "done",
    'UnboundTable': "done",
    'Le': "done",
    'Column': "done",
    'Lit': "done",
    'Mul': "done",
    'Sub': "done",
    'Add': "done",
    'sum': "done",
    'mean': "done",
    'count': "done",
    'SortKey': "done",
    'Limit': "done",
    'InnerJoin': "done",
    'Eq': "done",
    'strlike': "easy",
    'ArrayView': "hard",
    'min': "done",
    'selfJoin': "hard",
    'between': "done",
    'searchedCase': "hard",
    'Divide': "done",
    'Gt': "done",
    'simpleCase': "hard",
    'leftOuterJoin': "hard",
    'not': "easy",
    'Ge': "done",
    'Lt': "done",
    'max': "done",
    'count_distinct': "done",
    'SemiJoin': "hard",
    'ValueList': "easy",
    'substr': "easy",
    'antiJoin': "hard",
    'and': "done",
    'or': "easy",
    'neq': "easy",
    'Contains': "hard"
}


def parse_data(f: str):
    with open(f, "r") as f:
        counts = []
        curr_done_count = 0
        curr_easy_count = 0
        curr_hard_count = 0
        for line in f:
            l = line.strip()
            if l.isnumeric():
                counts.append(
                    [curr_done_count, curr_easy_count, curr_hard_count])
                curr_done_count = 0
                curr_easy_count = 0
                curr_hard_count = 0
            else:
                if hardness[l] == "done":
                    curr_done_count += 1
                elif hardness[l] == "easy":
                    curr_easy_count += 1
                elif hardness[l] == "hard":
                    curr_hard_count += 1
                else:
                    print("WTF" + l)
                    exit()
        counts.append([curr_done_count, curr_easy_count, curr_hard_count])
        return counts[1:]


has_subquery = [
    False, True, False, True, False, False, True, True, True, False, True,
    False, True, False, True, True, True, True, False, True, True, True
]

done, easy, hard = zip(*parse_data("../data/ibis_nodes.csv"))

# Color palette
light_gray = "#cacaca"
dark_gray = "#827b7b"
light_blue = "#a6cee3"
dark_blue = "#1f78b4"
light_green = "#b2df8a"
dark_green = "#33a02c"
light_red = "#fb9a99"
dark_red = "#e31a1c"

labels = [f"Q{c}" for c in range(1, 23)]
width = 0.3
x = np.arange(len(labels))

fig, ax = plt.subplots()


def plot_set(y, description, color):
    ax.bar(x, [sum(x) if has_subquery[i] else 0 for i, x in enumerate(y)],
           width,
           color=color,
           hatch='/////')
    ax.bar(x, [sum(x) if not has_subquery[i] else 0 for i, x in enumerate(y)],
           width,
           color=color,
           label=description)


ax.bar(x, [0 for i in range(22)],
       color='w',
       hatch='/////',
       label='With Subquery')

plot_set(list(zip(done, easy, hard)), 'Hard Features', dark_blue)
plot_set(list(zip(done, easy)), 'Easy Features', light_blue)
plot_set(list(zip(done)), 'Supported Features', dark_green)

plt.xlim([-0.5, 21.5])
ax.set_yticks([0, 5, 10, 15])
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


#ax.set_xlabel('Query of the TPC-H benchmark')

ax.legend(ncol=2,
          frameon=False,
          loc='lower right',
          bbox_to_anchor=(0, 0.9, 0.85, 0))

fig.tight_layout()

filename = os.path.basename(__file__).replace(".py", ".pdf")
fig.savefig(filename)
