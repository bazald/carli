#include "infinite_mario.h"

#include "jni_layer.h"

namespace Mario {

  void infinite_mario_ai(const State &prev, const State &current, Action &action) {
    action[BUTTON_LEFT] = 0;
    action[BUTTON_RIGHT] = 1;
    action[BUTTON_DOWN] = 0;

  //   for(int i = 0; i != OBSERVATION_SIZE; ++i) {
  //     for(int j = 0; j != OBSERVATION_SIZE; ++j) {
  //       if(current.getCompleteObservation[i][j] == SCENE_OBJECT_RED_KOOPA_WINGED ||
  //          current.getCompleteObservation[i][j] == SCENE_OBJECT_GREEN_KOOPA_WINGED ||
  //          current.getCompleteObservation[i][j] == SCENE_OBJECT_SPIKY_WINGED
  //       ) {
  //         action[BUTTON_LEFT] = 1;
  //         action[BUTTON_RIGHT] = 0;
  //         action[BUTTON_DOWN] = 0;
  //       }
  //     }
  //   }

    if(prev.action[BUTTON_JUMP])
      action[BUTTON_JUMP] = !current.isMarioOnGround;
    else
      action[BUTTON_JUMP] = current.mayMarioJump;

    if(current.getMarioMode == 2)
      action[BUTTON_SPEED] = !prev.action[BUTTON_SPEED];
    else if(prev.action[BUTTON_SPEED])
      action[BUTTON_SPEED] = !current.isMarioCarrying;
    else
      action[BUTTON_SPEED] = 1;
  }

}
