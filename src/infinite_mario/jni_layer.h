#ifndef INFINITE_MARIO_JNI_LAYER_H
#define INFINITE_MARIO_JNI_LAYER_H

#include <array>
#include <jni.h>

namespace Mario {

  enum {
    OBSERVATION_SIZE = 22
  };

  enum Button {
    BUTTON_LEFT  = 0,
    BUTTON_RIGHT = 1,
    BUTTON_DOWN  = 2,
    BUTTON_JUMP  = 3,
    BUTTON_SPEED = 4,
    BUTTON_END   = 5
  };

  enum Mode {
    MODE_SMALL = 0,
    MODE_BIG   = 1,
    MODE_FIERY = 2
  };

  enum Scene {
    SCENE_TILE_IRRELEVANT           =   0,
    SCENE_TILE_SOMETHING            =   1,
    SCENE_TILE_BORDER               = -10,
    SCENE_TILE_HALF_BORDER          = -11, /* Platforms which can be jumped up through */
    SCENE_TILE_BRICK                =  16,
    SCENE_TILE_POT_OR_CANNON        =  20,
    SCENE_TILE_QUESTION             =  21,
    SCENE_OBJECT_GOOMBA             =   2,
    SCENE_OBJECT_GOOMBA_WINGED      =   3,
    SCENE_OBJECT_RED_KOOPA          =   4,
    SCENE_OBJECT_RED_KOOPA_WINGED   =   5,
    SCENE_OBJECT_GREEN_KOOPA        =   6,
    SCENE_OBJECT_GREEN_KOOPA_WINGED =   7,
    SCENE_OBJECT_BULLET_BILL        =   8,
    SCENE_OBJECT_SPIKY              =   9,
    SCENE_OBJECT_SPIKY_WINGED       =  10,
    SCENE_OBJECT_ENEMY_FLOWER       =  12,
    SCENE_OBJECT_SHELL              =  13,
    SCENE_OBJECT_MUSHROOM           =  14,
    SCENE_OBJECT_FIRE_FLOWER        =  15,
    SCENE_OBJECT_FIREBALL           =  25
  };

  typedef std::array<bool, 5> Action;

  struct State {
    State();
    State(const State &prev, JNIEnv *env, jobject observation);

    jbooleanArray to_jbooleanArray(JNIEnv *env) const;

    std::array<std::array<char, OBSERVATION_SIZE>, OBSERVATION_SIZE> getCompleteObservation;

    std::pair<float, float> getMarioFloatPos;
    int getMarioMode;

    bool isMarioOnGround;
    bool mayMarioJump;
    bool isMarioCarrying;

    Action action;
  };

}

#endif
