#include "blocks_world_2.h"

#include "carli/parser/rete_parser.h"
#include "carli/utility/shared_suffix_list.h"

#include <algorithm>
#include <queue>

namespace Blocks_World_2 {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  static const int32_t g_num_colors = 3;

  static bool xor_string(const std::string &str) {
    char x = '\0';
    for(const char &c : str)
      x ^= c;
    return x & 1;
  }

  Environment::Environment() {
    if(m_goal == Goal::COLOR) {
      m_match_test = [](const Environment::Block &lhs, const Environment::Block &rhs)->bool{
        return lhs.color == rhs.color;
      };
    }
    else /*if(m_goal == Goal::EXACT)*/ {
      m_match_test = [](const Environment::Block &lhs, const Environment::Block &rhs)->bool{
        return lhs.id == rhs.id;
      };
    }
//    else
//      abort();

    init_impl();
  }

  bool Environment::success() const {
    switch(m_goal) {
    case Goal::EXACT:
    case Goal::COLOR:
      {
        std::unordered_set<const Environment::Stack *> matched_stacks;
        for(const auto &target_stack : m_target) {
          const Environment::Stack * best_match = nullptr;
          int64_t best_match_chaff = std::numeric_limits<int64_t>::max();
          for(const auto &stack : m_blocks) {
            if(matched_stacks.find(&stack) != matched_stacks.end())
              continue;
            if(target_stack.size() < stack.size())
              continue;

            const auto match = std::mismatch(stack.begin(), stack.end(), target_stack.begin(), m_match_test);

            if(match.second == target_stack.end()) {
              const int64_t chaff = stack.end() - match.first;
              if(chaff < best_match_chaff) {
                best_match = &stack;
                best_match_chaff = chaff;
              }
            }
          }

          if(best_match)
            matched_stacks.insert(best_match);
          else
            return false;
        }

        return true;
      }

    case Goal::STACK:
      {
        return m_blocks.size() == 1;
      }

    case Goal::UNSTACK:
      {
        for(const auto &stack : m_blocks) {
          if(stack.size() != 1)
            return false;
        }

        return true;
      }

    case Goal::ON_A_B:
    {
      for(const auto &stack : m_blocks) {
        const auto found = std::find_if(stack.begin(), stack.end(), [](const Environment::Block &block)->bool {return block.id == 2; });
        if(found == stack.end())
          continue;
        const auto fp1 = found + 1;
        if(fp1 == stack.end())
          return false;
        return fp1->id == 1;
      }

      abort();
    }

    default:
      abort();
    }
  }

