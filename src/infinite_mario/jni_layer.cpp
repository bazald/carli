#include "jni_layer.h"

#include "carli/experiment.h"
#include "ch_idsia_ai_agents_ai_JNIAgent.h"
#include "infinite_mario.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace Mario {

  static std::shared_ptr<State> g_prev_state;
  static std::shared_ptr<State> g_current_state;

  State::State()
    : action({{false, false, false, false, false}})
  {
  }

  State::State(const State &prev, JNIEnv *env, jobject observation)
   : enemiesSeen(prev.enemiesSeen),
   action(prev.action)
  {
    jclass cls = env->FindClass("ch/idsia/mario/environments/Environment");
    assert(cls);

    jmethodID getter;

    {
      getter = env->GetMethodID(cls, "getLevelSceneObservation", "()[[B");
      assert(getter);
      jobjectArray levelSceneObservation = jobjectArray(env->CallObjectMethod(observation, getter));
      for(int j = 0; j != OBSERVATION_HEIGHT; ++j) {
        jbyteArray row = jbyteArray(env->GetObjectArrayElement(levelSceneObservation, j));
        assert(row);
        jbyte *row_data = env->GetByteArrayElements(row, 0);
        assert(row_data);
        for(int i = 0; i != OBSERVATION_WIDTH; ++i)
          getLevelSceneObservation[j][i] = Tile(row_data[i]);
        env->ReleaseByteArrayElements(row, row_data, JNI_ABORT);
      }

      /** Begin pit detection **/

      for(int i = 0; i != OBSERVATION_WIDTH; ++i) {
        for(int j = OBSERVATION_HEIGHT - 1; j != -1; --j) {
          if(getLevelSceneObservation[j][i].tile == TILE_IRRELEVANT)
            getLevelSceneObservation[j][i].detail.pit = true;
          else
            break;
        }
      }
      for(int j = OBSERVATION_HEIGHT - 2; j != -1; --j) {
        for(int i = 1; i != OBSERVATION_WIDTH; ++i) {
          if(getLevelSceneObservation[j][i - 1].tile == TILE_IRRELEVANT && !getLevelSceneObservation[j][i - 1].detail.pit && getLevelSceneObservation[j][i].detail.pit) {
            getLevelSceneObservation[j][i].detail.pit = false;
            getLevelSceneObservation[j][i].detail.above_pit = true;
          }
        }
        for(int i = OBSERVATION_WIDTH - 2; i != -1; --i) {
          if(getLevelSceneObservation[j][i + 1].tile == TILE_IRRELEVANT && !getLevelSceneObservation[j][i + 1].detail.pit && getLevelSceneObservation[j][i].detail.pit) {
            getLevelSceneObservation[j][i].detail.pit = false;
            getLevelSceneObservation[j][i].detail.above_pit = true;
          }
        }
      }

      /** End pit detection **/

      //getter = env->GetMethodID(cls, "getLevelSceneObservationZ", "(I)[[B");
      //assert(getter);
      //jobjectArray levelSceneObservation = jobjectArray(env->CallObjectMethod(observation, getter, 0));
      //for(int j = 0; j != OBSERVATION_HEIGHT; ++j) {
      //  jbyteArray row = jbyteArray(env->GetObjectArrayElement(levelSceneObservation, j));
      //  assert(row);
      //  jbyte *row_data = env->GetByteArrayElements(row, 0);
      //  assert(row_data);
      //  for(int i = 0; i != OBSERVATION_WIDTH; ++i)
      //    getLevelSceneObservation[j][i] = Tile(row_data[i]);
      //  env->ReleaseByteArrayElements(row, row_data, JNI_ABORT);
      //}

      getter = env->GetMethodID(cls, "getEnemiesObservation", "()[[B");
      assert(getter);
      jobjectArray enemiesObservation = jobjectArray(env->CallObjectMethod(observation, getter));
      for(int j = 0; j != OBSERVATION_HEIGHT; ++j) {
        jbyteArray row = jbyteArray(env->GetObjectArrayElement(enemiesObservation, j));
        assert(row);
        jbyte *row_data = env->GetByteArrayElements(row, 0);
        assert(row_data);
        for(int i = 0; i != OBSERVATION_WIDTH; ++i)
          getEnemiesObservation[j][i] = Object(row_data[i]);
        env->ReleaseByteArrayElements(row, row_data, JNI_ABORT);
      }
    }

    {
      getter = env->GetMethodID(cls, "getEnemiesFloatPos", "()[F");
      assert(getter);
      jfloatArray pos = jfloatArray(env->CallObjectMethod(observation, getter));
      assert(pos);
      float *pos_data = env->GetFloatArrayElements(pos, 0);
      assert(pos_data);
      for(int64_t i = 0, iend = env->GetArrayLength(pos); i + 2 < iend; i += 3)
        getEnemiesFloatPos.push_back(Enemy_Info(Object(int(pos_data[i])), std::make_pair(pos_data[i + 1], pos_data[i + 2])));
      env->ReleaseFloatArrayElements(pos, pos_data, JNI_ABORT);

      getter = env->GetMethodID(cls, "getMarioMode", "()I");
      assert(getter);
      getMarioMode = env->CallIntMethod(observation, getter);
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
      isMarioOnGround = env->CallBooleanMethod(observation, getter) != 0;
      getter = env->GetMethodID(cls, "mayMarioJump", "()Z");
      assert(getter);
      mayMarioJump = env->CallBooleanMethod(observation, getter) != 0;
      getter = env->GetMethodID(cls, "isMarioCarrying", "()Z");
      assert(getter);
      isMarioCarrying = env->CallBooleanMethod(observation, getter) != 0;
    }

    {
      getter = env->GetMethodID(cls, "getKillsTotal", "()I");
      assert(getter);
      getKillsTotal = env->CallIntMethod(observation, getter);
      getter = env->GetMethodID(cls, "getKillsByFire", "()I");
      assert(getter);
      getKillsByFire = env->CallIntMethod(observation, getter);
      getter = env->GetMethodID(cls, "getKillsByStomp", "()I");
      assert(getter);
      getKillsByStomp = env->CallIntMethod(observation, getter);
      getter = env->GetMethodID(cls, "getKillsByShell", "()I");
      assert(getter);
      getKillsByShell = env->CallIntMethod(observation, getter);
    }

    /// Higher order variable calculations

    {
      std::set<int64_t> unmatchedJs;
      const auto jend = unmatchedJs.end();
      for(int64_t i = 0, iend = getEnemiesFloatPos.size(); i != iend; ++i)
        unmatchedJs.insert(i);

      for(int64_t i = 0, iend = prev.getEnemiesFloatPos.size(); i != iend; ++i) {
        const auto &old = prev.getEnemiesFloatPos[i];

        for(auto jt = unmatchedJs.begin(); jt != jend; ++jt) {
          auto &cur = getEnemiesFloatPos[*jt];

          if(cur.which)
            continue;
          if(old.object != cur.object)
            continue;

          const double dx = cur.position.first - old.position.first;
          const double dy = cur.position.second - old.position.second;

          if(dx * dx + dy * dy > 25.0)
            continue;

          cur.which = old.which ? old.which : ++enemiesSeen;
          cur.velocity = std::make_pair(dx, dy);

          unmatchedJs.erase(jt);
          break;
        }
      }

      for(auto &enemy : getEnemiesFloatPos)
        enemy.which = enemy.which ? enemy.which : ++enemiesSeen;
    }

    getMarioFloatVel = std::make_pair(getMarioFloatPos.first - prev.getMarioFloatPos.first, getMarioFloatPos.second - prev.getMarioFloatPos.second);

    if((prev.mayMarioJump || prev.isMarioHighJumping) && prev.action[BUTTON_JUMP])
      isMarioHighJumping = true;
    if(mayMarioJump || (prev.isMarioHighJumping && !action[BUTTON_JUMP]) || getMarioFloatPos.second >= prev.getMarioFloatPos.second)
      isMarioHighJumping = false;
  }

  jbooleanArray State::to_jbooleanArray(JNIEnv *env) const {
    jbooleanArray j_action = env->NewBooleanArray(5);
    assert(j_action);

    jboolean *c_action = env->GetBooleanArrayElements(j_action, 0);
    assert(c_action);

    for(int i = 0; i != BUTTON_END; ++i)
		c_action[i] = g_current_state->action[i];

    env->ReleaseBooleanArrayElements(j_action, c_action, 0);

    return j_action;
  }

}

