#include <jni.h>
#include <stdio.h>

#include "ch_idsia_ai_agents_ai_JNIAgent.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

enum {
  OBSERVATION_SIZE = 22
};

enum MARIO_BUTTON {
  MARIO_BUTTON_LEFT  = 0,
  MARIO_BUTTON_RIGHT = 1,
  MARIO_BUTTON_DOWN  = 2,
  MARIO_BUTTON_JUMP  = 3,
  MARIO_BUTTON_SPEED = 4,
  MARIO_BUTTON_END   = 5
};

enum MARIO_MODE {
  MARIO_MODE_SMALL = 0,
  MARIO_MODE_BIG   = 1,
  MARIO_MODE_FIERY = 2,
};

enum MARIO_SCENE {
  MARIO_SCENE_TILE_IRRELEVANT           =   0,
  MARIO_SCENE_TILE_SOMETHING            =   1,
  MARIO_SCENE_TILE_BORDER               = -10,
  MARIO_SCENE_TILE_HALF_BORDER          = -11, ///< Platforms can be jumped up through
  MARIO_SCENE_TILE_BRICK                =  16,
  MARIO_SCENE_TILE_POT_OR_CANNON        =  20,
  MARIO_SCENE_TILE_QUESTION             =  21,
  MARIO_SCENE_OBJECT_GOOMBA             =   2,
  MARIO_SCENE_OBJECT_GOOMBA_WINGED      =   3,
  MARIO_SCENE_OBJECT_RED_KOOPA          =   4,
  MARIO_SCENE_OBJECT_RED_KOOPA_WINGED   =   5,
  MARIO_SCENE_OBJECT_GREEN_KOOPA        =   6,
  MARIO_SCENE_OBJECT_GREEN_KOOPA_WINGED =   7,
  MARIO_SCENE_OBJECT_BULLET_BILL        =   8,
  MARIO_SCENE_OBJECT_SPIKY              =   9,
  MARIO_SCENE_OBJECT_SPIKY_WINGED       =  10,
  MARIO_SCENE_OBJECT_ENEMY_FLOWER       =  12,
  MARIO_SCENE_OBJECT_SHELL              =  13,
  MARIO_SCENE_OBJECT_MUSHROOM           =  14,
  MARIO_SCENE_OBJECT_FIRE_FLOWER        =  15,
  MARIO_SCENE_OBJECT_FIREBALL           =  25
};

typedef struct {
  char getCompleteObservation[OBSERVATION_SIZE][OBSERVATION_SIZE];

  float getMarioFloatPos[2];
  int getMarioMode;

  char isMarioOnGround;
  char mayMarioJump;
  char isMarioCarrying;

  char action[MARIO_BUTTON_END];
} Mario_State;

static Mario_State empty_Mario_State() {
  Mario_State ms;
  memset(&ms, 0, sizeof(ms));
  return ms;
}

static Mario_State g_prev_state;