  int64_t Environment::calculate_num_steps_to_goal() const {
    typedef Zeni::Shared_Suffix_List<Block> Stack_SSL;
    typedef Zeni::Shared_Suffix_List<Stack_SSL::list_pointer_type> Stacks_SSL;

    std::unordered_set<Stack_SSL::list_pointer_type, Rete::hash_deref<Stack_SSL>, Rete::compare_deref_eq> all_stacks;
    std::unordered_set<Stacks_SSL::list_pointer_type, Rete::hash_deref<Stacks_SSL>, Rete::compare_deref_eq> all_stackses;

    const auto Stack_to_Stack_SSL = [&all_stacks](const Stack &stack)->Stack_SSL::list_pointer_type {
      Stack_SSL::list_pointer_type stack_ssl;
      for(auto st = stack.rbegin(); st != stack.rend(); ++st) {
        Stack_SSL::list_pointer_type insertion = std::make_shared<Stack_SSL>(*st, stack_ssl);
        auto inserted = all_stacks.insert(insertion);
        stack_ssl = *inserted.first;
      }
      return stack_ssl;
    };
    const auto Stack_SSL_to_Stack = [&all_stacks](const Stack_SSL::list_pointer_type &stack_ssl)->Stack {
      Stack stack;
      for(auto st = stack_ssl->begin(); st != stack_ssl->end(); ++st)
        stack.push_back(*st);
      return stack;
    };

    const auto Stacks_to_Stacks_SSL = [&all_stackses, &Stack_to_Stack_SSL](const Stacks &stacks)->Stacks_SSL::list_pointer_type {
      Stacks_SSL::list_pointer_type stacks_ssl;
      for(auto st = stacks.rbegin(); st != stacks.rend(); ++st) {
        Stacks_SSL::list_pointer_type insertion = std::make_shared<Stacks_SSL>(Stack_to_Stack_SSL(*st), stacks_ssl);
        auto inserted = all_stackses.insert(insertion);
        stacks_ssl = *inserted.first;
      }
      return stacks_ssl;
    };
    const auto Stacks_SSL_to_Stacks = [&all_stackses, &Stack_SSL_to_Stack](const Stacks_SSL::list_pointer_type &stacks_ssl)->Stacks {
      Stacks stacks;
      for(auto st = stacks_ssl->begin(); st != stacks_ssl->end(); ++st)
        stacks.push_back(Stack_SSL_to_Stack(*st));
      return stacks;
    };

    struct State : public std::enable_shared_from_this<State> {
      State(const Stacks_SSL::list_pointer_type &stacks_, const int64_t &num_steps_, const int64_t &heuristic_, const std::shared_ptr<State> &prev_state_ = nullptr) : prev_state(prev_state_), stacks(stacks_), num_steps(num_steps_), heuristic(heuristic_) {}

      bool operator<(const State &rhs) const {
        return num_steps + heuristic < rhs.num_steps + rhs.heuristic ||
          num_steps < rhs.num_steps || (num_steps == rhs.num_steps && (heuristic < rhs.heuristic || (heuristic == rhs.heuristic && stacks < rhs.stacks)));
      }

      std::shared_ptr<State> prev_state;
      Stacks_SSL::list_pointer_type stacks;
      int64_t num_steps;
      int64_t heuristic;
    };
    std::set<std::shared_ptr<State>, Rete::compare_deref_lt> astar;

    auto target = get_target();
    std::sort(target.begin(), target.end());
    const auto target_ssl = Stacks_to_Stacks_SSL(target);

    //for(auto stack : target) {
    //  std::cerr << "  ";
    //  for(auto block : stack) {
    //    std::cerr << block.id << ' ';
    //  }
    //  std::cerr << std::endl;
    //}
    //target = Stacks_SSL_to_Stacks(target_ssl);
    //for(auto stack : target) {
    //  std::cerr << "  ";
    //  for(auto block : stack) {
    //    std::cerr << block.id << ' ';
    //  }
    //  std::cerr << std::endl;
    //}
    //const auto target_ssl2 = Stacks_to_Stacks_SSL(target);
    //exit(42);

    const auto heuristic = [this, &target](const Stacks &stacks)->int64_t {
      //return 0;

      std::unordered_map<Block, std::pair<Stacks::const_iterator, Stack::const_iterator>> target_pos;
      for(auto tt = target.begin(); tt != target.end(); ++tt) {
        for(auto t2 = tt->begin(); t2 != tt->end(); ++t2)
          target_pos[*t2] = std::make_pair(tt, t2);
      }

      std::unordered_map<Block, std::pair<Stacks::const_iterator, Stack::const_iterator>> stack_pos;
      for(auto st = stacks.begin(); st != stacks.end(); ++st) {
        for(auto s2 = st->begin(); s2 != st->end(); ++s2)
          stack_pos[*s2] = std::make_pair(st, s2);
      }

      //std::unordered_map<Block, Block> stack_below;
      //for(auto &stack : stacks) {
      //  for(auto s1 = stack.begin(), s2 = s1 + 1; s2 != stack.end(); s1 = s2, ++s2) {
      //    stack_below[*s2] = *s1;
      //  }
      //}

      std::unordered_map<Block, bool> in_place;
      size_t dist = 0;
      for(auto &target_stack : target) {
        auto matching_stack = std::find_if(stacks.begin(), stacks.end(), [this,&target_stack](const Stack &stack)->bool{return this->m_match_test(*stack.begin(), *target_stack.begin());});
        if(matching_stack == stacks.end()) {
//          std::cerr << "c" << target_stack.size() << std::endl;
          dist += target_stack.size();
        }
        else {
          auto tt = target_stack.begin();
          auto mt = matching_stack->begin();
          size_t dist2 = target_stack.size();
          while(tt != target_stack.end() && mt != matching_stack->end()) {
            if(this->m_match_test(*tt, *mt)) {
              --dist2;
              in_place[*tt] = true;
            }
            else
              break;
            ++tt;
            ++mt;
          }
//          const auto match = std::mismatch(target_stack.begin(), target_stack.end(), matching_stack->begin(), this->get_match_test());
//          std::cerr << "i" << target_stack.end() - tt << std::endl;
          dist += dist2;
        }
      }

      std::unordered_map<Block, bool> marked_double;
      for(auto &stack : stacks) {
        std::unordered_set<Block> below;
        std::unordered_set<Block> out_of_place_below;
        for(auto st = stack.begin(); st != stack.end(); ++st) {
          if(in_place[*st]) {
            below.insert(*st);
            continue;
          }

          // If a block it must be above in the target is out of place below, it must be moved an extra time
          for(auto oopb : out_of_place_below) {
            if(target_pos[oopb].first == target_pos[*st].first && target_pos[oopb].second < target_pos[*st].second) {
              marked_double[*st] = true;
              ++dist;
              break;
            }
          }

          // If it is on the target stack but out of place, it must be moved an extra time anyway
          if(!marked_double[*st]) {
            if(target_pos[*st].second != target_pos[*st].first->begin()) {
              if(below.find(*target_pos[*st].first->begin()) != below.end()) {
                marked_double[*st] = true;
                ++dist;
              }
            }
          }

          below.insert(*st);
          out_of_place_below.insert(*st);
        }
      }

      return int64_t(dist);
    };

    std::unordered_set<Stacks_SSL::list_pointer_type, Rete::hash_deref<Stacks_SSL>, Rete::compare_deref_eq> states_evaluated;

    {
      auto blocks = m_blocks;
      std::sort(blocks.begin(), blocks.end());
      auto blocks_ssl = Stacks_to_Stacks_SSL(blocks);
      states_evaluated.insert(blocks_ssl);
      astar.insert(std::make_shared<State>(blocks_ssl, 0, heuristic(blocks)));
    }

    int64_t num_steps = 0;
    while(!astar.empty()) {
      const auto state = *astar.begin();
      astar.erase(astar.begin());

      //std::cerr << astar.size() << ' ' << state->num_steps << ' ' << state->heuristic << std::endl;

      if(state->stacks == target_ssl) {
        std::stack<std::shared_ptr<State>> states;
        for(auto ss = state; ss; ss = ss->prev_state)
          states.push(ss);
        //int64_t step = 0;
        while(!states.empty()) {
          auto ss = states.top();
          states.pop();

          //auto stacks = Stacks_SSL_to_Stacks(ss->stacks);
          //std::cerr << "Step " << step++ << " -- " << ss->num_steps << "+" << ss->heuristic << std::endl;
          //for(auto stack : stacks) {
          //  std::cerr << "  ";
          //  for(auto block : stack) {
          //    std::cerr << block.id << ' ';
          //  }
          //  std::cerr << std::endl;
          //}
        }

        num_steps = state->num_steps;
        //std::cerr << "Num steps = " << num_steps << std::endl;
        break;
      }

      const auto stacks = Stacks_SSL_to_Stacks(state->stacks);

      std::unordered_map<Block, std::pair<Stacks::const_iterator, Stack::const_iterator>> target_pos;
      for(auto tt = target.begin(); tt != target.end(); ++tt) {
        for(auto t2 = tt->begin(); t2 != tt->end(); ++t2)
          target_pos[*t2] = std::make_pair(tt, t2);
      }

      std::unordered_map<Block, std::pair<Stacks::const_iterator, Stack::const_iterator>> stack_pos;
      for(auto st = stacks.begin(); st != stacks.end(); ++st) {
        for(auto s2 = st->begin(); s2 != st->end(); ++s2)
          stack_pos[*s2] = std::make_pair(st, s2);
      }

      std::unordered_map<Block, bool> in_place;
      for(auto &target_stack : target) {
        auto matching_stack = std::find_if(stacks.begin(), stacks.end(), [&target_stack](const Stack &stack)->bool{return *stack.begin() == *target_stack.begin();});
        if(matching_stack != stacks.end()) {
          auto tt = target_stack.begin();
          auto mt = matching_stack->begin();
          while(tt != target_stack.end() && mt != matching_stack->end()) {
            if(*tt == *mt) {
              in_place[*tt] = true;
            }
            else
              break;
            ++tt;
            ++mt;
          }
        }
      }
      
      bool move_into_place_found = false;
      for(auto st = stacks.begin(), send = stacks.end(); st != send && !move_into_place_found; ++st) {
        const auto block = *st->rbegin();
        if(in_place[block])
          continue;
        if(target_pos[block].second == target_pos[block].first->begin()) {
          /// Consider only moving the block into place on the table
          move_into_place_found = true;
          
          Stacks next = stacks;
          auto ni = next.begin() + (st - stacks.begin());
          ni->pop_back();
          next.push_back({block});
          std::sort(next.begin(), next.end());
          auto next_ssl = Stacks_to_Stacks_SSL(next);
          if(states_evaluated.find(next_ssl) == states_evaluated.end()) {
            states_evaluated.insert(next_ssl);
            astar.insert(std::make_shared<State>(next_ssl, state->num_steps + 1, heuristic(next), state));
          }
          
          break;
        }
        else {
          const auto target_dest = *(target_pos[block].second - 1);
          if(in_place[target_dest] && stack_pos[target_dest].second + 1 == stack_pos[target_dest].first->end()) {
            /// Consider only moving the block into place on the target block beneath
            move_into_place_found = true;
            const auto tt = stack_pos[target_dest].first;

            Stacks next = stacks;
            auto ni = next.begin() + (st - stacks.begin());
            auto ti = next.begin() + (tt - stacks.begin());
            ti->push_back(*ni->rbegin());
            ni->pop_back();
            if(ni->empty())
              next.erase(ni);
            std::sort(next.begin(), next.end());
            auto next_ssl = Stacks_to_Stacks_SSL(next);
            if(states_evaluated.find(next_ssl) == states_evaluated.end()) {
              states_evaluated.insert(next_ssl);
              astar.insert(std::make_shared<State>(next_ssl, state->num_steps + 1, heuristic(next), state));
            }

            break;
          }
        }
      }
      
      if(move_into_place_found)
        continue;
      
      for(auto st = stacks.begin(), send = stacks.end(); st != send; ++st) {
        if(st->size() > 1 && !in_place[*st->rbegin()]) {
          Stacks next = stacks;
          auto ni = next.begin() + (st - stacks.begin());
          auto block = *ni->rbegin();
          ni->pop_back();
          next.push_back({block});
          std::sort(next.begin(), next.end());
          auto next_ssl = Stacks_to_Stacks_SSL(next);
          if(states_evaluated.find(next_ssl) == states_evaluated.end()) {
            states_evaluated.insert(next_ssl);
            astar.insert(std::make_shared<State>(next_ssl, state->num_steps + 1, heuristic(next), state));
          }
        }

//         for(auto tt = stacks.begin(); tt != send; ++tt) {
//           if(st == tt)
//             continue;
//           Stacks next = stacks;
//           auto ni = next.begin() + (st - stacks.begin());
//           auto ti = next.begin() + (tt - stacks.begin());
//           ti->push_back(*ni->rbegin());
//           ni->pop_back();
//           if(ni->empty())
//             next.erase(ni);
//           std::sort(next.begin(), next.end());
//           auto next_ssl = Stacks_to_Stacks_SSL(next);
//           if(states_evaluated.find(next_ssl) == states_evaluated.end()) {
//             states_evaluated.insert(next_ssl);
//             astar.insert(std::make_shared<State>(next_ssl, state->num_steps + 1, heuristic(next), state));
//           }
//         }
      }
    }

    return num_steps;
  }

