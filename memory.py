#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
  scenario = 5
  
  if len(sys.argv) == 1:
    f = open('stdout.txt', 'r')
    seed = int(f.readline().split(' ', 1)[1])
    x = []
    smith = []
    while True:
      line = f.readline()
      if not line or line == '':
        break
      else:
        split = line.split(' ')
        x.append(int(split[0]))
        smith.append(float(split[7]))
    f.close()
    
    directory=''
    title='Value Function Size (seed ' + str(seed) + ')'
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
                x.append(int(split[0]))#) / 10000.0)
                xs.append(int(split[0]))
              y_avg = float(split[7])
              y_count = 1
            else:
              y_avg = y_avg * (y_count / (y_count + 1.0)) + float(split[7]) / (y_count + 1.0)
              y_count = y_count + 1
        if not done:
          files[group].smith['avg'].append(y_avg)
      
      for handle in files[group].handles:
        handle.f.close()
      
      first_group = False
    
    if len(files) == 1:
      title='Value Function Size (' + group.rsplit('/',1)[1].replace('_', '\_') + ')'
      smith = files[group].smith
      mode = 'single experiment evaluation'
    else:
      title='Value Function Size (' + group.rsplit('/',1)[0].replace('_', '\_') + ')'
      
      smith = {}
      for group in files:
        smith[group.rsplit('/',1)[1].replace('_', '\_')] = files[group].smith['avg']
      
      mode = 'multiple experiment evaluation'
  
  fig = plt.figure()
  fig.canvas.set_window_title('Value Function Size')
  
  rect = [0.19,0.15,0.7725,0.82]
  pylab.axes(rect)
  
  labels = []
  if len(sys.argv) == 1:
    y_labels = ['Values']
    yss = [smith]
    
    pylab.plot(x, smith, label="Number of Tiles / Weights", color='blue', linestyle='solid')
  else:
    if mode == 'single experiment evaluation':
      y_labels = ['Maximum', 'Average', 'Minimum']
      yss = [smith['avg']]
      
      pylab.plot(x, smith['avg'], label="Average", color='black', linestyle='solid')
    else:
      y_labels = []
      yss = []
      
      if scenario == 0:
        for agent in smith:
          y_labels.append(agent)
          yss.append(smith[agent])
          
          labels += pylab.plot(x, smith[agent], label=agent, linestyle='solid')
      
      remap_names = {}
      remap_names['specific\\_4x4\\_4x4\\_0'] = '4x4'
      remap_names['specific\\_8x8\\_8x8\\_0'] = '8x8'
      remap_names['specific\\_16x16\\_16x16\\_0'] = '16x16'
      remap_names['specific\\_32x32\\_32x32\\_0'] = '32x32'
      remap_names['specific\\_64x64\\_64x64\\_0'] = '64x64'
      remap_names['even\\_64x64\\_64x64\\_1'] = '1-64 static even'
      remap_names['even\\_2x2\\_64x64\\_3'] = '1-64 incremental even'
      remap_names['specific\\_16x16\\_16x16\\_0'] = '16x16'
      remap_names['specific\\_32x32\\_32x32\\_0'] = '32x32'
      remap_names['specific\\_64x64\\_64x64\\_0'] = '64x64'
      remap_names['specific\\_128x128\\_128x128\\_0'] = '128x128'
      remap_names['specific\\_256x256\\_256x256\\_0'] = '256x256'
      remap_names['even\\_256x256\\_256x256\\_1'] = '1-256 static even'
      remap_names['even\\_2x2\\_256x256\\_3'] = '1-256 incremental even'
      remap_names['inv-log-update-count\\_2x2\\_256x256\\_3'] = r'1-256 incremental $1/\ln$'
      
      if scenario == 4:
        agent_list = ['even\\_64x64\\_64x64\\_1', 'even\\_2x2\\_64x64\\_3']
        for agent in agent_list:
          y_labels.append('Weights: ' + remap_names[agent])
          yss.append(smith[agent])
          
          if agent is 'even\\_64x64\\_64x64\\_1':
            color = 'red'
            linestyle = '-'
          elif agent is 'even\\_2x2\\_64x64\\_3':
            color = 'red'
            linestyle = '--'
          
          labels += pylab.plot(x, smith[agent], label='Weights: ' + remap_names[agent], color=color, linestyle=linestyle)
      elif scenario == 5:
        agent_list = ['even\\_256x256\\_256x256\\_1', 'even\\_2x2\\_256x256\\_3', 'inv-log-update-count\\_2x2\\_256x256\\_3']
        for agent in agent_list:
          y_labels.append('Weights: ' + remap_names[agent])
          yss.append(smith[agent])
          
          if agent is 'even\\_256x256\\_256x256\\_1':
            color = 'red'
            linestyle = '-'
          elif agent is 'even\\_2x2\\_256x256\\_3':
            color = 'red'
            linestyle = '--'
          elif agent is 'inv-log-update-count\\_2x2\\_256x256\\_3':
            color = 'red'
            linestyle = ':'
          
          labels += pylab.plot(x, smith[agent], label='Weights: ' + remap_names[agent], color=color, linestyle=linestyle)
  
  pylab.grid(True)
  
  #pylab.title(title, fontsize=10)
  pylab.xlabel('Step Number', fontsize=8)
  pylab.ylabel('Number of Tiles / Weights', fontsize=8)
  pylab.ylim(ymin=0)
  
  if scenario == 4:
    pylab.xlim(xmax=20000)
    
    pylab.legend(loc=4, handlelength=4.2, numpoints=2, bbox_to_anchor=(0,0.12,1,1))
  elif scenario == 5:
    pylab.xlim(xmax=200000)
    
    pylab.legend(loc=4, handlelength=4.2, numpoints=2, bbox_to_anchor=(0,0.12,1,1))
  else:
    pylab.legend(loc=4, handlelength=4.2, numpoints=2)
  
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
    write_to_csv('memory.csv', 'Step Number', xs, y_labels, yss)
    pylab.savefig('memory.eps')
    pylab.savefig('memory.png', dpi=1200)
    pylab.savefig('memory.svg')
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
    write_to_csv(splitd[0] + '/csv/' + filename + '-memory.csv', 'Step Number', xs, y_labels, yss)
    
    if not os.path.exists(splitd[0] + '/eps'):
      os.makedirs(splitd[0] + '/eps')
    pylab.savefig(splitd[0] + '/eps/' + filename + '-memory.eps')
    
    if not os.path.exists(splitd[0] + '/png'):
      os.makedirs(splitd[0] + '/png')
    pylab.savefig(splitd[0] + '/png/' + filename + '-memory.png', dpi=1200)
    
    if not os.path.exists(splitd[0] + '/svg'):
      os.makedirs(splitd[0] + '/svg')
    pylab.savefig(splitd[0] + '/svg/' + filename + '-memory.svg')

if __name__ == "__main__":
  main()
