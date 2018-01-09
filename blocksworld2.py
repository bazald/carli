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
  parser = argparse.ArgumentParser("./blocksworld2.py")
  parser.add_argument("--scenario", help="Which graph should be generated?")
  args, filenames = parser.parse_known_args()

  scenario = 0
  if args.scenario:
    scenario = int(args.scenario)

  # 1: ./blocksworld2.py --scenario 1 experiment-bw2/catde-none/*.out experiment-bw2/policy-none/*.out experiment-bw2/value-none/*.out
  # 2: ./blocksworld2.py --scenario 2 experiment-bw2/catde-catde-none/*.out experiment-bw2/policy-policy-none/*.out experiment-bw2/value-value-none/*.out
  # 3: ./blocksworld2.py --scenario 3 experiment-bw2/catde-catde-bkls/*.out experiment-bw2/policy-policy-bkls/*.out experiment-bw2/value-value-bkls/*.out
  # 4: ./blocksworld2.py --scenario 4 experiment-bw2/catde-catde-bst/*.out experiment-bw2/policy-policy-bst/*.out experiment-bw2/value-value-bst/*.out
  # 5: ./blocksworld2.py --scenario 5 experiment-bw2/catde-catde-c500/*.out experiment-bw2/policy-policy-c500/*.out experiment-bw2/value-value-c500/*.out
  # 6: ./blocksworld2.py --scenario 6 experiment-bw2/catde-none-d/*.out experiment-bw2/policy-none-d/*.out experiment-bw2/value-none-d/*.out
  # 7: ./blocksworld2.py --scenario 7 experiment-bw2/catde-catde-none-d/*.out experiment-bw2/policy-policy-none-d/*.out experiment-bw2/value-value-none-d/*.out
  # 8: ./blocksworld2.py --scenario 8 experiment-bw2/catde-catde-bkls-d/*.out experiment-bw2/policy-policy-bkls-d/*.out experiment-bw2/value-value-bkls-d/*.out
  # 9: ./blocksworld2.py --scenario 9 experiment-bw2/catde-catde-bst-d/*.out experiment-bw2/policy-policy-bst-d/*.out experiment-bw2/value-value-bst-d/*.out
  # 10: ./blocksworld2.py --scenario 10 experiment-bw2/catde-catde-c900-d/*.out experiment-bw2/policy-policy-c900-d/*.out experiment-bw2/value-value-c900-d/*.out

  memory_plot = False # scenario is not 0
  unrefinement_plot = False # not memory_plot
  two_sided_plot = memory_plot or unrefinement_plot
  cumulative = True

  if cumulative:
    reward_label = 'Cumulative Reward / \# Episodes'
    val0 = 1
  else:
    reward_label = 'Reward Within an Episode'
    val0 = 4

  if len(filenames) == 1:
    f = open('stdout.txt', 'r')
    seed = int(f.readline().split(' ', 1)[1])
    x = []
    xs = []
    smith = []
    cpu = []
    memory = []
    unrefinements = []
    while True:
      line = f.readline()
      if not line or line == '':
        break
      else:
        split = line.split(' ')
        x.append(int(split[0]))#) / 10000.0)
        xs.append(int(split[0]))
        smith.append(float(split[val0 + 1]))
        cpu.append(float(split[9]))
        memory.append(float(split[7]))
        unrefinements.append(map(int, split[10].split(':')))
    f.close()
    
    directory=''
    title='Blocks World (seed ' + str(seed) + ')'
  else:
    files = {}
    for filename in filenames[1:]:
    #for filename in filenames:
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
      files[group].smith['mem_min'] = []
      files[group].smith['mem_max'] = []
      files[group].smith['cpu'] = []
      files[group].smith['unr'] = []
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
              y_mem_min = float(split[7])
              y_mem_max = float(split[7])
              y_cpu = float(split[9]) * 1000.0
              y_unr = map(float, split[10].split(':'))
              y_count = 1
            else:
              mul1 = 1.0 / (y_count + 1.0)
              mul0 = y_count * mul1
              y_min = min(y_min, float(split[val0 + 0]))
              y_avg = y_avg * mul0 + float(split[val0 + 1]) * mul1
              y_max = max(y_max, float(split[val0 + 2]))
              y_mem = y_mem * mul0 + float(split[7]) * mul1
              y_mem_min = min(y_mem_min, float(split[7]))
              y_mem_max = max(y_mem_max, float(split[7]))
              y_cpu = y_cpu * mul0 + float(split[9]) * 1000.0 * mul1
              unr = map(float, split[10].split(':'))
              for i in range(0, min(len(y_unr), len(unr))):
                y_unr[i] = y_unr[i] * mul0 + unr[i] * mul1
              if len(y_unr) > len(unr):
                for i in range(len(unr), len(y_unr)):
                  y_unr[i] = y_unr[i] * mul0
              else:
                for i in range(len(y_unr), len(unr)):
                  y_unr.append(unr[i] * mul1)
              y_count = y_count + 1
        if not done:
          files[group].smith['min'].append(y_min)
          files[group].smith['avg'].append(y_avg)
          files[group].smith['max'].append(y_max)
          files[group].smith['mem'].append(y_mem)
          files[group].smith['mem_min'].append(y_mem_min)
          files[group].smith['mem_max'].append(y_mem_max)
          files[group].smith['cpu'].append(y_cpu)
          files[group].smith['unr'].append(y_unr)
      
      for handle in files[group].handles:
        handle.f.close()
      
      first_group = False
  
    agent_list = []
    remap_names = {}
    memory = {}
    cpu = {}
    unrefinements = {}
    smith_min = {}
    smith_max = {}
    mem_min = {}
    mem_max = {}

    if len(files) == 1 and False:
      title='Blocks World (' + group.rsplit('/',1)[1].replace('_', '\_') + ')'
      smith = files[group].smith
      mode = 'single experiment evaluation'
      
      agent = group.rsplit('/',1)[1].replace('_', '\_')
      remap_names[agent] = agent
      agent_list.append(agent)
      memory[agent] = smith['mem']
      cpu[agent] = smith['cpu']
      unrefinements[agent] = smith['unr']
      smith_min[agent] = smith['min']
      smith_max[agent] = smith['max']
      mem_min[agent] = smith['mem_min']
      mem_max[agent] = smith['mem_max']
    else:
      title='Blocks World (' + group.rsplit('/',1)[0].replace('_', '\_') + ')'
      
      smith = {}
      for group in files:
        agent = group.rsplit('/',1)[1].replace('_', '\_')
        remap_names[agent] = agent
        agent_list.append(agent)
        smith[agent] = files[group].smith['avg']
        memory[agent] = files[group].smith['mem']
        cpu[agent] = files[group].smith['cpu']
        unrefinements[agent] = files[group].smith['unr']
        smith_min[agent] = files[group].smith['min']
        smith_max[agent] = files[group].smith['max']
        mem_min[agent] = files[group].smith['mem_min']
        mem_max[agent] = files[group].smith['mem_max']
      
      mode = 'multiple experiment evaluation'

  fig = plt.figure()
  fig.canvas.set_window_title('Blocks World')
  
  if two_sided_plot:
    rect = [0.19,0.17,0.65,0.80]
  else:
    rect = [0.17,0.17,0.80,0.80]
  pylab.axes(rect)
  
  labels = []
  if len(filenames) == 1:
    #if val0 == 4:
      #for i in range(1, len(smith)):
        #smith[i] = 0.95 * smith[i - 1] + 0.05 * smith[i];
    
    y_labels = ['Values']
    yss = [smith]
    
    labels += pylab.plot(x, smith, label="Values", color='blue', linestyle='solid')
  else:
    #if val0 == 4:
      #for a in smith:
        #for i in range(1, len(smith[a])):
          #smith[a][i] = 0.95 * smith[a][i - 1] + 0.05 * smith[a][i];
    
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
      
      #remap_names = {}
      remap_names['mt4'] = 'HOG'
      remap_names['mt5'] = 'HOG'
      remap_names['catde-none'] = 'CATDE RNN'
      remap_names['catde-none-d'] = 'CATDE RND'
      remap_names['policy-none'] = 'Policy RNN'
      remap_names['policy-none-d'] = 'Policy RND'
      remap_names['value-none'] = 'Value RNN'
      remap_names['value-none-d'] = 'Value RND'
      #remap_names['value-none'] = 'Value RNN B'
      #remap_names['value-none-d'] = 'Value RND B'
      remap_names['egreedy-value-none'] = 'Value RNN'
      remap_names['egreedy-value-none-d'] = 'Value RND'
      #remap_names['egreedy-value-none'] = 'Value RNN $\\epsilon$'
      #remap_names['egreedy-value-none-d'] = 'Value RND $\\epsilon$'
      remap_names['catde-catde-none'] = 'CATDE RUN'
      remap_names['catde-catde-none-d'] = 'CATDE RUD'
      remap_names['policy-policy-none'] = 'Policy RUN'
      remap_names['policy-policy-none-d'] = 'Policy RUD'
      remap_names['value-value-none'] = 'Value RUN'
      remap_names['value-value-none-d'] = 'Value RUD'
      remap_names['catde-catde-bkls'] = 'CATDE RBN'
      remap_names['catde-catde-bkls-d'] = 'CATDE RBD'
      remap_names['policy-policy-bkls'] = 'Policy RBN'
      remap_names['policy-policy-bkls-d'] = 'Policy RBD'
      remap_names['value-value-bkls'] = 'Value RBN'
      remap_names['value-value-bkls-d'] = 'Value RBD'
      remap_names['catde-catde-bst'] = 'CATDE RON'
      remap_names['catde-catde-bst-d'] = 'CATDE ROD'
      remap_names['policy-policy-bst'] = 'Policy RON'
      remap_names['policy-policy-bst-d'] = 'Policy ROD'
      remap_names['value-value-bst'] = 'Value RON'
      remap_names['value-value-bst-d'] = 'Value ROD'
      remap_names['catde-catde-c50'] = 'CATDE RCN'
      remap_names['catde-catde-c500'] = 'CATDE RCN'
      remap_names['catde-catde-c50-d'] = 'CATDE RCD'
      remap_names['catde-catde-c900-d'] = 'CATDE RCD'
      remap_names['policy-policy-c50'] = 'Policy RCN'
      remap_names['policy-policy-c500'] = 'Policy RCN'
      remap_names['policy-policy-c50-d'] = 'Policy RCD'
      remap_names['policy-policy-c900-d'] = 'Policy RCD'
      remap_names['value-value-c50'] = 'Value RCN'
      #remap_names['value-value-c50'] = 'Value RCN B'
      remap_names['value-value-c500'] = 'Value RCN'
      #remap_names['value-value-c500'] = 'Value RCN $\\epsilon$'
      remap_names['value-value-c50-d'] = 'Value RCD'
      #remap_names['value-value-c50-d'] = 'Value RCD B'
      remap_names['value-value-c900-d'] = 'Value RCD'
      #remap_names['value-value-c900-d'] = 'Value RCD $\\epsilon$'
      
      if scenario == 1:
        agent_list = ['catde-none', 'policy-none', 'value-none']
      elif scenario == 2:
        agent_list = ['catde-catde-none', 'policy-policy-none', 'value-value-none']
      elif scenario == 3:
        agent_list = ['catde-catde-bkls', 'policy-policy-bkls', 'value-value-bkls']
      elif scenario == 4:
        agent_list = ['catde-catde-bst', 'policy-policy-bst', 'value-value-bst']
      elif scenario == 5:
        agent_list = ['catde-catde-c500', 'policy-policy-c500', 'value-value-c500']
      elif scenario == 6:
        agent_list = ['catde-none-d', 'policy-none-d', 'value-none-d']
      elif scenario == 7:
        agent_list = ['catde-catde-none-d', 'policy-policy-none-d', 'value-value-none-d']
      elif scenario == 8:
        agent_list = ['catde-catde-bkls-d', 'policy-policy-bkls-d', 'value-value-bkls-d']
      elif scenario == 9:
        agent_list = ['catde-catde-bst-d', 'policy-policy-bst-d', 'value-value-bst-d']
      elif scenario == 10:
        agent_list = ['catde-catde-c900-d', 'policy-policy-c900-d', 'value-value-c900-d']
      elif scenario == 11:
        agent_list = ['value-none', 'value-value-none', 'value-value-bkls', 'value-value-bst', 'value-value-c500']
      elif scenario == 12:
        agent_list = ['value-none-d', 'value-value-none-d', 'value-value-bkls-d', 'value-value-bst-d', 'value-value-c900-d']
      elif scenario == 13:
        agent_list = ['mt4']
      elif scenario == 14:
        agent_list = ['mt5']
      if scenario > 0:
        for agent in agent_list:
          y_labels.append(remap_names[agent])
          yss.append(smith[agent])
          
          if scenario < 11:
            if agent.find('catde') != -1:
              color = 'red'
            elif agent.find('policy') != -1:
              color = 'green'
            elif agent.find('value') != -1:
              color = 'blue'
          elif scenario < 13:
            if agent.find('value-value-none') != -1:
              color = 'red'
            elif agent.find('-none') != -1:
              color = 'green'
            elif agent.find('-bkls') != -1:
              color = 'yellow'
            elif agent.find('-bst') != -1:
              color = 'orange'
            elif agent.find('-c') != -1:
              color = 'blue'
          #else:
            #color = None

          linestyle = '-'
          #if agent.find('-') == agent.rfind('-'):
            #linestyle = '-'
          #elif agent.find('bkls') != -1:
            #linestyle = '-.'
          #elif agent.find('bst') != -1:
            #linestyle = '-.'
          #elif agent.find('00') != -1:
            #linestyle = '--'
          #else:
            #linestyle = ':'

          labels += pylab.plot(x, smith[agent], label=remap_names[agent], color=color, linestyle=linestyle)
  
  pylab.grid(False)
  
  pylab.xlabel('Step Number', fontsize=8)
  pylab.ylabel(reward_label, fontsize=8)
  
  pylab.ylim(ymax=0)
  #pylab.xlim(xmax=10000)
  #if len(filenames) > 1:
    #if cumulative:
      #pylab.ylim(ymin=-250, ymax=0)
      #pylab.xlim(xmin=0, xmax=10000)
      #pylab.ylim(ymin=-50, ymax=0)
    #else:
      #pylab.ylim(ymin=-1000, ymax=0)
  
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
  
  if memory_plot:
    ax2 = fig.axes[0].twinx()
    ax2.xaxis.set_major_formatter(CommaFormatter())
    ax2.yaxis.set_major_formatter(CommaFormatter())

    for agent in agent_list:
      #y_labels.append('CPU: ' + remap_names[agent])
      #yss.append(memory[agent])
      y_labels.append('Mem: ' + remap_names[agent])
      yss.append(memory[agent])
      
      color = 'red'
      if agent is 'sof4' or agent is 'fof4' or agent is 'cof4' or agent is 'vof4' or agent is 'pof4':
        linestyle = ':'
      elif agent is 'snf4' or agent is 'fnf4' or agent is 'cnf4' or agent is 'vnf4' or agent is 'pnf4':
        linestyle = '--'
      elif agent is 'sonf4' or agent is 'fonf4' or agent is 'conf4' or agent is 'vonf4' or agent is 'ponf4':
        linestyle = '-.'
      else:
        linestyle = '-'
        
      #labels += pylab.plot(x, cpu[agent], label='CPU: ' + remap_names[agent], color=color, linestyle=linestyle)
      labels += pylab.plot(x, memory[agent], label='Mem: ' + remap_names[agent], color=color, linestyle=linestyle)

      print 'CPU Average for ' + agent + ': ' + str(cpu[agent][-1])
      print 'Memory Average for ' + agent + ': ' + str(memory[agent][-1])
    #ax2.set_xlim(0, 5000)
    #ax2.set_ylim(0, 2)
    #ax2.set_ylim(0, 100)
    
    #ax2.set_ylabel(r"Temperature ($^\circ$C)")
    #ax2.set_ylabel('CPU Time / Step (Milliseconds)')
    ax2.set_ylabel('Number of Tiles or Weights')
    #fig.axes[0].spines['left'].set_color('red')
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

    #if scenario is 5:
      #pylab.legend(labels, [l.get_label() for l in labels], loc=4, handlelength=4.2, numpoints=2, bbox_to_anchor=(-0.05,0.25,1,1))
  elif unrefinement_plot:
    ax2 = fig.axes[0].twinx()
    ax2.xaxis.set_major_formatter(CommaFormatter())
    ax2.yaxis.set_major_formatter(CommaFormatter())

    for agent in agent_list:
      if agent.find("none") is not -1:
        y_labels.append('Unr: ' + remap_names[agent])
        yss.append(unrefinements[agent])
        
        linestyle = ':'
        
        imax = len(unrefinements[agent][len(x) - 1])
        for i in range(0, imax):
          y = []
          if unrefinements[agent][len(x) - 1][i] > 0.0:
            for unr in unrefinements[agent]:
              if i < len(unr):
                y.append(unr[i])
              else:
                y.append(0.0)

            #labels += 
            pylab.plot(x, y, label='Unr('+str(i)+'): ' + remap_names[agent], color=str(float(i) / imax), linestyle=linestyle)
            plotted = True

      print 'Unrefinement Average for ' + agent + ': ' + str(sum(unrefinements[agent][len(x) - 1]))
    ax2.set_xlim(0)
    ax2.set_ylim(0)
    
    ax2.set_ylabel('Unrefinements')
    fig.axes[0].tick_params(axis='y', colors='blue')
    ax2.spines['right'].set_color('red')
    ax2.tick_params(axis='y', colors='red')

    # Fix right axis tick labels
    al=ax2.get_yticks().tolist()
    al2=[]
    for a in al:
      al2.append(str(a))
    ax2.set_yticklabels(al2)
    
    ### upper left
    #legend = pylab.legend(labels, [l.get_label() for l in labels], loc=2, handlelength=4.2, numpoints=2, prop={'size':4}, bbox_to_anchor=(0.03,-0.05,0,1))
    ## lower center
    #legend = pylab.legend(labels, [l.get_label() for l in labels], handlelength=4.2, numpoints=2, prop={'size':4}, bbox_to_anchor=(0.3,0.3,0.5,0))
    # lower right
    legend = pylab.legend(labels, [l.get_label() for l in labels], loc=4, handlelength=4.2, numpoints=2, prop={'size':4}, bbox_to_anchor=(-0.03,0.05,1,1))
  else:
    # lower right
    pylab.legend(labels, [l.get_label() for l in labels], loc=4, handlelength=4.2, numpoints=2)
    
    if mode == 'single experiment evaluation':
      for agent in agent_list:
        #print '50k Value for ' + agent + ': ' + str(smith[agent][x.index(50000)])
        #print '50k CPU Average for ' + agent + ': ' + str(cpu[agent][x.index(50000)])
        #print '50k Memory Average for ' + agent + ': ' + str(memory[agent][x.index(50000)])
        print 'Final Value for ' + agent + ': ' + str(smith['avg'][-1])
        print 'Final CPU Average for ' + agent + ': ' + str(cpu[agent][-1])
        print 'Final Memory Average for ' + agent + ': ' + str(memory[agent][-1])
    elif mode == 'multiple experiment evaluation':
      for agent in agent_list:
        #print '50k Value for ' + agent + ': ' + str(smith[agent][x.index(50000)])
        #print '50k CPU Average for ' + agent + ': ' + str(cpu[agent][x.index(50000)])
        #print '50k Memory Average for ' + agent + ': ' + str(memory[agent][x.index(50000)])
        print 'Final Value for ' + agent + ': ' + str(smith[agent][-1])
        print 'Final CPU Average for ' + agent + ': ' + str(cpu[agent][-1])
        print 'Final Memory Average for ' + agent + ': ' + str(memory[agent][-1])
  
  #xposition = [5000, 15000]
  #for xc in xposition:
    #plt.axvline(x=xc, color='green', linestyle=':')
  #for x in range(1,13):
    ##print [4000 * x]
    ##print [smith[agent][40 * x]]
    ##print [smith[agent][40 * x] - smith_min[agent][40 * x], smith_max[agent][40 * x] - smith[agent][40 * x]]
    #fig.axes[0].errorbar([4000 * x], [smith[agent][40 * x]], yerr=[[smith[agent][40 * x] - smith_min[agent][40 * x]], [smith_max[agent][40 * x] - smith[agent][40 * x]]], ecolor='blue')
  #for x in range(1,13):
    ##print [4000 * x - 2000]
    ##print [memory[agent][int(40 * x - 20)]]
    ##print [memory[agent][int(40 * x - 20)] - mem_min[agent][int(40 * x - 20)], mem_max[agent][int(40 * x - 20)] - memory[agent][int(40 * x - 20)]]
    #ax2.errorbar([4000 * x - 2000], [memory[agent][int(40 * x - 20)]], yerr=[[memory[agent][int(40 * x - 20)] - mem_min[agent][int(40 * x - 20)]], [mem_max[agent][int(40 * x - 20)] - memory[agent][int(40 * x - 20)]]], ecolor='red')

  #xposition = [50000]
  #for xc in xposition:
    #plt.axvline(x=xc, color='green', linestyle=':')
  #for x in range(1,13):
    #print [8000 * x]
    #print [smith[agent][80 * x]]
    #print [smith[agent][80 * x] - smith_min[agent][80 * x], smith_max[agent][80 * x] - smith[agent][80 * x]]
    #fig.axes[0].errorbar([8000 * x], [smith[agent][80 * x]], yerr=[[smith[agent][80 * x] - smith_min[agent][80 * x]], [smith_max[agent][80 * x] - smith[agent][80 * x]]], ecolor='blue')
  #for x in range(1,13):
    #print [8000 * x - 4000]
    #print [memory[agent][int(80 * x - 40)]]
    #print [memory[agent][int(80 * x - 40)] - mem_min[agent][int(80 * x - 40)], mem_max[agent][int(80 * x - 40)] - memory[agent][int(80 * x - 40)]]
    #ax2.errorbar([8000 * x - 4000], [memory[agent][int(80 * x - 40)]], yerr=[[memory[agent][int(80 * x - 40)] - mem_min[agent][int(80 * x - 40)]], [mem_max[agent][int(80 * x - 40)] - memory[agent][int(80 * x - 40)]]], ecolor='red')
  
  if len(filenames) == 1:
    write_to_csv('blocksworld2.csv', 'Step Number', xs, y_labels, yss)
    pylab.savefig('blocksworld2.eps')
    pylab.savefig('blocksworld2.png', dpi=1200)
    pylab.savefig('blocksworld2.svg')
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
