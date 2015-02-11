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
    if c is '0':
      return arg
    else:
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
  reward_label = 'Cumulative Reward / \# Episodes'
  val0 = 1
  #reward_label = 'Reward Within an Episode'
  #val0 = 4
  
  # 1: ./blocksworld2.py experiment-bw2/c*4/*.out
  # 2: ./blocksworld2.py experiment-bw2/v*4/*.out
  # 3: ./blocksworld2.py experiment-bw2/p*4/*.out
  scenario = 0

  two_sided_plot = scenario is not 0

  if len(sys.argv) == 1:
    f = open('stdout.txt', 'r')
    seed = int(f.readline().split(' ', 1)[1])
    x = []
    xs = []
    smith = []
    memory = []
    while True:
      line = f.readline()
      if not line or line == '':
        break
      else:
        split = line.split(' ')
        x.append(int(split[0]))#) / 10000.0)
        xs.append(int(split[0]))
        smith.append(float(split[val0 + 1]))
        memory.append(float(split[7]))
    f.close()
    
    directory=''
    title='Blocks World (seed ' + str(seed) + ')'
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
      files[group].smith['mem'] = []
      files[group].smith['cpu'] = []
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
                x.append(int(split[0]))#/ 10000.0)
                xs.append(int(split[0]))
              y_min = float(split[val0 + 0])
              y_avg = float(split[val0 + 1])
              y_max = float(split[val0 + 2])
              y_mem = float(split[7])
              y_cpu = float(split[9])
              y_count = 1
            else:
              y_min = min(y_min, float(split[val0 + 0]))
              y_avg = y_avg * (y_count / (y_count + 1.0)) + float(split[val0 + 1]) / (y_count + 1.0)
              y_max = max(y_max, float(split[val0 + 2]))
              y_mem = y_mem * (y_count / (y_count + 1.0)) + float(split[7]) / (y_count + 1.0)
              y_cpu = y_cpu * (y_count / (y_count + 1.0)) + float(split[9]) / (y_count + 1.0)
              y_count = y_count + 1
        if not done:
          files[group].smith['min'].append(y_min)
          files[group].smith['avg'].append(y_avg)
          files[group].smith['max'].append(y_max)
          files[group].smith['mem'].append(y_mem)
          files[group].smith['cpu'].append(y_cpu)
      
      for handle in files[group].handles:
        handle.f.close()
      
      first_group = False
  
    if len(files) == 1:
      title='Blocks World (' + group.rsplit('/',1)[1].replace('_', '\_') + ')'
      smith = files[group].smith
      mode = 'single experiment evaluation'
    else:
      title='Blocks World (' + group.rsplit('/',1)[0].replace('_', '\_') + ')'
      
      smith = {}
      memory = {}
      cpu = {}
      for group in files:
        smith[group.rsplit('/',1)[1].replace('_', '\_')] = files[group].smith['avg']
        memory[group.rsplit('/',1)[1].replace('_', '\_')] = files[group].smith['mem']
        cpu[group.rsplit('/',1)[1].replace('_', '\_')] = files[group].smith['cpu']
      
      mode = 'multiple experiment evaluation'

  fig = plt.figure()
  fig.canvas.set_window_title('Blocks World')
  
  if two_sided_plot:
    rect = [0.19,0.15,0.6525,0.82]
  else:
    rect = [0.19,0.15,0.7725,0.82]
  pylab.axes(rect)
  
  labels = []
  if len(sys.argv) == 1:
    if val0 == 4:
      for i in range(1, len(smith)):
        smith[i] = 0.95 * smith[i - 1] + 0.05 * smith[i];
    
    y_labels = ['Values']
    yss = [smith]
    
    labels += pylab.plot(x, smith, label="Values", color='blue', linestyle='solid')
  else:
    if val0 == 4:
      for a in smith:
        for i in range(1, len(smith[a])):
          smith[a][i] = 0.95 * smith[a][i - 1] + 0.05 * smith[a][i];
    
    if mode == 'single experiment evaluation':
      y_labels = ['Maximum', 'Average', 'Minimum']
      yss = [smith['max'], smith['avg'], smith['min']]
      
      labels += pylab.plot(x, smith['max'], label="Maximum", color='black', linestyle='dotted')
      labels += pylab.plot(x, smith['avg'], label="Average", color='black', linestyle='solid')
      labels += pylab.plot(x, smith['min'], label="Minimum", color='black', linestyle='dashed')
      #labels += pylab.plot(x, smith['max'], label="Maximum", color='green', linestyle='solid')
      ##labels += pylab.plot(x, smith['med'], label="Median", color='brown', linestyle='solid')
      #labels += pylab.plot(x, smith['min'], label="Minimum", color='teal', linestyle='solid')
      #labels += pylab.plot(x, smith['avg'], label="Average", color='blue', linestyle='solid')
    else:
      y_labels = []
      yss = []
      
      if scenario == 0:
        for agent in smith:
          y_labels.append(agent)
          yss.append(smith[agent])
          
          labels += pylab.plot(x, smith[agent], label=agent, linestyle='solid')
      
      remap_names = {}
      remap_names['cof4'] = 'Non-Rel CATDE'
      remap_names['vof4'] = 'Non-Rel Value'
      remap_names['pof4'] = 'Non-Rel Policy'
      remap_names['cnf4'] = 'Rel CATDE'
      remap_names['vnf4'] = 'Rel Value'
      remap_names['pnf4'] = 'Rel Policy'
      remap_names['conf4'] = 'Mixed CATDE'
      remap_names['vonf4'] = 'Mixed Value'
      remap_names['ponf4'] = 'Mixed Policy'
      
      if scenario == 1:
        agent_list = ['cof4', 'cnf4', 'conf4']
      elif scenario == 2:
        agent_list = ['vof4', 'vnf4', 'vonf4']
      elif scenario == 3:
        agent_list = ['pof4', 'pnf4', 'ponf4']
      if scenario > 0 and scenario < 4:
        for agent in agent_list:
          y_labels.append(remap_names[agent])
          yss.append(smith[agent])
          
          color = 'blue'
          if agent is 'cof4' or agent is 'vof4' or agent is 'pof4':
            linestyle = ':'
          elif agent is 'cnf4' or agent is 'vnf4' or agent is 'pnf4':
            linestyle = '--'
          elif agent is 'conf4' or agent is 'vonf4' or agent is 'ponf4':
            linestyle = '-.'
          
          labels += pylab.plot(x, smith[agent], label=remap_names[agent], color=color, linestyle=linestyle)
  
  pylab.grid(False)
  
  pylab.xlabel('Step Number', fontsize=8)
  pylab.ylabel(reward_label, fontsize=8)
  
  pylab.xlim(xmax=5000)
  if len(sys.argv) > 1:
    pylab.ylim(ymin=-500, ymax=0)
  
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
  
  if two_sided_plot:
    ax2 = fig.axes[0].twinx()
    ax2.xaxis.set_major_formatter(CommaFormatter())
    ax2.yaxis.set_major_formatter(CommaFormatter())

    for agent in agent_list:
      y_labels.append('CPU: ' + remap_names[agent])
      yss.append(cpu[agent])
      #y_labels.append('Mem: ' + remap_names[agent])
      #yss.append(memory[agent])
      
      color = 'red'
      if agent is 'cof4' or agent is 'vof4' or agent is 'pof4':
        linestyle = ':'
      elif agent is 'cnf4' or agent is 'vnf4' or agent is 'pnf4':
        linestyle = '--'
      elif agent is 'conf4' or agent is 'vonf4' or agent is 'ponf4':
        linestyle = '-.'
        
      labels += pylab.plot(x, cpu[agent], label='CPU: ' + remap_names[agent], color=color, linestyle=linestyle)
      #labels += pylab.plot(x, memory[agent], label='Mem: ' + remap_names[agent], color=color, linestyle=linestyle)

      print 'Memory Average for ' + agent + ': ' + str(memory[agent][-1])
    ax2.set_xlim(0, 5000)
    ax2.set_ylim(0, 0.002)
    #ax2.set_ylim(0, 100)
    
    #ax2.set_ylabel(r"Temperature ($^\circ$C)")
    ax2.set_ylabel('CPU Time / Step (Seconds)')
    #ax2.set_ylabel('Number of Tiles / Weights')
    #fig.axes[0].spines['left'].set_color('red')
    fig.axes[0].tick_params(axis='y', colors='blue')
    #fig.axes[0].yaxis.label.set_color('blue')
    ax2.spines['right'].set_color('red')
    ax2.tick_params(axis='y', colors='red')
    #ax2.yaxis.label.set_color('red')
    
    # lower right
    #pylab.legend(labels, [l.get_label() for l in labels], loc=4, handlelength=4.2, numpoints=2, bbox_to_anchor=(0,0.06,1,1))
  else:
    # lower right
    pylab.legend(labels, [l.get_label() for l in labels], loc=4, handlelength=4.2, numpoints=2)
  
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