std::ostream & operator<<(std::ostream &os, const Mario::Tile &tile) {
  switch(tile) {
  case Mario::TILE_IRRELEVANT           : os << ' '; break;
  case Mario::TILE_SOMETHING            : os << 'S'; break;
  case Mario::TILE_BORDER               : os << 'B'; break;
  case Mario::TILE_HALF_BORDER          : os << 'H'; break;
  case Mario::TILE_BRICK                : os << 'R'; break;
  case Mario::TILE_POT_OR_CANNON        : os << 'C'; break;
  case Mario::TILE_QUESTION             : os << 'Q'; break;
  default:
    //os << int(tile) << ',';
    abort();
  }

  return os;
}

std::ostream & operator<<(std::ostream &os, const Mario::Object &object) {
  switch(object) {
  case Mario::OBJECT_IRRELEVANT         : os << 'g'; break;
  case Mario::OBJECT_GOOMBA             : os << 'g'; break;
  case Mario::OBJECT_GOOMBA_WINGED      : os << 'o'; break;
  case Mario::OBJECT_RED_KOOPA          : os << 'r'; break;
  case Mario::OBJECT_RED_KOOPA_WINGED   : os << 'e'; break;
  case Mario::OBJECT_GREEN_KOOPA        : os << 'k'; break;
  case Mario::OBJECT_GREEN_KOOPA_WINGED : os << 'w'; break;
  case Mario::OBJECT_BULLET_BILL        : os << 'b'; break;
  case Mario::OBJECT_SPIKY              : os << 's'; break;
  case Mario::OBJECT_SPIKY_WINGED       : os << 'p'; break;
  case Mario::OBJECT_ENEMY_FLOWER       : os << 'l'; break;
  case Mario::OBJECT_SHELL              : os << 'S'; break;
  case Mario::OBJECT_MUSHROOM           : os << 'm'; break;
  case Mario::OBJECT_FIRE_FLOWER        : os << 'f'; break;
  case Mario::OBJECT_FIREBALL           : os << 'i'; break;
  default:
    abort();
  }

  return os;
}