  void Environment::init_impl() {
    /// Begin scenario shenanigans

    switch(get_scenario()) {
      case 1:
        if(get_total_step_count() < 50000 || get_total_step_count() >= 100000)
          m_goal = Goal::STACK;
        else
          m_goal = Goal::UNSTACK;
        break;

      case 2:
        if(get_total_step_count() < 50000 || get_total_step_count() >= 100000)
          m_goal = Goal::STACK;
        else
          m_goal = Goal::ON_A_B;
        break;

      case 3:
        if(get_total_step_count() < 50000 || get_total_step_count() >= 100000)
          m_goal = Goal::UNSTACK;
        else
          m_goal = Goal::STACK;
        break;

      case 4:
        if(get_total_step_count() < 50000 || get_total_step_count() >= 100000)
          m_goal = Goal::UNSTACK;
        else
          m_goal = Goal::ON_A_B;
        break;

      case 5:
        if(get_total_step_count() < 50000 || get_total_step_count() >= 100000)
          m_goal = Goal::ON_A_B;
        else
          m_goal = Goal::STACK;
        break;

      case 6:
        if(get_total_step_count() < 50000 || get_total_step_count() >= 100000)
          m_goal = Goal::ON_A_B;
        else
          m_goal = Goal::UNSTACK;
        break;

      case 7:
        if(get_total_step_count() < 50000 || get_total_step_count() >= 100000) {
          if(m_goal == Goal::COLOR) {
            m_match_test = [](const Environment::Block &lhs, const Environment::Block &rhs)->bool{
              return lhs.id == rhs.id;
            };
            m_goal = Goal::EXACT;
          }
        }
        else {
          if(m_goal == Goal::EXACT) {
            m_match_test = [](const Environment::Block &lhs, const Environment::Block &rhs)->bool{
              return lhs.color == rhs.color;
            };
            m_goal = Goal::COLOR;
          }
        }
        break;

      case 8:
        if(get_total_step_count() < 50000 || get_total_step_count() >= 100000) {
          if(m_goal == Goal::EXACT) {
            m_match_test = [](const Environment::Block &lhs, const Environment::Block &rhs)->bool{
              return lhs.color == rhs.color;
            };
            m_goal = Goal::COLOR;
          }
        }
        else {
          if(m_goal == Goal::COLOR) {
            m_match_test = [](const Environment::Block &lhs, const Environment::Block &rhs)->bool{
              return lhs.id == rhs.id;
            };
            m_goal = Goal::EXACT;
          }
        }
        break;

      default:
        break;
    }

    /// Begin normal init

    assert(m_num_blocks_min > 2);
    assert(m_num_blocks_max >= m_num_blocks_min);

    int64_t num_blocks = 0;
//    if(get_Option_Ranged<int64_t>(Options::get_global(), "num-episodes") == 156) {
//      int64_t eps = 0;
//      for(num_blocks = 3; num_blocks != 10; ++num_blocks) {
//        eps += 3 * num_blocks;
//        if(eps >= get_episode_count())
//          break;
//      }
//    }
//    else if(get_Option_Ranged<int64_t>(Options::get_global(), "num-episodes") <= 100) {
////       if(get_episode_count() <= 15)
////         num_blocks = 3;
////       else if(get_episode_count() <= 40)
////         num_blocks = 4;
////       else
////         num_blocks = 5;
//
////       if(get_step_count() < 5000)
////         num_blocks = 3;
////       else if(get_step_count() < 15000)
////         num_blocks = 3 + m_random.rand_lte(int32_t(1));
////       else
////         num_blocks = 3 + m_random.rand_lte(int32_t(2));
//
//      if(get_step_count() < 50000)
//        num_blocks = 3 + m_random.rand_lte(int32_t(1));
//      else
//        num_blocks = 3 + m_random.rand_lte(int32_t(2));
//    }
//    else
    {
      num_blocks = m_num_blocks_min + (m_num_blocks_min != m_num_blocks_max ? m_random.rand_lte(int32_t(m_num_blocks_max - m_num_blocks_min)) : 0);
    }

    std::vector<Block> blocks;
    blocks.reserve(num_blocks);
    for(int i = 1; i <= num_blocks; ++i)
      blocks.push_back(Block(i, m_random.rand_lt(g_num_colors)));
    m_table = Block(0, m_random.rand_lt(g_num_colors));

    std::shuffle(blocks.begin(), blocks.end(), m_random);

    if(m_goal == Goal::EXACT || m_goal == Goal::COLOR) {
      std::vector<Block> target_blocks = blocks;
      std::shuffle(target_blocks.begin(), target_blocks.end(), m_random);

      m_target = random_Stacks(target_blocks);
    }

    do {
      m_blocks = random_Stacks(blocks);
    } while(success());

    if(m_goal == Goal::STACK) {
      m_num_steps_to_goal = 0;
      size_t largest = 0;
      for(auto st = m_blocks.begin(); st != m_blocks.end(); ++st) {
        if(st->size() > largest) {
          m_num_steps_to_goal += int64_t(largest);
          largest = st->size();
        }
        else
          m_num_steps_to_goal += int64_t(st->size());
      }
    }
    else if(m_goal == Goal::UNSTACK) {
      m_num_steps_to_goal = 0;
      for(auto st = m_blocks.begin(); st != m_blocks.end(); ++st)
        m_num_steps_to_goal += int64_t(st->size() - 1);
    }
    else if(m_goal == Goal::ON_A_B) {
      m_num_steps_to_goal = 0;
      for(auto st = m_blocks.begin(); st != m_blocks.end(); ++st) {
        Stack::iterator block_1 = st->end(), block_2 = st->end();
        for(auto bt = st->begin(); bt != st->end(); ++bt) {
          if(bt->id == 1)
            block_1 = bt;
          else if(bt->id == 2)
            block_2 = bt;
        }
        
        if(block_1 != st->end()) {
          if(block_2 != st->end()) {
            if(block_1 < block_2)
              m_num_steps_to_goal += int64_t(st->end() - block_1);
            else
              m_num_steps_to_goal += int64_t(st->end() - block_2);
            break;
          }
          else
            m_num_steps_to_goal += int64_t(st->end() - block_1);
        }
        else if(block_2 != st->end())
          m_num_steps_to_goal += int64_t(st->end() - block_2 - 1);
      }
    }
    else if(m_goal == Goal::EXACT /*|| m_goal == Goal::COLOR*/) {
      static bool skip_first = true;
      if(skip_first)
        skip_first = false;
      else
        m_num_steps_to_goal = calculate_num_steps_to_goal();
    }
  }

