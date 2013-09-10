#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse, glob, os, random, shutil, subprocess, sys, thread, time
import pp

g_dir = 'experiment-pw'
g_plotters = ['./puddleworld.py', './memory.py']
g_plotter_grid = []#['./puddleworld-grid.py', './puddleworld-heat.py']
g_plotter_grid_filters = ['move(north)', 'move(south)', 'move(east)', 'move(west)', 'all']

g_base_command = "./carli --output experiment --environment puddle-world --random-start true --learning-rate 0.1 --discount-rate 0.999 --epsilon-greedy 0.1 --policy off-policy --pseudoepisode-threshold 20"

g_ep_tuples = []

## Experiment 0, non-hierarchical agents comparison
g_ep_tuples.append(("specific_4x4_4x4_0", "--scenario 0 --credit-assignment specific --split-min 5 --split-max 5"))
g_ep_tuples.append(("specific_8x8_8x8_0", "--scenario 0 --credit-assignment specific --split-min 7 --split-max 7"))
g_ep_tuples.append(("specific_16x16_16x16_0", "--scenario 0 --credit-assignment specific --split-min 9 --split-max 9"))
#g_ep_tuples.append(("specific_32x32_32x32_0", "--scenario 0 --credit-assignment specific --split-min 11 --split-max 11"))
#g_ep_tuples.append(("specific_64x64_64x64_0", "--scenario 0 --credit-assignment specific --split-min 13 --split-max 13"))

## Experiment 1, hierarchical agents performance comparison - compare to 0
#g_ep_tuples.append(('puddle-world', 0, 'even', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  5,  5, 0, 0.5, 0, 0, 0, 1, 0, 'false'))
#g_ep_tuples.append(('puddle-world', 0, 'even', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 1, 0, 'false'))
#g_ep_tuples.append(('puddle-world', 0, 'even', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  9,  9, 0, 0.5, 0, 0, 0, 1, 0, 'false'))
#g_ep_tuples.append(('puddle-world', 0, 'even', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 11, 11, 0, 0.5, 0, 0, 0, 1, 0, 'false'))
g_ep_tuples.append(("even_64x64_64x64_1", "--scenario 1 --credit-assignment even --split-min 13 --split-max 13"))

#g_ep_tuples.append(("cmac_0_4_2", "--cmac true --cmac-resolution 4 --cmac-tilings 2"))
#g_ep_tuples.append(("cmac_0_4_4", "--cmac true --cmac-resolution 4 --cmac-tilings 4"))
#g_ep_tuples.append(("cmac_0_4_8", "--cmac true --cmac-resolution 4 --cmac-tilings 8"))
#g_ep_tuples.append(("cmac_0_4_16", "--cmac true --cmac-resolution 4 --cmac-tilings 16"))
#g_ep_tuples.append(("cmac_0_8_2", "--cmac true --cmac-resolution 8 --cmac-tilings 2"))
#g_ep_tuples.append(("cmac_0_8_4", "--cmac true --cmac-resolution 8 --cmac-tilings 4"))
#g_ep_tuples.append(("cmac_0_8_8", "--cmac true --cmac-resolution 8 --cmac-tilings 8"))
g_ep_tuples.append(("cmac_0_8_16", "--cmac true --cmac-resolution 8 --cmac-tilings 16"))
#g_ep_tuples.append(("cmac_0_16_2", "--cmac true --cmac-resolution 16 --cmac-tilings 2"))
#g_ep_tuples.append(("cmac_0_16_4", "--cmac true --cmac-resolution 16 --cmac-tilings 4"))
#g_ep_tuples.append(("cmac_0_16_8", "--cmac true --cmac-resolution 16 --cmac-tilings 8"))
g_ep_tuples.append(("cmac_0_16_16", "--cmac true --cmac-resolution 16 --cmac-tilings 16"))
#g_ep_tuples.append(("cmac_0_32_2", "--cmac true --cmac-resolution 32 --cmac-tilings 2"))
#g_ep_tuples.append(("cmac_0_32_4", "--cmac true --cmac-resolution 32 --cmac-tilings 4"))
#g_ep_tuples.append(("cmac_0_32_8", "--cmac true --cmac-resolution 32 --cmac-tilings 8"))
#g_ep_tuples.append(("cmac_0_32_16", "--cmac true --cmac-resolution 32 --cmac-tilings 16"))
#g_ep_tuples.append(("cmac_0_64_2", "--cmac true --cmac-resolution 64 --cmac-tilings 2"))
#g_ep_tuples.append(("cmac_0_64_4", "--cmac true --cmac-resolution 64 --cmac-tilings 4"))
#g_ep_tuples.append(("cmac_0_64_8", "--cmac true --cmac-resolution 64 --cmac-tilings 8"))
#g_ep_tuples.append(("cmac_0_64_16", "--cmac true --cmac-resolution 64 --cmac-tilings 16"))

