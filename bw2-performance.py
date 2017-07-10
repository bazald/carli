#!/usr/bin/env python
# -*- coding: utf-8 -*-

import hashlib, os, re
import collections
import math
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
    if arg.find('e') is not -1:
      return arg
    s = arg.split('.')
    if len(s) is 2 and s[1][0] is not '0':
      return ""
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

class ExpFormatter(ScalarFormatter):
  def pprint_val(self, x):
    px = ScalarFormatter.pprint_val(self, x)
    if os.name is 'posix':
      px = px[1:len(px)-1]
    px = self.add_commas(self.exp(px))
    if os.name is 'posix' and len(px) is not 0:
      px = "$" + px + "$"
    return px
  
  def add_commas(self, arg):
    if arg.find('e') is not -1:
      return arg
    s = arg.split('.')
    if len(s) is 2 and s[1][0] is not '0':
      return ""
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
  
  def exp(self, arg):
    return str(math.pow(10, float(arg)) / 1000000.0)

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
  min_x = float("inf")
  max_x = float("-inf")
  values = {}
  for filename in sys.argv[1:]:
    f = open(filename, 'r')
    line = f.readline()
    f.close()
    directory = re.search('(^.*[^/]+)/+[^/]*$', filename).group(0)
    name = re.search('([^/]+)/+[^/]*$', filename).group(1)
    index = int(re.search('-([^-]+)\.txt$', filename).group(1))
    time = math.log(float(re.search('= ([^ ]+) ', line).group(1)) * 10000.0, 10.0)
    
    if index < min_x:
      min_x = index
    if index > max_x:
      max_x = index
    
    try:
      values[name][index] = time
    except KeyError:
      values[name] = {}
      values[name][index] = time

  x = []
  for i in range(min_x,max_x+1):
    x.append(i)

  fig = plt.figure()
  fig.canvas.set_window_title('Blocks World Rete Performance')
  
  pylab.axes([0.2,0.15,0.7725,0.82])
  
  remap_names = collections.OrderedDict()
  remap_names['dup-flu'] = 'Full Deoptimization'
  remap_names['dup'] = 'Disabled Node Sharing'
  remap_names['flu'] = 'Flushing WMEs'
  remap_names['opt'] = 'Optimized' 
  
  labels = []
  y_labels = []
  yss = []
  for agent, name in remap_names.items():
    y = []
    for index in x:
      try:
        y.append(values[agent][index])
      except KeyError:
        y.append(0)

    y_labels.append(remap_names[agent])
    yss.append(y)
    
    color = 'black'
    linestyle = ''
    if agent is 'opt':
      color = 'blue'
      linestyle = '-'
    elif agent is 'dup':
      color = 'orange'
      linestyle = '--'
    elif agent is 'flu':
      color = 'orange'
      linestyle = ':'
    elif agent is 'dup-flu':
      color = 'red'
      linestyle = '-.'
    
    labels += pylab.plot(x, y, label=remap_names[agent], color=color, linestyle=linestyle)
  
  pylab.grid(False)
  
  pylab.xlabel('Number of Blocks', fontsize=8)
  pylab.ylabel('CPU Time Per Step (s)', fontsize=8)
  
  pylab.xlim(xmax=72)
#  if len(sys.argv) > 1:
#    pylab.ylim(ymin=-500, ymax=0)
  pylab.ylim(ymin=0, ymax=10)
  
  fig.axes[0].xaxis.set_major_formatter(CommaFormatter())
  fig.axes[0].yaxis.set_major_formatter(ExpFormatter())
  
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
  pylab.legend(labels, [l.get_label() for l in labels], loc=4, handlelength=4.2, numpoints=2)

  splitd = directory.rsplit('/', 2)
  
  m = hashlib.md5()
  for name in remap_names:
    m.update(name)
  filename = str(m.hexdigest())
  
  if not os.path.exists(splitd[0] + '/csv'):
    os.makedirs(splitd[0] + '/csv')
  write_to_csv(splitd[0] + '/csv/' + filename + '.csv', 'Step Number', x, y_labels, yss)
  
  if not os.path.exists(splitd[0] + '/eps'):
    os.makedirs(splitd[0] + '/eps')
  pylab.savefig(splitd[0] + '/eps/' + filename + '.eps')
  
  if not os.path.exists(splitd[0] + '/png'):
    os.makedirs(splitd[0] + '/png')
  pylab.savefig(splitd[0] + '/png/' + filename + '.png', dpi=1200)
  
  if not os.path.exists(splitd[0] + '/svg'):
    os.makedirs(splitd[0] + '/svg')
  pylab.savefig(splitd[0] + '/svg/' + filename + '.svg')

if __name__ == "__main__":
  main()
