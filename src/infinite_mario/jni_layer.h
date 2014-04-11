#ifndef INFINITE_MARIO_JNI_LAYER_H
#define INFINITE_MARIO_JNI_LAYER_H

#include "carli/environment.h"

#include <array>
#include <inttypes.h>
#include <jni.h>

namespace Mario {

  enum {
    OBSERVATION_SIZE = 22
  };

  enum Button {
    BUTTON_NONE  = -1,
    BUTTON_LEFT  =  0,
    BUTTON_RIGHT =  1,
    BUTTON_DOWN  =  2,
    BUTTON_JUMP  =  3,
    BUTTON_SPEED =  4,
    BUTTON_END   =  5
  };

  enum Mode {
    MODE_SMALL = 0,
    MODE_BIG   = 1,
    MODE_FIERY = 2
  };

  enum Tile {
    TILE_IRRELEVANT           =   0,
    TILE_SOMETHING            =   1,
    TILE_BORDER               = -10,
    TILE_HALF_BORDER          = -11, /* Platforms which can be jumped up through */
    TILE_BRICK                =  16,
    TILE_POT_OR_CANNON        =  20,
    TILE_QUESTION             =  21
  };

  enum Object {
    OBJECT_IRRELEVANT         =   0,
    OBJECT_GOOMBA             =   2,
    OBJECT_GOOMBA_WINGED      =   3,
    OBJECT_RED_KOOPA          =   4,
    OBJECT_RED_KOOPA_WINGED   =   5,
    OBJECT_GREEN_KOOPA        =   6,
    OBJECT_GREEN_KOOPA_WINGED =   7,
    OBJECT_BULLET_BILL        =   8,
    OBJECT_SPIKY              =   9,
    OBJECT_SPIKY_WINGED       =  10,
    OBJECT_ENEMY_FLOWER       =  12,
    OBJECT_SHELL              =  13,
    OBJECT_MUSHROOM           =  14,
    OBJECT_FIRE_FLOWER        =  15,
    OBJECT_FIREBALL           =  25
  };

  typedef std::array<bool, 5> Action;

  class State : public Carli::Environment {
  public:
    State();
    State(const State &prev, JNIEnv *env, jobject observation);

    jbooleanArray to_jbooleanArray(JNIEnv *env) const;
    
    void init_impl() {}

    reward_type transition_impl(const Carli::Action &) {
      abort();
    }

    void print_impl(std::ostream &os) const {
      os << "infinite-mario(" << getMarioFloatPos.first << ',' << getMarioFloatPos.second
         << ';' << getMarioMode
         << ';' << isMarioOnGround << ',' << mayMarioJump << ',' << isMarioCarrying
         << ':';
      for(const auto &ac : action)
        os << ac;
      os << ')';
    }
    
    std::array<std::array<Tile, OBSERVATION_SIZE>, OBSERVATION_SIZE> getLevelSceneObservation;
    std::array<std::array<Object, OBSERVATION_SIZE>, OBSERVATION_SIZE> getEnemiesObservation;

    std::vector<std::pair<Object, std::pair<float, float>>> getEnemiesFloatPos;

    std::pair<float, float> getMarioFloatPos;
    int64_t getMarioMode = MODE_SMALL;

    bool isMarioOnGround = false;
    bool mayMarioJump = false;
    bool isMarioCarrying = false;
    
    int getKillsTotal = 0;
    int getKillsByFire = 0;
    int getKillsByStomp = 0;
    int getKillsByShell = 0;

    Action action = {{false, false, false, false, false}} ;
  };

}

std::ostream & operator<<(std::ostream &os, const Mario::Tile &tile);
std::ostream & operator<<(std::ostream &os, const Mario::Object &object);

#endif