# Experiment 3, dynamically refined hierarchical agents performance comparison - compare to 1 and 2
g_ep_tuples.append(("even_2x2_64x64_3", "--credit-assignment even --split-min 3 --split-max 13"))
g_ep_tuples.append(("inv-log-update-count_2x2_64x64_3", "--credit-assignment inv-log-update-count --split-min 3 --split-max 13"))
#g_ep_tuples.append(("inv-root-update-count_2x2_64x64_3", "--credit-assignment inv-root-update-count --split-min 3 --split-max 13"))
g_ep_tuples.append(("specific_2x2_64x64_3", "--credit-assignment specific --split-min 3 --split-max 13"))



### Experiment 2, alternative credit assignment performance comparison - compare to 1
##g_ep_tuples.append(('puddle-world', 0, 'random', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  5,  5, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'random', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'random', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  9,  9, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'random', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 11, 11, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'random', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  5,  5, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  9,  9, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 11, 11, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-root-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  5,  5, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-root-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-root-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  9,  9, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-root-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 11, 11, 0, 0.5, 0, 0, 0, 2, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-root-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 2, 0, 'false'))

### Experiment 100s, move goal
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 100,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 100,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 100,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 100,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 100,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 100,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 101, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 101, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 101, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 101, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 101, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 101, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 110,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 110,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 110,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 110,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 110,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 110,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 111, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 111, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 111, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 111, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 111, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 111, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 120,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 120,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 120,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 120,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 120,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 120,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 121, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 121, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 121, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 121, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 121, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 121, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 130,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 130,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 130,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 130,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 130,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 130,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 131, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 131, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 131, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 131, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 131, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 131, 50000, 'false'))

### Experiment 200s, increase puddle sizes
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 200,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 200,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 200,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 200,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 200,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 200,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 201, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 201, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 201, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 201, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 201, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 201, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 210,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 210,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 210,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 210,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 210,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 210,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 211, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 211, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 211, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 211, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 211, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 211, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 220,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 220,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 220,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 220,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 220,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 220,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 221, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 221, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 221, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 221, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 221, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 221, 50000, 'false'))

### Experiment 300s, increase noise
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 300,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 300,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 300,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 300,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 300,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 300,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 301, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 301, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 301, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 301, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 301, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 301, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 310,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 310,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 310,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 310,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 310,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 310,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 311, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 311, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 311, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 311, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 311, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 311, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 320,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 320,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 320,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 320,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 320,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 320,    -1, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 321, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'specific', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 321, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  7,  7, 0, 0.5, 0, 0, 0, 321, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 13, 13, 0, 0.5, 0, 0, 0, 321, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3,  7, 0, 0.5, 0, 0, 0, 321, 50000, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'even',     0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20,  3, 13, 0, 0.5, 0, 0, 0, 321, 50000, 'false'))

##g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 3, 13, 0, 0.5,     0, 0,    0, 0, 0, 'false'))
##g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 0.999, 0, 0.1, 0.1, 'false', 'off-policy', 20, 3, 13, 0, 0.5, 0.001, 0, 1000, 0, 0, 'false'))


parser = argparse.ArgumentParser(description='Run PuddleWorld experiments.')
parser.add_argument('-j', '--jobs', metavar='N', type=int,
                   action='store',
                   help='number of experiments to run in parallel')
parser.add_argument('-r', '--runs', metavar='N', type=int,
                   action='store', default=1,
                   help='number of runs per experiment')
parser.add_argument('-n', '--num-steps', metavar='N', type=int,
                   action='store', default=50000,
                   help='number of steps per run')

args = parser.parse_args()

if args.jobs is None:
  args.jobs = 'autodetect'


if not os.path.isdir(g_dir):
  os.mkdir(g_dir)
seeds = []
seeds_file = g_dir + '/seeds'
if os.path.isfile(seeds_file):
  f = open(seeds_file, 'r')
  for seed in f:
    seeds.append(int(seed))
  f.close()