  Environment::Stacks Environment::random_Stacks(const std::vector<Block> &blocks) {
    Stacks stacks;
    for(const auto &block : blocks) {
      const size_t stack_index = m_random.rand_lte(int32_t(stacks.size()));
      if(stack_index >= stacks.size())
        stacks.push_back(Stack(1, block));
      else
        stacks[stack_index].push_back(block);
    }
    std::sort(stacks.begin(), stacks.end(), [](const Stack &lhs, const Stack &rhs)->bool {
      if(lhs.size() < rhs.size())
        return false;
      if(rhs.size() < lhs.size())
        return true;
      return lhs < rhs;
    });

    return stacks;
  }

  std::pair<Agent::reward_type, Agent::reward_type> Environment::transition_impl(const Carli::Action &action) {
    const int64_t best_match_total_before = matching_blocks();
    const Move &move = debuggable_cast<const Move &>(action);

    Stacks::iterator src = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && stack.rbegin()->id == move.block;
    });
    Stacks::iterator dest = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && stack.rbegin()->id == move.dest;
    });

    if(src == m_blocks.end())
      return std::make_pair(-10.0, -10.0); ///< throw std::runtime_error("Attempt to move Block from under another Block.");

    const Block block = *src->rbegin();

    src->pop_back();

    if(dest != m_blocks.end()) {
      dest->push_back(block);
      if(src->empty())
        m_blocks.erase(src);
    }
    else {
      assert(move.dest == 0);
      assert(src->size() != 0);
      m_blocks.push_back(Stack(1, block));
    }

    std::sort(m_blocks.begin(), m_blocks.end());

    const int64_t best_match_total_after = matching_blocks();

    if(m_reward == Reward::GUIDING)
      return std::make_pair(double(best_match_total_after - best_match_total_before - 1.0 /* Because positive numbers are bad */), -1.0);
    else
      return std::make_pair(-1.0, -1.0);
  }

  int64_t Environment::matching_blocks() const {
    int64_t best_match_total = 0u;

    switch(m_goal) {
    case Goal::EXACT:
    case Goal::COLOR:
      {
        std::unordered_set<const Environment::Stack *> matched_stacks;
        for(const auto &target_stack : m_target) {
          const Environment::Stack * best_match = nullptr;
          size_t best_match_size = 0u;

          for(const auto &stack : m_blocks) {
            if(matched_stacks.find(&stack) != matched_stacks.end())
              continue;
            if(target_stack.size() < stack.size())
              continue;

            const auto match = std::mismatch(stack.begin(), stack.end(), target_stack.begin(), m_match_test);

            if(match.first == stack.end()) {
              if(match.second == target_stack.end()) {
                best_match = &stack;
                best_match_size = stack.size();
                break;
              }
              else if(stack.size() > best_match_size) {
                best_match = &stack;
                best_match_size = stack.size();
              }
            }
          }

          if(best_match) {
            matched_stacks.insert(best_match);
            best_match_total += best_match_size;
          }
        }

        break;
      }

    case Goal::STACK:
      {
        int64_t max_height = 0;
        for(const auto &stack : m_blocks) {
          if(int64_t(stack.size()) > max_height) {
            best_match_total -= max_height;
            max_height = stack.size();
          }
          else
            best_match_total -= stack.size();
        }

        break;
      }

    case Goal::UNSTACK:
      {
        best_match_total = m_blocks.size();

        break;
      }

    case Goal::ON_A_B:
      {
        for(const auto &stack : m_blocks) {
          auto founda = std::find_if(stack.begin(), stack.end(), [](const Environment::Block &block)->bool{return block.id == 1;});
          auto foundb = std::find_if(stack.begin(), stack.end(), [](const Environment::Block &block)->bool{return block.id == 2;});

          /// Neither A nor B
          if(founda == stack.end() && foundb == stack.end())
            best_match_total += stack.size(); ///< All blocks are fine

          /// Only A
          if(founda != stack.end() && foundb == stack.end())
            best_match_total += founda - stack.begin(); ///< Every block below A is fine

          /// B above A
          if(founda != stack.end() && foundb > founda)
            best_match_total += founda - stack.begin(); ///< Every block below A is fine; B will still only have to be moved once

          /// Only B
          if(founda == stack.end() && foundb != stack.end())
            best_match_total += foundb - stack.begin() + 1; ///< Every block B and below is fine

          /// A above B by more than 1
          if(foundb != stack.end() && founda > foundb + 1)
            best_match_total += foundb - stack.begin(); ///< Every block B and below is fine, but A will have to be moved twice

          /// Goal
          if(foundb != stack.end() && foundb + 1 == founda)
            best_match_total = stack.size(); ///< No blocks have to be moved
        }

        break;
      }

    default:
      abort();
    }

    return best_match_total;
  }

  void Environment::print_impl(ostream &os) const {
    os << "Blocks World (Table is Left):" << endl;
    for(const Stack &stack : m_blocks) {
      for(const Block &block : stack)
        os << ' ' << (block.color == 0 ? 'r' : block.color == 1 ? 'g' : 'b') << block.id;
      os << endl;
    }
    os << "Goal:" << endl;
    switch(m_goal) {
    case Goal::EXACT:
    case Goal::COLOR:
      for(const Stack &stack : m_target) {
        for(const Block &block : stack)
          os << ' ' << (block.color == 0 ? 'r' : block.color == 1 ? 'g' : 'b') << block.id;
        os << endl;
      }
      break;

    case Goal::STACK:
      os << " Stack" << endl;
      break;

    case Goal::UNSTACK:
      os << " Unstack" << endl;
      break;

    case Goal::ON_A_B:
      os << " On(A,B)" << endl;
      break;

    default:
      abort();
    }
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env_)
   : Carli::Agent(env_, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(variables, token);})
  {
    auto env = dynamic_pointer_cast<const Environment>(get_env());
    std::ostringstream oss;
    for(int64_t block_id = 0; block_id != env->get_num_blocks_max() + 1; ++block_id) {
      oss << block_id;
      m_block_ids[block_id] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(oss.str()));
      m_block_names[block_id] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(block_id));
      m_stack_ids[block_id] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string("|") + oss.str()));
      m_target_stack_ids[block_id] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(":") + oss.str()));
      oss.str("");
    }

    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
