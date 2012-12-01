#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse, glob, os, random, shutil, subprocess, sys, thread, time
import pp

g_dir = 'experiment'
g_plotter = './puddleworld.py'
g_plotter_grid = ['./puddleworld-grid.py', './puddleworld-heat.py']
g_plotter_grid_filters = ['move(north)', 'move(south)', 'move(east)', 'move(west)', 'all']

g_ep_tuples = []


#g_ep_tuples.append(('puddle-world', 0, 'even',                 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-update-count',     0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-depth',            0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-depth',   0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))


#g_ep_tuples.append(('puddle-world', 0, 'even',                 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 13, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-update-count',     0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 13, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 13, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-depth',            0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 13, 13, 0, 0.5, 0))


#g_ep_tuples.append(('puddle-world', 0, 'even',                 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 7, 7, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-update-count',     0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 7, 7, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 7, 7, 0, 0.5, 0))


#g_ep_tuples.append(('puddle-world', 0, 'even',                 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 99, 1, 0.84155, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-update-count',     0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 99, 1, 0.84155, 0))
#g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 99, 1, 0.84155, 0))


#g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-depth',   0.0, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-depth',   0.2, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-depth',   0.4, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-depth',   0.6, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-depth',   0.8, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
#g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-depth',   1.0, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))

#g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-specific', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, 0))
g_ep_tuples.append(('puddle-world', 0, 'inv-log-update-count', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, -1.0, 0))
g_ep_tuples.append(('puddle-world', 0, 'inv-depth', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, -1.0, 0))
g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-depth', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, -1.0, 0))
g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-specific', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, 0.5, -1.0, 0))
#g_ep_tuples.append(('puddle-world', 0, 'epsilon-even-depth', 0.5, 1.0, 0.1, 0.2, 'off-policy', 20, 5, 13, 0, -1.0, -1.0, 0))


parser = argparse.ArgumentParser(description='Run PuddleWorld experiments.')
parser.add_argument('-j', '--jobs', metavar='N', type=int,
                   action='store',
                   help='number of experiments to run in parallel')
parser.add_argument('-r', '--runs', metavar='N', type=int,
                   action='store', default=1,
                   help='number of runs per experiment')
