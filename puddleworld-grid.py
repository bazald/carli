#!/usr/bin/env python
# -*- coding: utf-8 -*-

import hashlib, os, re
import pylab
from matplotlib import rc
from pylab import arange,pi,sin,cos,sqrt

import matplotlib
from matplotlib.patches import CirclePolygon
from matplotlib.collections import PolyCollection
import pylab 

if os.name is 'posix':
  #golden_mean = (sqrt(5)-1.0)/2.0     # Aesthetic ratio
  fig_width = 3.375                   # width in inches
  fig_height = fig_width#*golden_mean  # height in inches
  fig_size =  [fig_width,fig_height]
  params = {'backend': 'ps',
            'axes.labelsize': 8,
            'text.fontsize': 8,
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
    if len(s) is 2 and s[1][0] is not '0':
      return ""
    if s[0][0] is '-':
      c = '-' + self.recurse(s[0][1:])
    else:
      c = self.recurse(s[0])
    return c
  
  def recurse(self, arg):
    if len(arg) < 4:
      return arg
    s = len(arg) - 3
    return self.recurse(arg[:s]) + ',' + arg[s:]

class Handle:
  def __init__(self, f, filename, seed):
    self.f = f
    self.filename = filename
    self.seed = seed

class Handles:
  def __init__(self):
    self.handles = []
    self.smith = {}

def main():
  title='Generated Value Function for Puddle World'  
  
  fig = plt.figure()
  fig.canvas.set_window_title('Puddle World')
  
  pylab.axes([0.125,0.15,0.8375,0.75])
  
  target_dir = 'all'
  if len(sys.argv) > 1:
    target_dir = sys.argv[1]
  
  lines = {}
  for filename in sys.argv[2:]:
    f = open(filename, 'r')
    regt = re.compile('([^:]+):')
    regls = re.compile('([^ ]+) ([^ ]+) ([^ ]+) ([^ \r\n]+)')
    dir = ''
    while True:
      line = f.readline()
      if not line:
        break
      t = regt.search(line)
      if not t == None:
        dir = t.group(1)
        #print dir + ':'
      elif dir == target_dir:
        ls = regls.search(line)
        if not ls == None:
          key = (ls.group(1), ls.group(2), ls.group(3), ls.group(4))
          try:
            lines[key] += 1.0
          except KeyError:
            lines[key] = 1.0
        else:
          raise Exception('Unknown line encountered in stderr.txt')
    f.close()
  
  divisor = len(sys.argv[2:])
  for key, value in lines.iteritems():
    rgb = 1 - value / divisor;
    fig.axes[0].add_line(pylab.Line2D([key[0], key[2]], [key[1], key[3]], color=(rgb, rgb, rgb)))
  
  #pylab.legend(loc=4, handlelength=4.2, numpoints=2)
  
  pylab.grid(False)
  
  pylab.xlabel('X', fontsize=8)
  pylab.ylabel('Y', fontsize=8)
  pylab.title(title, fontsize=10)
  pylab.xlim(xmin=0, xmax=1)
  pylab.ylim(ymin=0, ymax=1)
  
  #fig.axes[0].xaxis.set_major_formatter(CommaFormatter())
  #fig.axes[0].yaxis.set_major_formatter(CommaFormatter())
  fig.axes[0].set_xticks([0,1])
  fig.axes[0].set_yticks([0,1])
  
  #xlabels = fig.axes[0].xaxis.get_ticklabels()
  #last_xlabel = xlabels[len(xlabels) - 1]
  #last_xlabel.set_horizontalalignment('right')
  #last_xlabel.set_x(0)
  ##fig.axes[0].yaxis.set_scale('log')
  ##print last_xlabel.get_size()
  ##print last_xlabel.get_position()
  ##print last_xlabel.get_text()
  ##print last_xlabel
  
  pylab.savefig('puddle-world.eps')
  pylab.savefig('puddle-world.png', dpi=1200)
  plt.show()

if __name__ == "__main__":
  main()
