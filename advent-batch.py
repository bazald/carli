#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse, glob, os, random, shutil, subprocess, sys, thread, time
import pp

g_dir = 'experiment-adv'
g_plotters = ['./advent.py']
g_error = False

g_base_command = "./advent --output experiment --print-every 1000 --epsilon-greedy 0.1 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.1 --secondary-learning-rate 0.1 --policy off-policy --split-update-count 20 --unsplit-update-count 100"

g_ep_tuples = []

g_ep_tuples.append(("catde-none",          "--split-test catde  --unsplit-test none                                                         --rules rules/advent.carli"))
#g_ep_tuples.append(("catde-catde-none",    "--split-test catde  --unsplit-test catde  --resplit-bias none                                   --rules rules/advent.carli"))
#g_ep_tuples.append(("catde-catde-bkls",    "--split-test catde  --unsplit-test catde  --resplit-bias blacklist                              --rules rules/advent.carli"))
#g_ep_tuples.append(("catde-catde-bst",     "--split-test catde  --unsplit-test catde  --resplit-bias boost                                  --rules rules/advent.carli"))
#g_ep_tuples.append(("catde-catde-c500",    "--split-test catde  --unsplit-test catde  --resplit-bias boost     --concrete-update-count 500  --rules rules/advent.carli"))
#g_ep_tuples.append(("catde-catde-c300",   "--split-test catde  --unsplit-test catde  --resplit-bias boost     --concrete-update-count 300 --rules rules/advent.carli"))

g_ep_tuples.append(("value-none",          "--split-test value  --unsplit-test none                                                         --rules rules/advent.carli"))
#g_ep_tuples.append(("value-value-none",    "--split-test value  --unsplit-test value  --resplit-bias none                                   --rules rules/advent.carli"))
#g_ep_tuples.append(("value-value-bkls",    "--split-test value  --unsplit-test value  --resplit-bias blacklist                              --rules rules/advent.carli"))
#g_ep_tuples.append(("value-value-bst",     "--split-test value  --unsplit-test value  --resplit-bias boost                                  --rules rules/advent.carli"))
#g_ep_tuples.append(("value-value-c500",    "--split-test value  --unsplit-test value  --resplit-bias boost     --concrete-update-count 500  --rules rules/advent.carli"))
#g_ep_tuples.append(("value-value-c300",   "--split-test value  --unsplit-test value  --resplit-bias boost     --concrete-update-count 300 --rules rules/advent.carli"))

g_ep_tuples.append(("policy-none",         "--split-test policy --unsplit-test none                                                         --rules rules/advent.carli"))
#g_ep_tuples.append(("policy-policy-none",  "--split-test policy --unsplit-test policy --resplit-bias none                                   --rules rules/advent.carli"))
#g_ep_tuples.append(("policy-policy-bkls",  "--split-test policy --unsplit-test policy --resplit-bias blacklist                              --rules rules/advent.carli"))
#g_ep_tuples.append(("policy-policy-bst",   "--split-test policy --unsplit-test policy --resplit-bias boost                                  --rules rules/advent.carli"))
#g_ep_tuples.append(("policy-policy-c500",  "--split-test policy --unsplit-test policy --resplit-bias boost     --concrete-update-count 500  --rules rules/advent.carli"))
#g_ep_tuples.append(("policy-policy-c300", "--split-test policy --unsplit-test policy --resplit-bias boost     --concrete-update-count 300 --rules rules/advent.carli"))

parser = argparse.ArgumentParser(description='Run Advent experiments.')
parser.add_argument('-j', '--jobs', metavar='N', type=int,
                   action='store',
                   help='number of experiments to run in parallel')
parser.add_argument('-r', '--runs', metavar='N', type=int,
                   action='store', default=1,
                   help='number of runs per experiment')
parser.add_argument('-n', '--num-steps', metavar='N', type=int,
                   action='store', default=2000000,
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
  def __init__(self, num_steps, seed, stderr, stdout, rules, experiment, vfm):
    self.num_steps = num_steps
    self.seed = seed
    self.stderr = stderr
    self.stdout = stdout
    self.rules = rules
    self.experiment = experiment
    self.vfm = vfm
    self.errorcode = 0

  def get_args(self):
    args = self.experiment.split()
    args.extend(['--num-steps', str(self.num_steps),
                 '--seed', str(self.seed),
                 '--stderr', self.stderr,
                 '--stdout', self.stdout,
                 '--rules-out', self.rules])
    if self.vfm:
      args.extend(['--value-function-map-filename', self.vfm])
    return args

  def print_args(self):
    args = self.get_args()
    cmd = args[0]
    for arg in args[1:]:
      cmd += ' ' + arg
    return cmd

  def run(self):
    args = self.get_args()
    #print self.print_args()
    try:
      subprocess.check_call(args, stdin=None, stdout=None, stderr=None)
    except subprocess.CalledProcessError, e:
      g_error = True
      print "Called Process Error:", e.cmd, "=", e.returncode, ":", e.output
      self.errorcode = e.returncode
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
    stderr = dir + '/advent-' + str(seed) + '.err'
    stdout = dir + '/advent-' + str(seed) + '.out'
    rules = dir + '/advent-' + str(seed) + '.carli'
    vfm = dir + '/advent-' + str(seed) + '.vfm'
    experiment = Experiment(args.num_steps, seed, stderr, stdout, rules, g_base_command + ' ' + ep_tuple[1], vfm=vfm)
    g_experiments.append(experiment)
    print experiment.print_args()

class Progress:
  def __init__(self, experiments):
    self.lock = thread.allocate_lock()

    self.count = {}
    self.finished = {}
    for experiment in experiments:
      try:
        self.count[experiment.experiment] += 1
        #print "Updated count of " + experiment.experiment + " = " + str(self.count[experiment.experiment])
      except KeyError:
        self.count[experiment.experiment] = 1
        #print "New count of " + experiment.experiment + " = " + str(self.count[experiment.experiment])
      self.finished[experiment.experiment] = 0

  def just_finished(self, experiment):
    global g_error
    self.lock.acquire()
    if experiment.errorcode:
      g_error = True
    else:
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
    #print 'Finished ' + str(fin) + ' of ' + str(num)
    self.lock.release()
    return fin >= num

class Plots:
  def __init__(self):
    self.experiment = 'plots'

plots = []
for i in range(len(g_ep_tuples) * len(g_plotters)):
  plots.append(Plots())

def syscall(args):
  subprocess.check_call(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)

def take_fives(group):
  while not g_error:
    job_server.print_stats()
    #print "Group = " + group
    if progress.all_finished(group):
      break
    else:
      time.sleep(5)
  if not g_error:
    job_server.wait(group)

job_server = pp.Server(args.jobs)
progress = Progress(g_experiments + plots)
start_time = time.time()
#for experiment in g_experiments:
  #experiment.run()
jobs = [(job_server.submit(Experiment.run, (experiment,), (), ('subprocess', 'thread'), callback=progress.just_finished, group=experiment.experiment)) for experiment in g_experiments]

for ep_tuple, dir in zip(g_ep_tuples, g_dirs):
  take_fives(g_base_command + ' ' + ep_tuple[1])
  for plotter in g_plotters:
    args = [plotter] + glob.glob(dir + '/*.out')
    jobs.append(job_server.submit(syscall, (args,), (), ('subprocess', 'thread',), callback=progress.just_finished_plot, group='plots'))

take_fives('plots')

print 'Total time elapsed: ', time.time() - start_time, 'seconds'