//    std::cerr << "Feature Generation Time: " << m_feature_generation_time << std::endl;

    destroy();
  }

  void Agent::generate_rete() {
    std::string rules_in = dynamic_cast<const Option_String &>(Options::get_global()["rules"]).get_value();
    if(rules_in == "default")
      rules_in = "rules/blocks-world-2.carli";
    if(rete_parse_file(*this, rules_in))
      abort();
  }

  void Agent::generate_features() {
//    using dseconds = std::chrono::duration<double, std::ratio<1,1>>;
//    const auto start_time = std::chrono::high_resolution_clock::now();

    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &blocks = env->get_blocks();
    const auto &target = env->get_target();
    const auto &table = env->get_table();

    if(get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes")) {
      Rete::Agenda::Locker locker(agenda);
      clear_old_wmes();
    }

    Rete::Agenda::Locker locker(agenda);

    if(env->get_goal() == Environment::Goal::ON_A_B)
      insert_new_wme(std::make_shared<Rete::WME>(m_block_ids[1], m_goal_on_attr, m_block_ids[2]));
    else if(env->get_goal() == Environment::Goal::EXACT) {
      for(const auto &stack : target) {
        Rete::Symbol_Identifier_Ptr_C prev_id = m_table_id;
        insert_new_wme(std::make_shared<Rete::WME>(m_block_ids[stack.begin()->id], m_goal_on_attr, m_table_id));
        for(const auto &block : stack) {
          const Rete::Symbol_Identifier_Ptr_C block_id = m_block_ids[block.id];
          assert(block.id);
          insert_new_wme(std::make_shared<Rete::WME>(block_id, m_goal_on_attr, prev_id));
          prev_id = block_id;
        }
      }
    }

    std::ostringstream oss;
    int64_t max_height = 0;
    for(const auto &stack : blocks) {
      const Rete::Symbol_Identifier_Ptr_C stack_id = m_stack_ids[stack.begin()->id];

      int64_t height = 0;
      for(const auto &block : stack) {
        const auto block_id = m_block_ids[block.id];
        insert_new_wme(std::make_shared<Rete::WME>(block_id, m_height_attr, std::make_shared<Rete::Symbol_Constant_Int>(++height)));

        insert_new_wme(std::make_shared<Rete::WME>(block_id, m_color_attr, std::make_shared<Rete::Symbol_Constant_Int>(block.color)));

        const double brightness = m_random.frand_lte();
        insert_new_wme(std::make_shared<Rete::WME>(block_id, m_brightness_attr, std::make_shared<Rete::Symbol_Constant_Float>(brightness)));
        if(brightness >= 0.5)
          insert_new_wme(std::make_shared<Rete::WME>(block_id, m_glowing_attr, m_true_value));
      }
      max_height = std::max(max_height, int64_t(stack.size()));

      for(const auto &dest_stack : blocks) {
        if(stack == dest_stack)
          continue;
        oss << "move-" << stack.rbegin()->id << "-|" << dest_stack.begin()->id;
        const Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
        const Rete::Symbol_Identifier_Ptr_C dest_id = m_stack_ids[dest_stack.begin()->id];
        oss.str("");
        insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
        insert_new_wme(std::make_shared<Rete::WME>(dest_id, m_action_in_attr, action_id));
        insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_action_out_attr, action_id));
        insert_new_wme(std::make_shared<Rete::WME>(action_id, m_block_attr, m_block_ids[stack.rbegin()->id]));
        insert_new_wme(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_block_ids[dest_stack.rbegin()->id]));
      }

      if(stack.size() > 1) {
        oss << "move-" << stack.rbegin()->id << "-TABLE";
        const Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
        oss.str("");
        insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
        insert_new_wme(std::make_shared<Rete::WME>(m_table_id, m_action_in_attr, action_id));
        insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_action_out_attr, action_id));
        insert_new_wme(std::make_shared<Rete::WME>(action_id, m_block_attr, m_block_ids[stack.rbegin()->id]));
        insert_new_wme(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_table_id));
      }
    }
    insert_new_wme(std::make_shared<Rete::WME>(m_table_id, m_height_attr, std::make_shared<Rete::Symbol_Constant_Int>(0)));
    insert_new_wme(std::make_shared<Rete::WME>(m_table_id, m_color_attr, std::make_shared<Rete::Symbol_Constant_Int>(table.color)));
    const double brightness = m_random.frand_lte();
    insert_new_wme(std::make_shared<Rete::WME>(m_table_id, m_brightness_attr, std::make_shared<Rete::Symbol_Constant_Float>(brightness)));
    if(brightness >= 0.5)
      insert_new_wme(std::make_shared<Rete::WME>(m_table_id, m_glowing_attr, m_true_value));

    insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_blocks_attr, m_blocks_id));
    insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_stacks_attr, m_stacks_id));
    insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_target_attr, m_target_id));
    insert_new_wme(std::make_shared<Rete::WME>(m_blocks_id, m_block_attr, m_table_id));
    insert_new_wme(std::make_shared<Rete::WME>(m_table_id, m_name_attr, m_table_name));
    insert_new_wme(std::make_shared<Rete::WME>(m_stacks_id, m_stack_attr, m_table_stack_id));
    insert_new_wme(std::make_shared<Rete::WME>(m_target_id, m_stack_attr, m_table_stack_id));
    insert_new_wme(std::make_shared<Rete::WME>(m_table_stack_id, m_top_attr, m_table_id));
    insert_new_wme(std::make_shared<Rete::WME>(m_table_stack_id, m_matches_attr, m_table_stack_id));
