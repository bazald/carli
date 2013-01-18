#!/usr/bin/env python
# -*- coding: utf-8 -*-

import hashlib, os, re
import pylab
from matplotlib import rc
from pylab import arange,pi,sin,cos,sqrt

import matplotlib
from matplotlib.patches import CirclePolygon
from matplotlib.collections import PolyCollection
from matplotlib.patches import Rectangle
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
  fig = plt.figure()
  fig.canvas.set_window_title('Mountain Car')
  
  pylab.axes([0.125,0.15,0.8375,0.75])
  
  target_entries = 'all'
  if len(sys.argv) > 1:
    target_entries = sys.argv[1]
  
  rects = {}
  all_rects = {}
  for filename in sys.argv[2:]:
    f = open(filename, 'r')
    regt = re.compile('(.+):')
    regls = re.compile('(.+),(.+)/(.+),([^=]+)$')
    regr = re.compile('(.+),(.+)/(.+),(.+)=(.+)$')
    entries = ''
    while True:
      line = f.readline()
      if not line:
        break
      t = regt.search(line)
      if not t == None:
        entries = t.group(1)
        #print entries + ':'
      else:
        r = regr.search(line)
        if not r == None:
          key = (r.group(1), r.group(2), r.group(3), r.group(4), entries)
          try:
            all_rects[key] += int(r.group(5))
          except KeyError:
            all_rects[key] = int(r.group(5))

          if entries == target_entries:
            key = (float(r.group(1)), float(r.group(2)), float(r.group(3)), float(r.group(4)))
            try:
              rects[key] += int(r.group(5))
            except KeyError:
              rects[key] = int(r.group(5))
    f.close()
    directory=re.search('(^.*[^/]+)/+[^/]*$', filename).group(1)
  
  divisor = 0
  for key, value in all_rects.iteritems():
    divisor = max(divisor, float(value) / ((float(key[2]) - float(key[0])) * (float(key[3]) - float(key[1]))))

  for key, value in rects.iteritems():
    area = (key[2] - key[0]) * (key[3] - key[1])
    rgb = min(1, max(0, 1 - (value / area) / divisor))
    #print str(key[0]) + ',' + str(key[1]) + '-' + str(key[2]) + ',' + str(key[3]) + '=' + str(rgb)
    fig.axes[0].add_patch(Rectangle((key[0], key[1]), key[2] - key[0], key[3] - key[1], color=(rgb, rgb, rgb), linewidth=0, zorder=(1 - area)))

  #pylab.legend(loc=4, handlelength=4.2, numpoints=2)
  
  pylab.grid(False)
  
  pylab.xlabel('X', fontsize=8)
  pylab.ylabel('X-Dot', fontsize=8)
  pylab.title('Generated Value Function for ' + target_entries, fontsize=10)
  
  pylab.xlim(xmin=-1.2, xmax=0.6)
  pylab.ylim(ymin=-0.07, ymax=0.07)
  
  #fig.axes[0].xaxis.set_major_formatter(CommaFormatter())
  #fig.axes[0].yaxis.set_major_formatter(CommaFormatter())
  fig.axes[0].set_xticks([-1.2,0.6])
  fig.axes[0].set_yticks([-0.07,0.07])
  
  #xlabels = fig.axes[0].xaxis.get_ticklabels()
  #last_xlabel = xlabels[len(xlabels) - 1]
  #last_xlabel.set_horizontalalignment('right')
  #last_xlabel.set_x(0)
  ##fig.axes[0].yaxis.set_scale('log')
  ##print last_xlabel.get_size()
  ##print last_xlabel.get_position()
  ##print last_xlabel.get_text()
  ##print last_xlabel
  
  if len(sys.argv) == 2:
    pylab.savefig('mountain-car-' + target_entries + '.eps')
    pylab.savefig('mountain-car-' + target_entries + '.png', dpi=1200)
    pylab.savefig('mountain-car-' + target_entries + '.svg')
    plt.show()
  else:
    splitd = directory.rsplit('/', 1)
    filename = splitd[1] + '-heat'
    
    if not os.path.exists(splitd[0] + '/eps'):
      os.makedirs(splitd[0] + '/eps')
    pylab.savefig(splitd[0] + '/eps/' + filename + '-' + target_entries + '.eps')
    
    if not os.path.exists(splitd[0] + '/png'):
      os.makedirs(splitd[0] + '/png')
    pylab.savefig(splitd[0] + '/png/' + filename + '-' + target_entries + '.png', dpi=1200)
    
    if not os.path.exists(splitd[0] + '/svg'):
      os.makedirs(splitd[0] + '/svg')
    pylab.savefig(splitd[0] + '/svg/' + filename + '-' + target_entries + '.svg')

if __name__ == "__main__":
  main()
