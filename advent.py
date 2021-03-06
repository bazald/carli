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
    if px == "800,000":
      px = "800,000 1"
    if px == "1,000,000":
      px = ",000,000"
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

  # 1: ./advent.py --scenario 1 experiment-adv/catde-none/*.out experiment-adv/policy-none/*.out experiment-adv/value-none/*.out
  # 2: ./advent.py --scenario 2 experiment-adv/catde-catde-none/*.out experiment-adv/policy-policy-none/*.out experiment-adv/value-value-none/*.out
  # 3: ./advent.py --scenario 3 experiment-adv/catde-catde-bkls/*.out experiment-adv/policy-policy-bkls/*.out experiment-adv/value-value-bkls/*.out
  # 4: ./advent.py --scenario 4 experiment-adv/catde-catde-bst/*.out experiment-adv/policy-policy-bst/*.out experiment-adv/value-value-bst/*.out
  # 5: ./advent.py --scenario 5 experiment-adv/catde-catde-c200/*.out experiment-adv/policy-policy-c200/*.out experiment-adv/value-value-c200/*.out

  memory_plot = False # scenario is not 0
  unrefinement_plot = False # not memory_plot
  two_sided_plot = memory_plot or unrefinement_plot
  cumulative = True

  if cumulative:
    reward_label = 'Average Return Per Episode'
    val0 = 1
  else:
    reward_label = 'Reward Within an Episode'
    val0 = 4

  if len(filenames) == 0:
    f = open('stdout.txt', 'r')
    seed = int(f.readline().split(' ', 1)[1])
    x = []
    xs = []
    smith = []
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
        memory.append(float(split[7]))
        unrefinements.append(map(int, split[10].split(':')))
    f.close()
    
    directory=''
    title='Blocks World (seed ' + str(seed) + ')'
  else:
    files = {}
    for filename in filenames:
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

    if len(files) == 1:
      title='Blocks World (' + group.rsplit('/',1)[1].replace('_', '\_') + ')'
      smith = files[group].smith
      mode = 'single experiment evaluation'
      
      agent = group.rsplit('/',1)[1].replace('_', '\_')
      remap_names[agent] = agent
      agent_list.append(agent)
      memory[agent] = smith['mem']
      cpu[agent] = smith['cpu']
      unrefinements[agent] = smith['unr']
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
      
      mode = 'multiple experiment evaluation'

  fig = plt.figure()
  fig.canvas.set_window_title('Blocks World')
  
  if two_sided_plot:
    rect = [0.17,0.17,0.70,0.80]
  else:
    rect = [0.19,0.17,0.79,0.80]
  pylab.axes(rect)
  
  labels = []
  if len(filenames) == 0:
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
      remap_names['catde-catde-bkls'] = 'CATDE SBN'
      remap_names['catde-catde-bst'] = 'CATDE SON'
      remap_names['catde-catde-c500'] = 'CATDE SCN'
      remap_names['catde-catde-c200'] = 'CATDE SCN'
      remap_names['catde-catde-c500-even'] = 'CATDE SCN'
      remap_names['catde-catde-none'] = 'CATDE SNN'
      remap_names['catde-none'] = 'CATDE NNN'
      remap_names['policy-policy-bkls'] = 'Policy SBN'
      remap_names['policy-policy-bst'] = 'Policy SON'
      remap_names['policy-policy-c500'] = 'Policy SCN'
      remap_names['policy-policy-c200'] = 'Policy SCN'
      remap_names['policy-policy-c500-even'] = 'Policy SCN'
      remap_names['policy-policy-none'] = 'Policy SNN'
      remap_names['policy-none'] = 'Policy NNN'
      remap_names['value-value-bkls'] = 'Value SBN'
      remap_names['value-value-bst'] = 'Value SON'
      remap_names['value-value-c500'] = 'Value SCN'
      remap_names['value-value-c200'] = 'Value SCN'
      remap_names['value-value-c500-even'] = 'Value SCN'
      remap_names['value-value-none'] = 'Value SNN'
      remap_names['value-none'] = 'Value NNN'
      
      if scenario == 1:
        agent_list = ['catde-none', 'policy-none', 'value-none']
      elif scenario == 2:
        agent_list = ['catde-catde-none', 'policy-policy-none', 'value-value-none']
      elif scenario == 3:
        agent_list = ['catde-catde-bkls', 'policy-policy-bkls', 'value-value-bkls']
      elif scenario == 4:
        agent_list = ['catde-catde-bst', 'policy-policy-bst', 'value-value-bst']
      elif scenario == 5:
        agent_list = ['catde-catde-c200', 'policy-policy-c200', 'value-value-c200']
      if scenario > 0 and scenario < 6:
        for agent in agent_list:
          y_labels.append(remap_names[agent])
          yss.append(smith[agent])
          
          linestyle ='-'
          if agent is 'catde-none' or agent is 'catde-catde-none' or agent is 'catde-catde-bkls' or agent is 'catde-catde-bst' or agent is 'catde-catde-c200':
            color = 'red'