//    if(get_total_step_count() < 5000) {
//      insert_new_wme(std::make_shared<Rete::WME>(m_table_stack_id, m_early_matches_attr, m_table_stack_id));
//      if(xor_string(m_table_stack_id->value))
//        insert_new_wme(std::make_shared<Rete::WME>(m_table_stack_id, m_late_matches_attr, m_table_stack_id));
//    }
//    else {
//      if(xor_string(m_table_stack_id->value))
//        insert_new_wme(std::make_shared<Rete::WME>(m_table_stack_id, m_early_matches_attr, m_table_stack_id));
//      insert_new_wme(std::make_shared<Rete::WME>(m_table_stack_id, m_late_matches_attr, m_table_stack_id));
//    }

    for(const auto &stack : blocks) {
      Rete::Symbol_Identifier_Ptr_C stack_id = m_stack_ids[stack.begin()->id];

      insert_new_wme(std::make_shared<Rete::WME>(m_stacks_id, m_stack_attr, stack_id));
      for(const auto &block : stack) {
        const Rete::Symbol_Identifier_Ptr_C block_id = m_block_ids[block.id];
        insert_new_wme(std::make_shared<Rete::WME>(m_blocks_id, m_block_attr, block_id));
        insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_block_attr, block_id));
        insert_new_wme(std::make_shared<Rete::WME>(block_id, m_name_attr, m_block_names[block.id]));
        insert_new_wme(std::make_shared<Rete::WME>(block_id, m_above_attr, m_table_id));
        insert_new_wme(std::make_shared<Rete::WME>(block_id, m_higher_than_attr, m_table_id));
      }

      insert_new_wme(std::make_shared<Rete::WME>(m_block_ids[stack.rbegin()->id], m_clear_attr, m_true_value));

      insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_top_attr, m_block_ids[stack.rbegin()->id]));
      ///insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_matches_attr, m_table_stack_id));
      if(int64_t(stack.size()) == max_height)
        insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_tallest_attr, m_true_value));
    }

    for(const auto &stack : blocks) {
      int64_t block1_height = 1;
      for(const auto &block1 : stack) {
        const Rete::Symbol_Identifier_Ptr_C block1_id = m_block_ids[block1.id];
        if(block1_height == 1)
          insert_new_wme(std::make_shared<Rete::WME>(block1_id, m_on_attr, m_table_id));
        int64_t block2_height = 1;
        for(const auto &block2 : stack) {
          if(block2_height >= block1_height)
            break;
          const Rete::Symbol_Identifier_Ptr_C block2_id = m_block_ids[block2.id];
          if(block1_height == block2_height + 1)
            insert_new_wme(std::make_shared<Rete::WME>(block1_id, m_on_attr, block2_id));
          insert_new_wme(std::make_shared<Rete::WME>(block1_id, m_above_attr, block2_id));
          ++block2_height;
        }
        ++block1_height;
      }
    }

    for(const auto &stack1 : blocks) {
      int64_t block1_height = 1;
      for(const auto &block1 : stack1) {
        const Rete::Symbol_Identifier_Ptr_C block1_id = m_block_ids[block1.id];
        for(const auto &stack2 : blocks) {
          int64_t block2_height = 1;
          for(const auto &block2 : stack2) {
            if(block2_height >= block1_height)
              break;
            const Rete::Symbol_Identifier_Ptr_C block2_id = m_block_ids[block2.id];
            insert_new_wme(std::make_shared<Rete::WME>(block1_id, m_higher_than_attr, block2_id));
            ++block2_height;
          }
        }
        ++block1_height;
      }
    }

    std::unordered_set<const Environment::Stack *> matched_stacks;
    for(const auto &target_stack : target) {
      const Environment::Stack * best_match = nullptr;
      size_t best_match_size = 0u;

      for(const auto &stack : blocks) {
        if(matched_stacks.find(&stack) != matched_stacks.end())
          continue;
        if(target_stack.size() < stack.size())
          continue;

        const auto match = std::mismatch(stack.begin(), stack.end(), target_stack.begin(), env->get_match_test());

        if(match.first == stack.end()) {
          if(match.second == target_stack.end()) {
            best_match = &stack;
            best_match_size = stack.size();
            break;
          }
          else if(stack.size() > best_match_size) {
            best_match = &stack;
            best_match_size = stack.size();
          }
        }
      }

      if(best_match) {
        const Rete::Symbol_Identifier_Ptr_C target_id = m_target_stack_ids[target_stack.begin()->id];
        const Rete::Symbol_Identifier_Ptr_C stack_id = m_stack_ids[(*best_match).begin()->id];

        const auto bmend = target_stack.size() < best_match->size() ? best_match->begin() + target_stack.size() : best_match->end();
        const auto match = std::mismatch(best_match->begin(), bmend, target_stack.begin(), env->get_match_test());

        insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_matches_attr, target_id));
