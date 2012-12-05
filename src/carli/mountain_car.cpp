/*
This is an example program for reinforcement learning with linear 
function approximation.  The code follows the psuedo-code for linear, 
gradient-descent Sarsa(lambda) given in Figure 8.8 of the book 
"Reinforcement Learning: An Introduction", by Sutton and Barto.
One difference is that we use the implementation trick mentioned on 
page 189 to only keep track of the traces that are larger 
than "min-trace". 

Before running the program you need to obtain the tile-coding 
software, available at http://envy.cs.umass.edu/~rich/tiles.C and tiles.h
(see http://envy.cs.umass.edu/~rich/tiles.html for documentation).

The code below is in three main parts: 1) Mountain Car code, 2) General 
RL code, and 3) top-level code and misc.

Written by Rich Sutton 12/19/00
 */

#include "mountain_car.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

using namespace std;

namespace Mountain_Car {

  //////////     Part 1: Mountain Car code     //////////////

  static const double mcar_min_position = -1.2;
  static const double mcar_max_position = 0.6;
  static const double mcar_max_velocity = 0.07;            // the negative of this in the minimum velocity
  static const double mcar_goal_position = 0.5;

  void Environment::MCarInit(float &position, float &velocity)
  // Initialize state of Car
    { position = -0.5;
      velocity = 0.0;}

  void Environment::MCarStep(float &position, float &velocity, int a)
  // Take action a, update state of car
    { assert(0 <= a && a <= 2);
      velocity += (a-1)*0.001 + cos(3*position)*(-0.0025);
      if (velocity > mcar_max_velocity) velocity = mcar_max_velocity;
      if (velocity < -mcar_max_velocity) velocity = -mcar_max_velocity;
      position += velocity;
      if (position > mcar_max_position) position = mcar_max_position;
      if (position < mcar_min_position) position = mcar_min_position;
      if (position==mcar_min_position && velocity<0) velocity = 0;}

  bool Environment::MCarAtGoal(const float &position, const float &velocity)
  // Is Car within goal region?
    { return position >= mcar_goal_position;}

}
