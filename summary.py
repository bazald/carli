#!/usr/bin/env python
# -*- coding: utf-8 -*-

import hashlib, os, re
import sys, getopt, random, time, datetime
from optparse import OptionParser

class Handle:
  def __init__(self, f, filename, seed):
    self.f = f
    self.filename = filename
    self.seed = seed

class Handles:
  def __init__(self):
    self.handles = []
    self.avg_cpu_time = 0.0
    self.avg_regret = 0.0

def main():
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
  
  for group in files:
    done = False
    while not done:
      avg_cpu_time = 0.0
      avg_regret = 0.0
      for handle in files[group].handles:
        line = handle.f.readline()
        if not line or line == '':
          done = True
          break
        else:
          split = line.split(' ')
          avg_regret += float(split[2])
          avg_cpu_time += float(split[8])
      if done:
        seconds = files[group].avg_cpu_time / len(files[group].handles)
        minutes = int(seconds / 60)
        seconds = seconds - 60 * minutes
        print "Average Return Per Episode of " + group.split('/', 1)[1] + ": " + str(files[group].avg_regret / len(files[group].handles)) + " in " + str(minutes) + ":" + "{0:.1f}".format(seconds)
      else:
        files[group].avg_cpu_time = avg_cpu_time
        files[group].avg_regret = avg_regret
    
    for handle in files[group].handles:
      handle.f.close()

if __name__ == "__main__":
  main()
