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

  /// Initialize state of Car
  void Environment::MCarInit() {
    if(m_random_start) {
      m_x = float(m_random_init.frand_lt() * (mcar_goal_position - mcar_min_position) + mcar_min_position);
      m_x_dot = float(m_random_init.frand_lt() * (2 * mcar_max_velocity) - mcar_max_velocity);
    }
    else {
      m_x = -0.5;
      m_x_dot = 0.0;
    }
  }

  /// Take action a, update state of car
  void Environment::MCarStep(int a) {
    assert(0 <= a && a <= 2);

    m_x_dot += float((a - 1) * m_cart_force + cos(3 * m_x) * -m_grav_force);
    if(m_x_dot > mcar_max_velocity)
      m_x_dot = float(mcar_max_velocity);
    if(m_x_dot < -mcar_max_velocity)
      m_x_dot = float(-mcar_max_velocity);

    m_x += float(m_x_dot);
    if(m_x > mcar_max_position)
      m_x = float(mcar_max_position);
    if(m_x < mcar_min_position)
      m_x = float(mcar_min_position);

    if(m_x == mcar_min_position && m_x_dot < 0)
      m_x_dot = 0;
  }

  /// Is Car within goal region?
  bool Environment::MCarAtGoal() const {
    return m_x >= mcar_goal_position;
  }

}