/* Implementation of native method sayHello() of HelloJNI class */
JNIEXPORT jbooleanArray JNICALL Java_ch_idsia_ai_agents_ai_JNIAgent_c_1getAction
  (JNIEnv *env, jobject /*obj*/, jobject observation)
{
  try {
    Mario::g_prev_state = Mario::g_current_state;
    Mario::g_current_state = std::make_shared<Mario::State>(*Mario::g_prev_state, env, observation);
  }
  catch(...) {
    std::cerr << "Exception in Java_ch_idsia_ai_agents_ai_JNIAgent_c_1getAction part 1." << std::endl;
    abort();
  }

  try {
    infinite_mario_ai(Mario::g_prev_state, Mario::g_current_state, Mario::g_current_state->action);
  }
  catch(...) {
    std::cerr << "Exception in Java_ch_idsia_ai_agents_ai_JNIAgent_c_1getAction part 2." << std::endl;
    abort();
  }

  return Mario::g_current_state->to_jbooleanArray(env);
}

JNIEXPORT void JNICALL Java_ch_idsia_ai_agents_ai_JNIAgent_c_1reset(JNIEnv *, jobject) {
  try {
    infinite_mario_reinit(Mario::g_prev_state, Mario::g_current_state);

    Mario::g_prev_state = std::make_shared<Mario::State>();
    Mario::g_current_state = std::make_shared<Mario::State>();
  }
  catch(...) {
    std::cerr << "Exception in Java_ch_idsia_ai_agents_ai_JNIAgent_c_1reset." << std::endl;
    abort();
  }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM * /*vm*/, void * /*pvt*/) {
//#ifndef NDEBUG
//  static volatile bool test = true;
//  while(test) {
//    continue;
//  }
//#endif

  try {
    Carli::Experiment experiment; ///< Set up global Options

    {
      std::ifstream fin;
      for(const auto &filename : {"args.txt", "../args.txt", "../../args.txt"}) {
        fin.open(filename);
        if(fin)
          break;
      }

      if(fin) {
        int argc = 0;
        std::vector<std::string> argvv;
        std::vector<const char *> argv(1, "carli");

	      {
		      std::string arg;
		      while(std::getline(fin, arg) && !arg.empty()) {
			      ++argc;
			      argvv.push_back(arg);
		      }
	      }

        if(argc) {
          ++argc;
          argv.reserve(argvv.size() + 1);
          for(const auto &arg : argvv)
            argv.push_back(arg.c_str());

          if(experiment.take_args(argc, &argv[0]) < argc) {
            Options &options = Options::get_global();

            options.print_help(std::cerr);
            std::cerr << std::endl;

            std::ostringstream oss;
            oss << "Unknown trailing arguments:";
            while(options.optind < argc)
              oss << ' ' << argv[static_cast<unsigned int>(options.optind++)];
            oss << std::endl;

            throw std::runtime_error(oss.str());
          }
        }
      }
    }

    Mario::g_prev_state = std::make_shared<Mario::State>();
    Mario::g_current_state = std::make_shared<Mario::State>();
  }
  catch(...) {
    std::cerr << "Exception in JNI_OnLoad." << std::endl;
    abort();
  }

  return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM * /*vm*/, void * /*pvt*/) {
  try {
    Mario::infinite_mario_reinit(Mario::g_prev_state, Mario::g_current_state);
  }
  catch(...) {
    std::cerr << "Exception in Java_ch_idsia_ai_agents_ai_JNIAgent_c_1reset." << std::endl;
    abort();
  }
}
