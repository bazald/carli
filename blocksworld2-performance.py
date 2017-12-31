#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import hashlib, os, re
import pylab
from matplotlib import rc
from pylab import arange,pi,sin,cos,sqrt

if os.name is 'posix':
  golden_mean = (sqrt(5)-1.0)/2.0     # Aesthetic ratio
  fig_width = 3                       # width in inches
  fig_height = fig_width*golden_mean  # height in inches
  fig_size =  [fig_width,fig_height]
  params = {'backend': 'ps',
            'axes.labelsize': 8,
            'font.size': 8,
            'legend.fontsize': 6,
            'xtick.labelsize': 6,
            'ytick.labelsize': 6,
            'text.usetex': True,
            'ps.usedistiller': 'xpdf',
            'figure.figsize': fig_size}
  pylab.rcParams.update(params)
  rc('font',**{'family':'serif','serif':['Times']})

pylab.rcParams['path.simplify'] = True

import sys, getopt, random, time, datetime
import numpy as np
import matplotlib
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
from optparse import OptionParser
from matplotlib.ticker import ScalarFormatter
#from matplotlib2tikz import save

print 'matplotlib.__version__ is ' + matplotlib.__version__

class CommaFormatter(ScalarFormatter):
  def pprint_val(self, x):
    px = ScalarFormatter.pprint_val(self, x)
    if os.name is 'posix':
      px = px[1:len(px)-1]
    px = self.add_commas(px)
    if os.name is 'posix' and len(px) is not 0:
      px = "$" + px + "$"
    return px
  
  def add_commas(self, arg):
    s = arg.split('.')
    #if len(s) is 2 and s[1][0] is not '0':
      #return ""
    if s[0][0] not in {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}:
      c = '-' + self.recurse(s[0][1:])
    else:
      c = self.recurse(s[0])
    if len(s) > 1 and s[1] is not '0':
      return c + '.' + s[1]
    else:
      return c
  
  def recurse(self, arg):
    if len(arg) < 4:
      return arg
    s = len(arg) - 3
    return self.recurse(arg[:s]) + ',' + arg[s:]

def write_to_csv(filename, x_label, xs, y_labels, yss):
  f = open(filename, 'w')
  
  f.write(x_label.translate(None, ',\\'))
  for y_label in y_labels:
    f.write(',' + y_label.translate(None, ',\\'))
  f.write('\n')
  
  for i in range(len(xs)):
    f.write(str(xs[i]))
    for ys in yss:
      f.write(',' + str(ys[i]))
    f.write('\n')

def main():
  parser = argparse.ArgumentParser("./blocksworld2.py")
  parser.add_argument("--scenario", help="Which graph should be generated?")
  args, filenames = parser.parse_known_args()

  scenario = 0
  if args.scenario:
    scenario = int(args.scenario)
  
  # 1: ./blocksworld2-performance.py --scenario 1 blocksworld2-performance-episode.txt && for type in csv eps png svg; do mv blocksworld2-performance.$type blocksworld2-performance-episode.$type; done
  # 2: ./blocksworld2-performance.py --scenario 2 blocksworld2-performance-step.txt && for type in csv eps png svg; do mv blocksworld2-performance.$type blocksworld2-performance-step.$type; done

  reward_label = 'Time (Seconds)'

  file_handles = []
  smith = {}
  remap_names = {}
  x = []
  x_max = -1
  for filename in filenames:
    file_handles.append(open(filename, 'r'))
  
  for f in file_handles:
    done = False
    while not done:
      line = f.readline()
      if not line or line == '':
        done = True
        break
      split = line.split(' ')
      group = split[0]
      i = 1
      while True:
        success = False
        try:
          x_val = int(split[i])
          y_val = float(split[i + 3])
          success = True
        except ValueError:
          group += ' ' + split[i]
          i = i + 1
        if success:
          remap_names[group] = group
          break
      if x_val > x_max:
        x.append(x_val)
        x_max = x_val
      try:
        smith[group].append(y_val)
      except KeyError:
        smith[group] = []
        smith[group].append(y_val)
    
  for handle in file_handles:
    handle.close()
  
  agent_list = []
  title='Blocks World Performance'

  fig = plt.figure()
  fig.canvas.set_window_title('Blocks World')
  
  rect = [0.17,0.17,0.80,0.80]
  pylab.axes(rect)
  
  labels = []
  #if val0 == 4:
    #for a in smith:
      #for i in range(1, len(smith[a])):
        #smith[a][i] = 0.95 * smith[a][i - 1] + 0.05 * smith[a][i];
  
  y_labels = []
  yss = []
  
  for agent in smith:
    y_labels.append(agent)
    yss.append(smith[agent])
    
    labels += pylab.plot(x, smith[agent], label=agent, linestyle='solid')

  remap_names = {}
  #remap_names['Exact Disabled Node Sharing & Flushing WMEs'] = 'Exact Disabled Node Sharing \\& Flushing WMEs'
  
  pylab.grid(False)
  
  pylab.xlabel('Number of Blocks', fontsize=8)
  pylab.ylabel(reward_label, fontsize=8)
  
  if scenario == 1:
    pylab.ylim(ymax=60)
  elif scenario == 2:
    pylab.ylim(ymax=2)
  
  fig.axes[0].xaxis.set_major_formatter(CommaFormatter())
  fig.axes[0].yaxis.set_major_formatter(CommaFormatter())
  
  xlabels = fig.axes[0].xaxis.get_ticklabels()
  last_xlabel = xlabels[len(xlabels) - 1]
  last_xlabel.set_horizontalalignment('right')
  last_xlabel.set_x(0)
  #fig.axes[0].yaxis.set_scale('log')
  #print last_xlabel.get_size()
  #print last_xlabel.get_position()
  #print last_xlabel.get_text()
  #print last_xlabel
  
  # lower right
  pylab.legend(labels, [l.get_label() for l in labels], loc=2, handlelength=4.2, numpoints=2)
    
  write_to_csv('blocksworld2-performance.csv', 'Number of Blocks', x, y_labels, yss)
  pylab.savefig('blocksworld2-performance.eps')
  pylab.savefig('blocksworld2-performance.png', dpi=1200)
  pylab.savefig('blocksworld2-performance.svg')
  plt.show()

if __name__ == "__main__":
  main()
