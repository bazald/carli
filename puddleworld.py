#!/usr/bin/env python
# -*- coding: utf-8 -*-

import hashlib, os, re
import pylab
from matplotlib import rc
from pylab import arange,pi,sin,cos,sqrt

if os.name is 'posix':
  golden_mean = (sqrt(5)-1.0)/2.0     # Aesthetic ratio
  fig_width = 3.375                   # width in inches
  fig_height = fig_width*golden_mean  # height in inches
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
  if len(sys.argv) == 1:
    f = open('stdout.txt', 'r')
    seed = int(f.readline().split(' ', 1)[1])
    x = []
    xs = []
    smith = []
    while True:
      line = f.readline()
      if not line or line == '':
        break
      else:
        split = line.split(' ')
        x.append(int(split[0]) / 10000.0)
        xs.append(int(split[0]))
        smith.append(float(split[2]))
    f.close()
    
    directory=''
    title='Puddle World (seed ' + str(seed) + ')'
  else:
    files = {}
    for filename in sys.argv[1:]:
      f = open(filename, 'r')
      seed = int(f.readline().split(' ', 1)[1])
      
      directory=re.search('(^.*[^/]+)/+[^/]*$', filename).group(1) #filename.rsplit('/', 1)[0]
      try:
        files[directory].handles.append(Handle(f, filename, seed))
      except KeyError:
        files[directory] = Handles()
        files[directory].handles.append(Handle(f, filename, seed))
    
    first_group = True
    x = []
    xs = []
    for group in files:
      files[group].smith['avg'] = []
      files[group].smith['min'] = []
      files[group].smith['max'] = []
      done = False
      while not done:
        first_handle = True
        for handle in files[group].handles:
          line = handle.f.readline()
          if not line or line == '':
            done = True
            break
          else:
            split = line.split(' ')
            if first_handle:
              first_handle = False
              if first_group:
                x.append(int(split[0]) / 10000.0)
                xs.append(int(split[0]))
              y_min = float(split[1])
              y_avg = float(split[2])
              y_max = float(split[3])
              y_count = 1
            else:
              y_min = min(y_min, float(split[1]))
              y_avg = y_avg * (y_count / (y_count + 1.0)) + float(split[2]) / (y_count + 1.0)
              y_max = max(y_max, float(split[3]))
              y_count = y_count + 1
        if not done:
          files[group].smith['min'].append(y_min)
          files[group].smith['avg'].append(y_avg)
          files[group].smith['max'].append(y_max)
      
      for handle in files[group].handles:
        handle.f.close()
      
      first_group = False
    
    if len(files) == 1:
      title='Puddle World (' + group.rsplit('/',1)[1].replace('_', '\_') + ')'
      smith = files[group].smith
      mode = 'single experiment evaluation'
    else:
      title='Puddle World (' + group.rsplit('/',1)[0].replace('_', '\_') + ')'
      
      smith = {}
      for group in files:
        smith[group.rsplit('/',1)[1].replace('_', '\_')] = files[group].smith['avg']
      
      mode = 'multiple experiment evaluation'
  
  #if title == 'Puddle World (puddle-world-overgeneral.soar\_0-0\_rl\_uperf)':
    #title = 'UPERF Performance in Puddle World (' + str(len(sys.argv) - 1) + ' Runs)'
  if title == 'Puddle World (puddle-world-overgeneral.soar\_0-0\_rl\_td-error\_0.84155\_2\_1.0\_20\_True)':
    title = 'TD-Error Performance in Puddle World (' + str(len(sys.argv) - 1) + ' Runs)'
  if title == 'Puddle World (puddle-world.soar\_1-1\_even\_td-error\_0.84155\_2\_1.0\_20\_True\_0-1-1\_0-1-1\_0-2-1\_0-2-2\_0-4-2\_0-4-4\_0-8-4\_0-8-8\_0-16-8\_0-16-16)':
    title = 'Even Credit Assignment in Puddle World (' + str(len(sys.argv) - 1) + ' Runs)'
  if title == 'Puddle World (puddle-world.soar\_1-1\_fc\_td-error\_0.84155\_2\_1.0\_20\_True\_0-1-1\_0-1-1\_0-2-1\_0-2-2\_0-4-2\_0-4-4\_0-8-4\_0-8-8\_0-16-8\_0-16-16)':
    title = '1/FC Credit Assignment in Puddle World (' + str(len(sys.argv) - 1) + ' Runs)'
  if title == 'Puddle World (puddle-world.soar\_1-1\_rl\_td-error\_0.84155\_2\_1.0\_20\_True\_0-1-1\_0-1-1\_0-2-1\_0-2-2\_0-4-2\_0-4-4\_0-8-4\_0-8-8\_0-16-8\_0-16-16)':
    title = '1/RL Credit Assignment in Puddle World (' + str(len(sys.argv) - 1) + ' Runs)'
  if title == 'Puddle World (puddle-world.soar\_1-1\_log-rl\_td-error\_0.84155\_2\_1.0\_20\_True\_0-1-1\_0-1-1\_0-2-1\_0-2-2\_0-4-2\_0-4-4\_0-8-4\_0-8-8\_0-16-8\_0-16-16)':
    title = '1/log(RL) Credit Assignment in Puddle World (' + str(len(sys.argv) - 1) + ' Runs)'
  
  fig = plt.figure()
  fig.canvas.set_window_title('Puddle World')
  
  pylab.axes([0.125,0.15,0.8375,0.75])
  
  if len(sys.argv) == 1:
    for i in range(1, len(smith)):
      smith[i] = 0.95 * smith[i - 1] + 0.05 * smith[i];
    
    y_labels = ['Values']
    yss = [smith]
    
    pylab.plot(x, smith, label="Values", color='blue', linestyle='solid')
  else:
    for a in smith:
      for i in range(1, len(smith[a])):
        smith[a][i] = 0.95 * smith[a][i - 1] + 0.05 * smith[a][i];
    
    if mode == 'single experiment evaluation':
      y_labels = ['Maximum', 'Average', 'Minimum']
      yss = [smith['max'], smith['avg'], smith['min']]
      
      pylab.plot(x, smith['max'], label="Maximum", color='black', linestyle='dotted')
      pylab.plot(x, smith['avg'], label="Average", color='black', linestyle='solid')
      pylab.plot(x, smith['min'], label="Minimum", color='black', linestyle='dashed')
      #pylab.plot(x, smith['max'], label="Maximum", color='green', linestyle='solid')
      ##pylab.plot(x, smith['med'], label="Median", color='brown', linestyle='solid')
      #pylab.plot(x, smith['min'], label="Minimum", color='teal', linestyle='solid')
      #pylab.plot(x, smith['avg'], label="Average", color='blue', linestyle='solid')
    else:
      y_labels = []
      yss = []
      for agent in smith:
        y_labels.append(agent)
        yss.append(smith[agent])
        
        pylab.plot(x, smith[agent], label=agent, linestyle='solid')
  
  pylab.legend(loc=4, handlelength=4.2, numpoints=2)
  
  pylab.grid(True)
  
  pylab.xlabel('Step Number (in 10,000s)', fontsize=8)
  pylab.ylabel('Reward / \# Episodes (Mvng Avg, n=20)', fontsize=8)
  pylab.title(title, fontsize=10)
  pylab.ylim(ymin=-250, ymax=0)
  
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
  
  if len(sys.argv) == 1:
    write_to_csv('puddleworld.csv', 'Step Number', xs, y_labels, yss)
    pylab.savefig('puddleworld.eps')
    pylab.savefig('puddleworld.png', dpi=1200)
    pylab.savefig('puddleworld.svg')
    plt.show()
  else:
    splitd = directory.rsplit('/', 1)
    
    if mode == 'single experiment evaluation':
      filename = splitd[1]
    else:
      m = hashlib.md5()
      for agent in smith:
        m.update(agent)
      filename = str(m.hexdigest())
    
    if not os.path.exists(splitd[0] + '/csv'):
      os.makedirs(splitd[0] + '/csv')
    write_to_csv(splitd[0] + '/csv/' + filename + '.csv', 'Step Number', xs, y_labels, yss)
    
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
