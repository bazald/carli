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
  reward_label = 'Cumulative Reward / \# Episodes'
  val0 = 1
  #reward_label = 'Reward Within an Episode'
  #val0 = 4
  
  # 1: ./mountaincar.py experiment-mc/*_0/*.out
  # 2: ./mountaincar.py experiment-mc/*_0/*.out
  # 3: ./mountaincar.py experiment-mc/*_0/*.out experiment-mc/*_1/*.out experiment-mc/cmac_*/*.out
  # 4: ./mountaincar.py experiment-mc/*_1/*.out experiment-mc/even_*_3/*.out experiment-mc/even_*_4/*.out
  # 5: ./mountaincar.py experiment-mc/*_1/*.out experiment-mc/even_*_3/*.out experiment-mc/inv-log-update-count_*_3/*.out
  # 6: ./mountaincar.py experiment-mc/*_1/*.out experiment-mc/*_3/*.out
  scenario = 0

  two_sided_plot = scenario > 3 and scenario < 6

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
        x.append(int(split[0]))
        xs.append(int(split[0]))
        smith.append(float(split[val0 + 1]))
        memory.append(float(split[7]))
    f.close()
    
    directory=''
    title='Mountain Car (seed ' + str(seed) + ')'
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
                x.append(int(split[0]))
                xs.append(int(split[0]))
              y_min = float(split[val0 + 0])
              y_avg = float(split[val0 + 1])
              y_max = float(split[val0 + 2])
              y_mem = float(split[7])
              y_count = 1
            else:
              y_min = min(y_min, float(split[val0 + 0]))
              y_avg = y_avg * (y_count / (y_count + 1.0)) + float(split[val0 + 1]) / (y_count + 1.0)
              y_max = max(y_max, float(split[val0 + 2]))
              y_mem = y_mem * (y_count / (y_count + 1.0)) + float(split[7]) / (y_count + 1.0)
              y_count = y_count + 1
        if not done:
          files[group].smith['min'].append(y_min)
          files[group].smith['avg'].append(y_avg)
          files[group].smith['max'].append(y_max)
          files[group].smith['mem'].append(y_mem)
      
      for handle in files[group].handles:
        handle.f.close()
      
      first_group = False
    
    if len(files) == 1:
      title='Mountain Car (' + group.rsplit('/',1)[1].replace('_', '\_') + ')'
      smith = files[group].smith
      mode = 'single experiment evaluation'
    else:
      title='Mountain Car (' + group.rsplit('/',1)[0].replace('_', '\_') + ')'
      
      smith = {}
      memory = {}
      for group in files:
        smith[group.rsplit('/',1)[1].replace('_', '\_')] = files[group].smith['avg']
        memory[group.rsplit('/',1)[1].replace('_', '\_')] = files[group].smith['mem']
      
      mode = 'multiple experiment evaluation'
  
  fig = plt.figure()
  fig.canvas.set_window_title('Mountain Car')
  
  if two_sided_plot:
    rect = [0.17,0.15,0.6725,0.83]
  else:
    rect = [0.17,0.15,0.7925,0.83]
  pylab.axes(rect)
  
  labels = []
  if len(sys.argv) == 1:
    if val0 == 4:
      for i in range(1, len(smith)):
        smith[i] = 0.95 * smith[i - 1] + 0.05 * smith[i];
    
    y_labels = ['Values']
    yss = [smith]
    
    pylab.plot(x, smith, label="Values", color='blue', linestyle='solid')
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
      
      #if scenario == 3:
        #for agent in smith:
          #y_labels.append(agent)
          #yss.append(smith[agent])
          
          #labels += pylab.plot(x, smith[agent], label=agent, linestyle='solid')

      remap_names = {}
      remap_names['specific\\_8x8\\_8x8\\_0'] = '8x8'
      remap_names['specific\\_16x16\\_16x16\\_0'] = '16x16'
      remap_names['specific\\_32x32\\_32x32\\_0'] = '32x32'
      remap_names['specific\\_64x64\\_64x64\\_0'] = '64x64'
      remap_names['specific\\_128x128\\_128x128\\_0'] = '128x128'
      remap_names['specific\\_256x256\\_256x256\\_0'] = '256x256'
      remap_names['even\\_256x256\\_256x256\\_1'] = '1-256 static even'
      remap_names['inv-log-update-count\\_256x256\\_256x256\\_1'] = '1-256 static $1/\ln$'
      remap_names['inv-root-update-count\\_256x256\\_256x256\\_1'] = '1-256 static $1/\sqrt{~~~}$'
      remap_names['even\\_2x2\\_256x256\\_3'] = '1-256 incremental even'
      remap_names['even\\_2x2\\_256x256\\_4'] = '1-256 incremental in'
      remap_names['inv-log-update-count\\_2x2\\_256x256\\_3'] = r'1-256 incremental $1/\ln$'
      remap_names['inv-root-update-count\\_2x2\\_256x256\\_3'] = r'1-256 incremental $1/\sqrt{~~~}$'
      remap_names['specific\\_2x2\\_256x256\\_3'] = '1-256 incremental specific'
      remap_names['cmac\\_0\\_8\\_16'] = '8x8 CMAC, 16 tilings'
      remap_names['cmac\\_0\\_16\\_16'] = '16x16 CMAC, 16 tilings'
      remap_names['cmac\\_0\\_32\\_16'] = '32x32 CMAC, 16 tilings'

      if scenario == 1:
        agent_list = ['specific\\_16x16\\_16x16\\_0', 'specific\\_32x32\\_32x32\\_0', 'specific\\_64x64\\_64x64\\_0', 'specific\\_128x128\\_128x128\\_0', 'specific\\_256x256\\_256x256\\_0']
      elif scenario == 2:
        agent_list = ['specific\\_16x16\\_16x16\\_0', 'specific\\_32x32\\_32x32\\_0', 'specific\\_64x64\\_64x64\\_0', 'specific\\_128x128\\_128x128\\_0', 'specific\\_256x256\\_256x256\\_0']
      elif scenario == 3:
        agent_list = ['even\\_256x256\\_256x256\\_1',
                      'cmac\\_0\\_8\\_16',
                      'cmac\\_0\\_16\\_16',
                      'cmac\\_0\\_32\\_16',
                      'specific\\_16x16\\_16x16\\_0',
                      'specific\\_32x32\\_32x32\\_0',
                      'specific\\_64x64\\_64x64\\_0']
      if scenario > 0 and scenario < 4:
        for agent in agent_list:
          y_labels.append(remap_names[agent])
          yss.append(smith[agent])
          
          if agent is 'specific\\_8x8\\_8x8\\_0':
            color = 'pink'
            linestyle = '-'
          elif agent is 'specific\\_16x16\\_16x16\\_0':
            color = 'blue'
            linestyle = ':'
          elif agent is 'specific\\_32x32\\_32x32\\_0':
            color = 'red'
            linestyle = ':'
          elif agent is 'specific\\_64x64\\_64x64\\_0':
            color = 'teal'
            linestyle = ':'
          if agent is 'specific\\_128x128\\_128x128\\_0':
            color = 'grey'
            linestyle = '-'
          elif agent is 'specific\\_256x256\\_256x256\\_0':
            color = 'green'
            linestyle = '-'
          elif agent is 'even\\_256x256\\_256x256\\_1':
            color = 'black'
            linestyle = '-'
          elif agent is 'cmac\\_0\\_8\\_16':
            color = 'blue'
            linestyle = '--'
          elif agent is 'cmac\\_0\\_16\\_16':
            color = 'red'
            linestyle = '--'
          elif agent is 'cmac\\_0\\_32\\_16':
            color = 'teal'
            linestyle = '--'
          
          labels += pylab.plot(x, smith[agent], label=remap_names[agent], color=color, linestyle=linestyle)
      
      if scenario == 4:
        agent_list = ['even\\_2x2\\_256x256\\_4', 'even\\_256x256\\_256x256\\_1', 'even\\_2x2\\_256x256\\_3']
      elif scenario == 5:
        agent_list = ['even\\_256x256\\_256x256\\_1', 'inv-log-update-count\\_2x2\\_256x256\\_3', 'even\\_2x2\\_256x256\\_3']
        agent_list_mem = ['even\\_256x256\\_256x256\\_1', 'even\\_2x2\\_256x256\\_3', 'inv-log-update-count\\_2x2\\_256x256\\_3']
      if scenario > 3 and scenario < 6:
        for agent in agent_list:
          y_labels.append('Reward: ' + remap_names[agent])
          yss.append(smith[agent])
          
          if agent is 'even\\_256x256\\_256x256\\_1':
            color = 'blue'
            linestyle = '-'
          elif agent is 'even\\_2x2\\_256x256\\_3':
            color = 'blue'
            linestyle = '--'
          elif agent is 'even\\_2x2\\_256x256\\_4':
            color = 'blue'
            linestyle = '-.'
          elif agent is 'inv-log-update-count\\_2x2\\_256x256\\_3':
            color = 'blue'
            linestyle = ':'
          
          labels += pylab.plot(x, smith[agent], label='Reward: ' + remap_names[agent], color=color, linestyle=linestyle)

      if scenario == 6:
        agent_list = ['even\\_2x2\\_256x256\\_3',
                      #'inv-log-update-count\\_2x2\\_256x256\\_3',
                      #'inv-root-update-count\\_2x2\\_256x256\\_3',
                      'specific\\_2x2\\_256x256\\_3']
        for agent in agent_list:
          y_labels.append(remap_names[agent])
          yss.append(smith[agent])
          
          if agent is 'even\\_2x2\\_256x256\\_3':
            color = 'blue'
            linestyle = '-'
          elif agent is 'inv-log-update-count\\_2x2\\_256x256\\_3':
            color = 'blue'
            linestyle = ':'
          elif agent is 'inv-root-update-count\\_2x2\\_256x256\\_3':
            color = 'blue'
            linestyle = '-.'
          elif agent is 'specific\\_2x2\\_256x256\\_3':
            color = 'red'
            linestyle = '-'
          
          labels += pylab.plot(x, smith[agent], label=remap_names[agent], color=color, linestyle=linestyle)
  
  pylab.grid(False)
  
  pylab.xlabel('Step Number', fontsize=8)
  pylab.ylabel(reward_label, fontsize=8)
  
  if scenario == 0:
    #pylab.title(title, fontsize=10)
    if len(sys.argv) > 1:
      pylab.ylim(ymin=-500, ymax=0)
  elif scenario == 1:
    #pylab.title('Mountain Car: Single Level Tilings', fontsize=10)
    pylab.ylim(ymin=-7000, ymax=0)
  elif scenario == 2:
    #pylab.title('Mountain Car: Single Level Tilings Expanded', fontsize=10)
    pylab.xlim(xmax=100000)
    pylab.ylim(ymin=-7000, ymax=0)
  elif scenario == 3:
    override = {'x': '0.45', 'fontsize': 'medium', 'verticalalignment': 'baseline', 'horizontalalignment': 'center'}
    #pylab.title('Mountain Car: Includes Static Hierarchical Tiling 1-256', fontsize=10, fontdict=override)
    pylab.xlim(xmax=100000)
    pylab.ylim(ymin=-7000, ymax=0)
  elif scenario == 4:
    #pylab.title('Mountain Car: Static and Incremental Hierarchical Tiling', fontsize=10)
    pylab.xlim(xmax=200000)
    pylab.ylim(ymin=-4000, ymax=0)
  elif scenario == 5:
    #pylab.title('Mountain Car: Static and Incremental Hierarchical Tilings', fontsize=10)
    pylab.xlim(xmax=200000)
    pylab.ylim(ymin=-1500, ymax=0)
  elif scenario == 6:
    #pylab.title('Mountain Car: Incremental Hierarchical Tilings', fontsize=10)
    pylab.xlim(xmax=200000)
    pylab.ylim(ymin=-35000, ymax=0)
  
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
      y_labels.append('Memory: ' + remap_names[agent])
      yss.append(memory[agent])
      
      if agent is 'even\\_256x256\\_256x256\\_1':
        color = 'red'
        linestyle = '-'
      elif agent is 'even\\_2x2\\_256x256\\_3':
        color = 'red'
        linestyle = '--'
      elif agent is 'even\\_2x2\\_256x256\\_4':
        color = 'red'
        linestyle = '-.'
      elif agent is 'inv-log-update-count\\_2x2\\_256x256\\_3':
        color = 'red'
        linestyle = ':'
      
      labels += pylab.plot(x, memory[agent], label='Memory: ' + remap_names[agent], color=color, linestyle=linestyle)
    ax2.set_xlim(0, 200000)
    ax2.set_ylim(0, 300000)
    
    #ax2.set_ylabel(r"Temperature ($^\circ$C)")
    ax2.set_ylabel('Number of Tiles / Weights')
    fig.axes[0].spines['left'].set_color('red')
    fig.axes[0].tick_params(axis='y', colors='blue')
    #fig.axes[0].yaxis.label.set_color('blue')
    ax2.spines['right'].set_color('red')
    ax2.tick_params(axis='y', colors='red')
    #ax2.yaxis.label.set_color('red')

    # Fix right axis tick labels
    al=ax2.get_yticks().tolist()
    al2=[]
    for a in al:
      al2.append(str(a))
    ax2.set_yticklabels(al2)

    # lower right
    pylab.legend(labels, [l.get_label() for l in labels], loc=4, handlelength=4.2, numpoints=2, bbox_to_anchor=(0,0.09,1,1))
  else:
    # lower right
    pylab.legend(labels, [l.get_label() for l in labels], loc=4, handlelength=4.2, numpoints=2)
  
  if len(sys.argv) == 1:
    write_to_csv('mountaincar.csv', 'Step Number', xs, y_labels, yss)
    pylab.savefig('mountaincar.eps')
    pylab.savefig('mountaincar.png', dpi=1200)
    pylab.savefig('mountaincar.svg')
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
