#include "jni_layer.h"

#include "ch_idsia_ai_agents_ai_JNIAgent.h"
#include "infinite_mario.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace Mario {

  static State g_prev_state;
  static State g_current_state;

  State::State()
   : getMarioMode(MODE_SMALL),
   isMarioOnGround(false),
   mayMarioJump(false),
   isMarioCarrying(false)
  {
  }

  State::State(const State &prev, JNIEnv *env, jobject observation)
   : action(prev.action)
  {
    jclass cls = env->FindClass("ch/idsia/mario/environments/Environment");
    assert(cls);

    jmethodID getter;

    {
      getter = env->GetMethodID(cls, "getCompleteObservation", "()[[B");
      assert(getter);
      jobjectArray completeObservation = jobjectArray(env->CallObjectMethod(observation, getter));
      for(int i = 0; i != OBSERVATION_SIZE; ++i) {
        jbyteArray col = jbyteArray(env->GetObjectArrayElement(completeObservation, i));
        assert(col);
        jbyte *col_data = env->GetByteArrayElements(col, 0);
        assert(col_data);
        memcpy(&getCompleteObservation[i][0], col_data, OBSERVATION_SIZE);
        env->ReleaseByteArrayElements(col, col_data, JNI_ABORT);
      }
    }

    {
      getter = env->GetMethodID(cls, "getMarioFloatPos", "()[F");
      assert(getter);
      jfloatArray pos = jfloatArray(env->CallObjectMethod(observation, getter));
      assert(pos);
      float *pos_data = env->GetFloatArrayElements(pos, 0);
      assert(pos_data);
      getMarioFloatPos.first = pos_data[0];
      getMarioFloatPos.second = pos_data[1];
      env->ReleaseFloatArrayElements(pos, pos_data, JNI_ABORT);

      getter = env->GetMethodID(cls, "getMarioMode", "()I");
      assert(getter);
      getMarioMode = env->CallIntMethod(observation, getter);
    }

    {
      getter = env->GetMethodID(cls, "isMarioOnGround", "()Z");
      assert(getter);
      isMarioOnGround = env->CallBooleanMethod(observation, getter);
      getter = env->GetMethodID(cls, "mayMarioJump", "()Z");
      assert(getter);
      mayMarioJump = env->CallBooleanMethod(observation, getter);
      getter = env->GetMethodID(cls, "isMarioCarrying", "()Z");
      assert(getter);
      isMarioCarrying = env->CallBooleanMethod(observation, getter);
    }
  }

  jbooleanArray State::to_jbooleanArray(JNIEnv *env) const {
    jbooleanArray j_action = env->NewBooleanArray(5);
    assert(j_action);

    jboolean *action = env->GetBooleanArrayElements(j_action, 0);
    assert(action);

    for(int i = 0; i != BUTTON_END; ++i)
      action[i] = g_current_state.action[i];

    env->ReleaseBooleanArrayElements(j_action, action, 0);

    return j_action;
  }

}

/* Implementation of native method sayHello() of HelloJNI class */
JNIEXPORT jbooleanArray JNICALL Java_ch_idsia_ai_agents_ai_JNIAgent_c_1getAction
  (JNIEnv *env, jobject /*obj*/, jobject observation)
{
  Mario::g_current_state = Mario::State(Mario::g_prev_state, env, observation);

  infinite_mario_ai(Mario::g_prev_state, Mario::g_current_state, Mario::g_current_state.action);

  Mario::g_prev_state = Mario::g_current_state;

  return Mario::g_current_state.to_jbooleanArray(env);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM */*vm*/, void */*pvt*/) {
  return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM */*vm*/, void */*pvt*/) {
}