#            linestyle = ':'
          elif agent is 'policy-none' or agent is 'policy-policy-none' or agent is 'policy-policy-bkls' or agent is 'policy-policy-bst' or agent is 'policy-policy-c200':
            color = 'green'
#            linestyle = '--'
          elif agent is 'value-none' or agent is 'value-value-none' or agent is 'value-value-bkls' or agent is 'value-value-bst' or agent is 'value-value-c200':
            color = 'blue'
#            linestyle = '-'
          
          labels += pylab.plot(x, smith[agent], label=remap_names[agent], color=color, linestyle=linestyle)
  
  pylab.grid(False)
  
  pylab.xlabel('Step Number', fontsize=8)
  pylab.ylabel(reward_label, fontsize=8)
  
  #pylab.xlim(xmax=10000)
  if len(filenames) > 1:
    if cumulative:
      pylab.ylim(ymin=-500, ymax=0)
    else:
      pylab.ylim(ymin=-10000, ymax=0)
  
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
      y_labels.append('CPU: ' + remap_names[agent])
      yss.append(cpu[agent])
      #y_labels.append('Mem: ' + remap_names[agent])
      #yss.append(memory[agent])
      
      color = 'red'
      if agent is 'sof4' or agent is 'fof4' or agent is 'cof4' or agent is 'vof4' or agent is 'pof4':
        linestyle = ':'
      elif agent is 'snf4' or agent is 'fnf4' or agent is 'cnf4' or agent is 'vnf4' or agent is 'pnf4':
        linestyle = '--'
      elif agent is 'sonf4' or agent is 'fonf4' or agent is 'conf4' or agent is 'vonf4' or agent is 'ponf4':
        linestyle = '-.'
        
      labels += pylab.plot(x, cpu[agent], label='CPU: ' + remap_names[agent], color=color, linestyle=linestyle)
      #labels += pylab.plot(x, memory[agent], label='Mem: ' + remap_names[agent], color=color, linestyle=linestyle)

      print 'CPU Average for ' + agent + ': ' + str(cpu[agent][-1])
      print 'Memory Average for ' + agent + ': ' + str(memory[agent][-1])
    ax2.set_xlim(0, 5000)
    ax2.set_ylim(0, 2)
    #ax2.set_ylim(0, 100)
    
    #ax2.set_ylabel(r"Temperature ($^\circ$C)")
    ax2.set_ylabel('CPU Time / Step (Milliseconds)')
    #ax2.set_ylabel('Number of Tiles / Weights')
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

    if scenario is 5:
      pylab.legend(labels, [l.get_label() for l in labels], loc=4, handlelength=4.2, numpoints=2, bbox_to_anchor=(-0.05,0.25,1,1))
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
    
    if mode == 'multiple experiment evaluation':
      for agent in agent_list:
        #print '50k Value for ' + agent + ': ' + str(smith[agent][x.index(50000)])
        #print '50k CPU Average for ' + agent + ': ' + str(cpu[agent][x.index(50000)])
        #print '50k Memory Average for ' + agent + ': ' + str(memory[agent][x.index(50000)])
        print 'Final Value for ' + agent + ': ' + str(smith[agent][-1])
        print 'Final CPU Average for ' + agent + ': ' + str(cpu[agent][-1])
        print 'Final Memory Average for ' + agent + ': ' + str(memory[agent][-1])
  
  if len(filenames) == 1:
    write_to_csv('advent.csv', 'Step Number', xs, y_labels, yss)
    pylab.savefig('advent.eps')
    pylab.savefig('advent.png', dpi=1200)
    pylab.savefig('advent.svg')
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