parser.add_argument('-s', '--steps', metavar='N', type=int,
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
  def __init__(self, num_steps, seed, stderr, stdout, ep_tuple):
    self.num_steps = num_steps
    self.seed = seed
    self.stderr = stderr
    self.stdout = stdout
    self.ep_tuple = ep_tuple
    self.environment = ep_tuple[0]
    self.contribute_update_count = ep_tuple[1]
    self.credit_assignment = ep_tuple[2]
    self.credit_assignment_epsilon = ep_tuple[3]
    self.discount_rate = ep_tuple[4]
    self.epsilon_greedy = ep_tuple[5]
    self.learning_rate = ep_tuple[6]
    self.policy = ep_tuple[7]
    self.pseudoepisode_threshold = ep_tuple[8]
    self.split_min = ep_tuple[9]
    self.split_max = ep_tuple[10]
    self.split_pseudoepisodes = ep_tuple[11]
    self.split_cabe = ep_tuple[12]
    self.split_mabe = ep_tuple[13]
    self.split_update_count = ep_tuple[14]
    
  def get_args(self):
    args = ['./carli',
            '--num-steps', str(self.num_steps),
            '--seed', str(self.seed),
            '--environment', self.environment,
            '--contribute-update-count', str(self.contribute_update_count),
            '--credit-assignment', self.credit_assignment,
            '--credit-assignment-epsilon', str(self.credit_assignment_epsilon),
            '--discount-rate', str(self.discount_rate),
            '--epsilon-greedy', str(self.epsilon_greedy),
            '--learning-rate', str(self.learning_rate),
            '--policy', self.policy,
            '--pseudoepisode-threshold', str(self.pseudoepisode_threshold),
            '--split-min', str(self.split_min),
            '--split-max', str(self.split_max),
            '--split-pseudoepisodes', str(self.split_pseudoepisodes),
            '--split-cabe', str(self.split_cabe),
            '--split-mabe', str(self.split_mabe),
            '--split-update-count', str(self.split_update_count),
            '--output', 'experimental']
    return args
  
  def print_args(self):
    args = self.get_args()
    cmd = ''
    for arg in args:
      cmd += arg + ' '
    cmd += '> ' + self.stdout
    cmd += ' 2> ' + self.stderr
    print cmd
  
  def run(self):
    args = self.get_args()
    f1 = open(self.stdout, 'w')
    f2 = open(self.stderr, 'w')
    subprocess.call(args, stderr=f2, stdout=f1)
    f2.close()
    f1.close()
    return self


dirs = []
experiments = []
for ep_tuple in g_ep_tuples:
  dir = g_dir + '/' #+ ep_tuple[0]
  dir += ep_tuple[2]
  #dir += '_' + str(ep_tuple[1])
  #dir += '_' + ep_tuple[2]
  dir += '_' + str(ep_tuple[3])
  #dir += '_' + str(ep_tuple[4])
  #dir += '_' + str(ep_tuple[5])
  #dir += '_' + str(ep_tuple[6])
  #dir += '_' + ep_tuple[7]
  dir += '_' + str(ep_tuple[8])
  dir += '_' + str(ep_tuple[9])
  dir += '_' + str(ep_tuple[10])
  dir += '_' + str(ep_tuple[11])
  dir += '_' + str(ep_tuple[12])
  dir += '_' + str(ep_tuple[13])
  #dir += '_' + str(ep_tuple[14])
  if not os.path.isdir(dir):
    os.mkdir(dir)
  dirs.append(dir)
  
  for seed in seeds:
    stderr = dir + '/puddleworld-' + str(seed) + '.err'
    stdout = dir + '/puddleworld-' + str(seed) + '.out'
    experiment = Experiment(args.steps, seed, stderr, stdout, ep_tuple)
    experiments.append(experiment)
    a = experiment.get_args()
    s = a[0]
    for ss in a[1:]:
      s += ' ' + ss
    print s

class Progress:
  def __init__(self, experiments):
    self.lock = thread.allocate_lock()
    
    self.count = {}
    self.finished = {}
    for experiment in experiments:
      try:
        self.count[experiment.ep_tuple] += 1
      except KeyError:
        self.count[experiment.ep_tuple] = 1
      self.finished[experiment.ep_tuple] = 0

  def just_finished(self, experiment):
    self.lock.acquire()
    self.finished[experiment.ep_tuple] += 1
    self.lock.release()

  def all_finished(self, ep_tuple):
    self.lock.acquire()
    num = self.count[ep_tuple]
    fin = self.finished[ep_tuple]
    self.lock.release()
    return fin is num

job_server = pp.Server(args.jobs)
progress = Progress(experiments)
start_time = time.time()
jobs = [(job_server.submit(Experiment.run, (experiment,), (), ('subprocess', 'thread',), callback=progress.just_finished, group=experiment.ep_tuple)) for experiment in experiments]

for ep_tuple, dir in zip(g_ep_tuples, dirs):
  while True:
    job_server.print_stats()
    if progress.all_finished(ep_tuple):
      break
    else:
      time.sleep(5)
  job_server.wait(ep_tuple)
  args = [g_plotter] + glob.glob(dir + '/*.out')
  print 'Plotting data for ' + str(ep_tuple) + '\n'
  subprocess.call(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
  for plotter in g_plotter_grid:
    for filter in g_plotter_grid_filters:
      args = [plotter, filter] + glob.glob(dir + '/*.err')
      print 'Plotting ' + plotter + ' data for ' + str(ep_tuple) + ', ' + filter + '\n'
      subprocess.call(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)

print 'Total time elapsed: ', time.time() - start_time, 'seconds'