if len(seeds) != args.runs:
  if len(seeds) > 0:
    raise Exception('Number of seeds differs from number of runs.')
  else:
    seeds = []
    for i in range(0, args.runs):
      seeds.append(random.randint(0,65535))
    f = open(seeds_file, 'w')
    for seed in seeds:
      f.write(str(seed) + '\n')
    f.close()
print str(seeds) + '\n'


class Experiment:
  def __init__(self, num_steps, seed, stderr, stdout, experiment):
    self.num_steps = num_steps
    self.seed = seed
    self.stderr = stderr
    self.stdout = stdout
    self.experiment = experiment
    
  def get_args(self):
    args = self.experiment.split(' ')
    args.extend(['--num-steps', str(self.num_steps),
                 '--seed', str(self.seed),
                 '--stderr', self.stderr,
                 '--stdout', self.stdout])
    return args
  
  def print_args(self):
    args = self.get_args()
    cmd = args[0]
    for arg in args[1:]:
      cmd += ' ' + arg
    print cmd
  
  def run(self):
    args = self.get_args()
    subprocess.call(args)
    return self

def resolution(split):
  depth = (split - 1) / 2
  size = str(pow(2, depth))
  return size + 'x' + size

g_dirs = []
g_experiments = []
for ep_tuple in g_ep_tuples:
  dir = g_dir + '/' + ep_tuple[0]
  if not os.path.isdir(dir):
    os.mkdir(dir)
  g_dirs.append(dir)
  
  for seed in seeds:
    stderr = dir + '/puddleworld-' + str(seed) + '.err'
    stdout = dir + '/puddleworld-' + str(seed) + '.out'
    experiment = Experiment(args.num_steps, seed, stderr, stdout, g_base_command + ' ' + ep_tuple[1])
    g_experiments.append(experiment)
    experiment.print_args()

class Progress:
  def __init__(self, experiments):
    self.lock = thread.allocate_lock()
    
    self.count = {}
    self.finished = {}
    for experiment in experiments:
      try:
        self.count[experiment.experiment] += 1
      except KeyError:
        self.count[experiment.experiment] = 1
      self.finished[experiment.experiment] = 0

  def just_finished(self, experiment):
    self.lock.acquire()
    self.finished[experiment.experiment] += 1
    self.lock.release()

  def just_finished_plot(self, args):
    self.lock.acquire()
    self.finished['plots'] += 1
    self.lock.release()

  def all_finished(self, experiment):
    self.lock.acquire()
    num = self.count[experiment]
    fin = self.finished[experiment]
    self.lock.release()
    return fin is num

class Plots:
  def __init__(self):
    self.experiment = 'plots'

plots = []
for i in range(len(g_ep_tuples) * (len(g_plotters) + len(g_plotter_grid) * len(g_plotter_grid_filters))):
  plots.append(Plots())

def syscall(args):
  subprocess.call(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)

def take_fives(group):
  while True:
    job_server.print_stats()
    if progress.all_finished(group):
      break
    else:
      time.sleep(5)
  job_server.wait(group)

job_server = pp.Server(args.jobs)
progress = Progress(g_experiments + plots)
start_time = time.time()
#for experiment in g_experiments:
  #experiment.run()
jobs = [(job_server.submit(Experiment.run, (experiment,), (), ('subprocess', 'thread',), callback=progress.just_finished, group=experiment.experiment)) for experiment in g_experiments]

for ep_tuple, dir in zip(g_ep_tuples, g_dirs):
  take_fives(g_base_command + ' ' + ep_tuple[1])
  print 'Plotting data for ' + ep_tuple[1] + '\n'
  for plotter in g_plotters:
    args = [plotter] + glob.glob(dir + '/*.out')
    jobs.append(job_server.submit(syscall, (args,), (), ('subprocess', 'thread',), callback=progress.just_finished_plot, group='plots'))
  for plotter in g_plotter_grid:
    for filter in g_plotter_grid_filters:
      args = [plotter, filter] + glob.glob(dir + '/*.err')
      print 'Plotting ' + plotter + ' data for ' + ep_tuple[1] + ', ' + filter + '\n'
      jobs.append(job_server.submit(syscall, (args,), (), ('subprocess', 'thread',), callback=progress.just_finished_plot, group='plots'))

take_fives('plots')

print 'Total time elapsed: ', time.time() - start_time, 'seconds'