//        if(get_total_step_count() < 5000) {
//          insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_early_matches_attr, target_id));
//          if(xor_string(stack_id->value) ^ xor_string(target_id->value))
//            insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_late_matches_attr, target_id));
//        }
//        else {
//          if(xor_string(stack_id->value) ^ xor_string(target_id->value))
//            insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_early_matches_attr, target_id));
//          insert_new_wme(std::make_shared<Rete::WME>(stack_id, m_late_matches_attr, target_id));
//        }

        if(match.second != target_stack.end()) {
          for(const auto &stack : blocks) {
            for(const auto &block : stack) {
              if(env->get_match_test()(block, *match.second)) {
                const Rete::Symbol_Identifier_Ptr_C block_id = m_block_ids[block.id];
                insert_new_wme(std::make_shared<Rete::WME>(block_id, m_matches_top_attr, stack_id));
                if(get_total_step_count() < 5000) {
                  insert_new_wme(std::make_shared<Rete::WME>(block_id, m_early_matches_top_attr, stack_id));
                  if(xor_string(block_id->value) ^ xor_string(stack_id->value))
                    insert_new_wme(std::make_shared<Rete::WME>(block_id, m_late_matches_top_attr, stack_id));
                }
                else {
                  if(xor_string(block_id->value) ^ xor_string(stack_id->value))
                    insert_new_wme(std::make_shared<Rete::WME>(block_id, m_early_matches_top_attr, stack_id));
                  insert_new_wme(std::make_shared<Rete::WME>(block_id, m_late_matches_top_attr, stack_id));
                }
              }
            }
          }
        }

        matched_stacks.insert(best_match);
      }
    }

    int64_t discrepancy = 0;
    for(const auto &target_stack : target) {
      discrepancy += target_stack.size();
      for(const auto &stack : blocks) {
        const auto gsend = stack.size() < target_stack.size() ? target_stack.begin() + stack.size() : target_stack.end();
        const auto match = std::mismatch(target_stack.begin(), gsend, stack.begin(), env->get_match_test());
        if(match.first != target_stack.begin()) {
          discrepancy -= match.first - target_stack.begin();
          for(auto bt = target_stack.begin(); bt != match.first; ++bt)
            insert_new_wme(std::make_shared<Rete::WME>(m_block_ids[bt->id], m_in_place_attr, m_true_value));
          break;
        }
      }
      assert(!target_stack.empty());
      const Rete::Symbol_Identifier_Ptr_C target_base_id = m_block_ids[target_stack.begin()->id];
      insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_matches_top_attr, m_table_stack_id));
