#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse, glob, os, random, shutil, subprocess, sys, thread, time
import pp

g_dir = 'experiment-bw'
g_plotters = ['./blocksworld.py', './memory.py']
g_plotter_grid = []#['./blocksworld-grid.py', './blocksworld-heat.py']
g_plotter_grid_filters = ['move(north)', 'move(south)', 'move(east)', 'move(west)', 'all']

g_base_command = "./blocks_world --output experiment --epsilon-greedy 0.1 --policy off-policy"

g_ep_tuples = []

### Experiment 0
g_ep_tuples.append(("catde_normal", "--split-test catde --rules rules/blocks-world.carli"))
#g_ep_tuples.append(("catde_extra_block", "--split-test catde --rules rules/blocks-world-4-blocks.carli --epsilon-greedy 0.01"))
#g_ep_tuples.append(("catde_irrelevant_features", "--split-test catde --rules rules/blocks-world-irrelevant.carli"))
#g_ep_tuples.append(("catde_random_features", "--split-test catde --rules rules/blocks-world-random.carli"))

#g_ep_tuples.append(("value_normal", "--split-test value --rules rules/blocks-world.carli"))
#g_ep_tuples.append(("value_extra_block", "--split-test value --rules rules/blocks-world-4-blocks.carli"))
#g_ep_tuples.append(("value_irrelevant_features", "--split-test value --rules rules/blocks-world-irrelevant.carli"))
#g_ep_tuples.append(("value_random_features", "--split-test value --rules rules/blocks-world-random.carli"))

parser = argparse.ArgumentParser(description='Run Blocks World experiments.')
parser.add_argument('-j', '--jobs', metavar='N', type=int,
                   action='store',
                   help='number of experiments to run in parallel')
parser.add_argument('-r', '--runs', metavar='N', type=int,
                   action='store', default=1,
                   help='number of runs per experiment')
parser.add_argument('-n', '--num-steps', metavar='N', type=int,
                   action='store', default=50000,
                   help='number of steps per run')

args = parser.parse_args()

if args.jobs is None:
  args.jobs = 'autodetect'


if not os.path.isdir(g_dir):
  os.mkdir(g_dir)
seeds = []
seeds_file = g_dir + '/seeds'
if os.path.isfile(seeds_file):
  f = open(seeds_file, 'r')
  for seed in f:
    seeds.append(int(seed))
  f.close()
if len(seeds) != args.runs:
  if len(seeds) > 0:
    raise Exception('Number of seeds differs from number of runs.')
  else:
    seeds = []
    for i in range(0, args.runs):
      seeds.append(random.randint(0,65535))
    f = open(seeds_file, 'w')
    for seed in seeds:
      f.write(str(seed) + '\n')
    f.close()
print str(seeds) + '\n'


class Experiment:
  def __init__(self, num_steps, seed, stderr, stdout, experiment, vfm):
    self.num_steps = num_steps
    self.seed = seed
    self.stderr = stderr
    self.stdout = stdout
    self.experiment = experiment
    self.vfm = vfm
    
  def get_args(self):
    args = self.experiment.split()
    args.extend(['--num-steps', str(self.num_steps),
                 '--seed', str(self.seed),
                 '--stderr', self.stderr,
                 '--stdout', self.stdout])
    if self.vfm:
      args.extend(['--value-function-map-filename', self.vfm])
    return args
  
  def print_args(self):
    args = self.get_args()
    cmd = args[0]
    for arg in args[1:]:
      cmd += ' ' + arg
    print cmd
  
  def run(self):
    args = self.get_args()
    subprocess.call(args)
    return self

def resolution(split):
  depth = (split - 1) / 2
  size = str(pow(2, depth))
  return size + 'x' + size

g_dirs = []
g_experiments = []
for ep_tuple in g_ep_tuples:
  dir = g_dir + '/' + ep_tuple[0]
  if not os.path.isdir(dir):
    os.mkdir(dir)
  g_dirs.append(dir)
  
  for seed in seeds:
    stderr = dir + '/blocksworld-' + str(seed) + '.err'
    stdout = dir + '/blocksworld-' + str(seed) + '.out'
    vfm = dir + '/blocksworld-' + str(seed) + '.vfm'
    experiment = Experiment(args.num_steps, seed, stderr, stdout, g_base_command + ' ' + ep_tuple[1], vfm=vfm)
    g_experiments.append(experiment)
    experiment.print_args()

class Progress:
  def __init__(self, experiments):
    self.lock = thread.allocate_lock()
    
    self.count = {}
    self.finished = {}
    for experiment in experiments:
      try:
        self.count[experiment.experiment] += 1
      except KeyError:
        self.count[experiment.experiment] = 1
      self.finished[experiment.experiment] = 0

  def just_finished(self, experiment):
    self.lock.acquire()
    self.finished[experiment.experiment] += 1
    self.lock.release()

  def just_finished_plot(self, args):
    self.lock.acquire()
    self.finished['plots'] += 1
    self.lock.release()

  def all_finished(self, experiment):
    self.lock.acquire()
    num = self.count[experiment]
    fin = self.finished[experiment]
    self.lock.release()
    return fin is num

class Plots:
  def __init__(self):
    self.experiment = 'plots'

plots = []
for i in range(len(g_ep_tuples) * (len(g_plotters) + len(g_plotter_grid) * len(g_plotter_grid_filters))):
  plots.append(Plots())

def syscall(args):
  subprocess.call(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)

def take_fives(group):
  while True:
    job_server.print_stats()
    if progress.all_finished(group):
      break
    else:
      time.sleep(5)
  job_server.wait(group)

job_server = pp.Server(args.jobs)
progress = Progress(g_experiments + plots)
start_time = time.time()
#for experiment in g_experiments:
  #experiment.run()
jobs = [(job_server.submit(Experiment.run, (experiment,), (), ('subprocess', 'thread',), callback=progress.just_finished, group=experiment.experiment)) for experiment in g_experiments]

for ep_tuple, dir in zip(g_ep_tuples, g_dirs):
  take_fives(g_base_command + ' ' + ep_tuple[1])
  for plotter in g_plotters:
    args = [plotter] + glob.glob(dir + '/*.out')
    jobs.append(job_server.submit(syscall, (args,), (), ('subprocess', 'thread',), callback=progress.just_finished_plot, group='plots'))
  for plotter in g_plotter_grid:
    for filter in g_plotter_grid_filters:
      args = [plotter, filter] + glob.glob(dir + '/*.err')
      print 'Plotting ' + plotter + ' data for ' + ep_tuple[1] + ', ' + filter + '\n'
      jobs.append(job_server.submit(syscall, (args,), (), ('subprocess', 'thread',), callback=progress.just_finished_plot, group='plots'))

take_fives('plots')

print 'Total time elapsed: ', time.time() - start_time, 'seconds'