static Mario_State current_Mario_State(JNIEnv *env, jobject observation) {
  Mario_State ms = empty_Mario_State();

  jclass cls = (*env)->FindClass(env, "ch/idsia/mario/environments/Environment");
  assert(cls);

  jmethodID getter;

  {
    getter = (*env)->GetMethodID(env, cls, "getCompleteObservation", "()[[B");
    assert(getter);
    jobjectArray completeObservation = (jobjectArray)(*env)->CallObjectMethod(env, observation, getter);
    for(int i = 0; i != OBSERVATION_SIZE; ++i) {
      jbyteArray col = (jbyteArray)(*env)->GetObjectArrayElement(env, completeObservation, i);
      assert(col);
      jbyte *col_data = (*env)->GetByteArrayElements(env, col, 0);
      assert(col_data);
      memcpy(ms.getCompleteObservation[i], col_data, OBSERVATION_SIZE);
      (*env)->ReleaseByteArrayElements(env, col, col_data, JNI_ABORT);
    }
  }

  {
    getter = (*env)->GetMethodID(env, cls, "getMarioFloatPos", "()[F");
    assert(getter);
    jfloatArray pos = (jfloatArray)(*env)->CallObjectMethod(env, observation, getter);
    assert(pos);
    float *pos_data = (float *)(*env)->GetFloatArrayElements(env, pos, 0);
    assert(pos_data);
    memcpy(ms.getMarioFloatPos, pos_data, 2 * sizeof(float));
    (*env)->ReleaseFloatArrayElements(env, pos, pos_data, JNI_ABORT);

    getter = (*env)->GetMethodID(env, cls, "getMarioMode", "()I");
    assert(getter);
    ms.getMarioMode = (*env)->CallIntMethod(env, observation, getter);
  }

  {
    getter = (*env)->GetMethodID(env, cls, "isMarioOnGround", "()Z");
    assert(getter);
    ms.isMarioOnGround = (*env)->CallBooleanMethod(env, observation, getter);
    getter = (*env)->GetMethodID(env, cls, "mayMarioJump", "()Z");
    assert(getter);
    ms.mayMarioJump = (*env)->CallBooleanMethod(env, observation, getter);
    getter = (*env)->GetMethodID(env, cls, "isMarioCarrying", "()Z");
    assert(getter);
    ms.isMarioCarrying = (*env)->CallBooleanMethod(env, observation, getter);
  }

  memcpy(ms.action, g_prev_state.action, sizeof(ms.action));

  return ms;
}

static jbooleanArray action_from_Mario_State(JNIEnv *env, Mario_State ms) {
  jbooleanArray j_action = (*env)->NewBooleanArray(env, 5);
  assert(j_action);

  jboolean *action = (*env)->GetBooleanArrayElements(env, j_action, 0);
  assert(action);

  for(int i = 0; i != MARIO_BUTTON_END; ++i)
    action[i] = ms.action[i];

  (*env)->ReleaseBooleanArrayElements(env, j_action, action, 0);

  return j_action;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *pvt) {
  g_prev_state = empty_Mario_State();
  return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *pvt) {
}

/* Implementation of native method sayHello() of HelloJNI class */
JNIEXPORT jbooleanArray JNICALL Java_ch_idsia_ai_agents_ai_JNIAgent_c_1getAction
  (JNIEnv *env, jobject obj, jobject observation)
{
  Mario_State current_state = current_Mario_State(env, observation);

  current_state.action[MARIO_BUTTON_LEFT] = 0;
  current_state.action[MARIO_BUTTON_RIGHT] = 1;
  current_state.action[MARIO_BUTTON_DOWN] = 0;

//   for(int i = 0; i != OBSERVATION_SIZE; ++i) {
//     for(int j = 0; j != OBSERVATION_SIZE; ++j) {
//       if(current_state.getCompleteObservation[i][j] == MARIO_SCENE_OBJECT_RED_KOOPA_WINGED ||
//          current_state.getCompleteObservation[i][j] == MARIO_SCENE_OBJECT_GREEN_KOOPA_WINGED ||
//          current_state.getCompleteObservation[i][j] == MARIO_SCENE_OBJECT_SPIKY_WINGED
//       ) {
//         current_state.action[MARIO_BUTTON_LEFT] = 1;
//         current_state.action[MARIO_BUTTON_RIGHT] = 0;
//         current_state.action[MARIO_BUTTON_DOWN] = 0;
//       }
//     }
//   }

  if(g_prev_state.action[MARIO_BUTTON_JUMP])
    current_state.action[MARIO_BUTTON_JUMP] = !current_state.isMarioOnGround;
  else
    current_state.action[MARIO_BUTTON_JUMP] = current_state.mayMarioJump;

  if(current_state.getMarioMode == 2)
    current_state.action[MARIO_BUTTON_SPEED] = !g_prev_state.action[MARIO_BUTTON_SPEED];
  else if(g_prev_state.action[MARIO_BUTTON_SPEED])
    current_state.action[MARIO_BUTTON_SPEED] = !current_state.isMarioCarrying;
  else
    current_state.action[MARIO_BUTTON_SPEED] = 1;

  g_prev_state = current_state;

  return action_from_Mario_State(env, current_state);
}