//      if(get_total_step_count() < 5000) {
//        insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_early_matches_top_attr, m_table_stack_id));
//        if(xor_string(target_base_id->value) ^ xor_string(m_table_stack_id->value))
//          insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_late_matches_top_attr, m_table_stack_id));
//      }
//      else {
//        if(xor_string(target_base_id->value) ^ xor_string(m_table_stack_id->value))
//          insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_early_matches_top_attr, m_table_stack_id));
//        insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_late_matches_top_attr, m_table_stack_id));
//      }
    }
//    insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_discrepancy_attr, Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(discrepancy))));

    for(const auto &target_stack : target) {
      Rete::Symbol_Identifier_Ptr_C target_stack_id = m_target_stack_ids[target_stack.begin()->id];

      insert_new_wme(std::make_shared<Rete::WME>(m_target_id, m_stack_attr, target_stack_id));
      for(const auto &block : target_stack) {
        const Rete::Symbol_Identifier_Ptr_C block_id = m_block_ids[block.id];
        insert_new_wme(std::make_shared<Rete::WME>(target_stack_id, m_block_attr, block_id));
      }
      if(std::find(blocks.begin(), blocks.end(), target_stack) == blocks.end())
        insert_new_wme(std::make_shared<Rete::WME>(target_stack_id, m_top_attr, m_block_ids[target_stack.rbegin()->id]));
      const Rete::Symbol_Identifier_Ptr_C target_base_id = m_block_ids[target_stack.begin()->id];
      insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_matches_top_attr, m_table_stack_id));
//      if(get_total_step_count() < 5000) {
//        insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_early_matches_top_attr, m_table_stack_id));
//        if(xor_string(target_base_id->value) ^ xor_string(m_table_stack_id->value))
//          insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_late_matches_top_attr, m_table_stack_id));
//      }
//      else {
//        if(xor_string(target_base_id->value) ^ xor_string(m_table_stack_id->value))
//          insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_early_matches_top_attr, m_table_stack_id));
//        insert_new_wme(std::make_shared<Rete::WME>(target_base_id, m_late_matches_top_attr, m_table_stack_id));
//      }
    }

//    const auto end_time = std::chrono::high_resolution_clock::now();
//    m_feature_generation_time += std::chrono::duration_cast<dseconds>(end_time - start_time).count();

    clear_old_wmes();
  }

  void Agent::update() {
    const auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(env->success())
      m_metastate = Metastate::SUCCESS;
  }

}
