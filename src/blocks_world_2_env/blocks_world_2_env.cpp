#include "blocks_world_2.h"

#include "carli/parser/rete_parser.h"

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
    assert(m_num_target_blocks <= m_num_blocks);
    if(m_num_target_blocks > m_num_blocks) {
      std::cerr << "Fewer blocks available than demanded by the goal configuration!" << std::endl;
      abort();
    }

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
          const auto found = std::find_if(stack.begin(), stack.end(), [](const Environment::Block &block)->bool{return block.id == 2;});
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

  void Environment::init_impl() {
    assert(m_num_blocks > 0);

    std::vector<Block> blocks;
    blocks.reserve(m_num_blocks);
    for(int i = 1; i <= m_num_blocks; ++i)
      blocks.push_back(Block(i, m_random.rand_lt(g_num_colors)));

    std::shuffle(blocks.begin(), blocks.end(), m_random);

    do {
      m_blocks = random_Stacks(blocks);
    } while((m_goal == Goal::STACK || m_goal == Goal::UNSTACK || m_goal == Goal::ON_A_B) && success());

    if(m_goal == Goal::EXACT || m_goal == Goal::COLOR) {
      std::vector<Block> target_blocks;
      target_blocks.reserve(m_num_target_blocks);
      for(int i = 0; i < m_num_target_blocks; ++i)
        target_blocks.push_back(blocks[i]);

      std::shuffle(target_blocks.begin(), target_blocks.end(), m_random);

      do {
        m_target = random_Stacks(target_blocks);
      } while(success());
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

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env)
   : Carli::Agent(env, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(variables, token);})
  {
    std::ostringstream oss;
    for(const auto &stack : dynamic_pointer_cast<const Environment>(get_env())->get_blocks()) {
      for(const auto &block : stack) {
        oss << block.id;
        m_block_ids[block.id] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(oss.str()));
        m_block_names[block.id] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(block.id));
        m_stack_ids[block.id] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string("|") + oss.str()));
        m_target_stack_ids[block.id] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(":") + oss.str()));
        oss.str("");
      }
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
    std::list<Rete::WME_Ptr_C> wmes_current;

    if(env->get_goal() == Environment::Goal::ON_A_B)
      wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[1], m_goal_on_attr, m_block_ids[2]));
    else if(env->get_goal() == Environment::Goal::EXACT) {
      for(const auto &stack : target) {
        Rete::Symbol_Identifier_Ptr_C prev_id = m_table_id;
        for(const auto &block : stack) {
          const Rete::Symbol_Identifier_Ptr_C block_id = m_block_ids[block.id];
          assert(block.id);
          wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_goal_on_attr, prev_id));
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

        wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_height_attr, std::make_shared<Rete::Symbol_Constant_Int>(++height)));

        wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_color_attr, std::make_shared<Rete::Symbol_Constant_Int>(block.color)));

        const double brightness = m_random.frand_lte();
        wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_brightness_attr, std::make_shared<Rete::Symbol_Constant_Float>(brightness)));
        if(brightness >= 0.5)
          wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_glowing_attr, m_true_value));
      }
      max_height = std::max(max_height, height);

      for(const auto &dest_stack : blocks) {
        if(stack == dest_stack)
          continue;
        oss << "move-" << stack.rbegin()->id << "-|" << dest_stack.begin()->id;
        const Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
        const Rete::Symbol_Identifier_Ptr_C dest_id = m_stack_ids[dest_stack.begin()->id];
        oss.str("");
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(dest_id, m_action_in_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_action_out_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_block_attr, m_block_ids[stack.rbegin()->id]));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_block_ids[dest_stack.rbegin()->id]));
      }

      if(stack.size() > 1) {
        oss << "move-" << stack.rbegin()->id << "-TABLE";
        const Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
        oss.str("");
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_table_id, m_action_in_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_action_out_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_block_attr, m_block_ids[stack.rbegin()->id]));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_table_id));
      }
    }

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_blocks_attr, m_blocks_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_target_attr, m_target_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_table_id, m_name_attr, m_table_name));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_blocks_id, m_stack_attr, m_table_stack_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_target_id, m_stack_attr, m_table_stack_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_table_stack_id, m_top_attr, m_table_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_table_stack_id, m_matches_attr, m_table_stack_id));
    if(get_total_step_count() < 5000) {
      wmes_current.push_back(std::make_shared<Rete::WME>(m_table_stack_id, m_early_matches_attr, m_table_stack_id));
      if(xor_string(m_table_stack_id->value))
        wmes_current.push_back(std::make_shared<Rete::WME>(m_table_stack_id, m_late_matches_attr, m_table_stack_id));
    }
    else {
      if(xor_string(m_table_stack_id->value))
        wmes_current.push_back(std::make_shared<Rete::WME>(m_table_stack_id, m_early_matches_attr, m_table_stack_id));
      wmes_current.push_back(std::make_shared<Rete::WME>(m_table_stack_id, m_late_matches_attr, m_table_stack_id));
    }

    for(const auto &stack : blocks) {
      Rete::Symbol_Identifier_Ptr_C stack_id = m_stack_ids[stack.begin()->id];

      wmes_current.push_back(std::make_shared<Rete::WME>(m_blocks_id, m_stack_attr, stack_id));
      for(const auto &block : stack) {
        const Rete::Symbol_Identifier_Ptr_C block_id = m_block_ids[block.id];
        wmes_current.push_back(std::make_shared<Rete::WME>(m_blocks_id, m_block_attr, block_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_block_attr, block_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_name_attr, m_block_names[block.id]));
      }

      wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[stack.rbegin()->id], m_clear_attr, m_true_value));

      wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_top_attr, m_block_ids[stack.rbegin()->id]));
      ///wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_matches_attr, m_table_stack_id));
      if(int64_t(stack.size()) == max_height)
        wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_tallest_attr, m_true_value));
    }

    for(const auto &stack : blocks) {
      int64_t block1_height = 1;
      for(const auto &block1 : stack) {
        const Rete::Symbol_Identifier_Ptr_C block1_id = m_block_ids[block1.id];
        int64_t block2_height = 1;
        for(const auto &block2 : stack) {
          if(block2_height >= block1_height)
            break;
          const Rete::Symbol_Identifier_Ptr_C block2_id = m_block_ids[block2.id];
          if(block1_height == block2_height + 1)
            wmes_current.push_back(std::make_shared<Rete::WME>(block1_id, m_on_attr, block2_id));
          wmes_current.push_back(std::make_shared<Rete::WME>(block1_id, m_above_attr, block2_id));
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
            wmes_current.push_back(std::make_shared<Rete::WME>(block1_id, m_higher_than_attr, block2_id));
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

        wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_matches_attr, target_id));
        if(get_total_step_count() < 5000) {
          wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_early_matches_attr, target_id));
          if(xor_string(stack_id->value) ^ xor_string(target_id->value))
            wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_late_matches_attr, target_id));
        }
        else {
          if(xor_string(stack_id->value) ^ xor_string(target_id->value))
            wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_early_matches_attr, target_id));
          wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_late_matches_attr, target_id));
        }

        if(match.second != target_stack.end()) {
          for(const auto &stack : blocks) {
            for(const auto &block : stack) {
              if(env->get_match_test()(block, *match.second)) {
                const Rete::Symbol_Identifier_Ptr_C block_id = m_block_ids[block.id];
                wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_matches_top_attr, stack_id));
                if(get_total_step_count() < 5000) {
                  wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_early_matches_top_attr, stack_id));
                  if(xor_string(block_id->value) ^ xor_string(stack_id->value))
                    wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_late_matches_top_attr, stack_id));
                }
                else {
                  if(xor_string(block_id->value) ^ xor_string(stack_id->value))
                    wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_early_matches_top_attr, stack_id));
                  wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_late_matches_top_attr, stack_id));
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
            wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[bt->id], m_in_place_attr, m_true_value));
          break;
        }
      }
      assert(!target_stack.empty());
      const Rete::Symbol_Identifier_Ptr_C target_base_id = m_block_ids[target_stack.begin()->id];
      wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_matches_top_attr, m_table_stack_id));
      if(get_total_step_count() < 5000) {
        wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_early_matches_top_attr, m_table_stack_id));
        if(xor_string(target_base_id->value) ^ xor_string(m_table_stack_id->value))
          wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_late_matches_top_attr, m_table_stack_id));
      }
      else {
        if(xor_string(target_base_id->value) ^ xor_string(m_table_stack_id->value))
          wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_early_matches_top_attr, m_table_stack_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_late_matches_top_attr, m_table_stack_id));
      }
    }
//    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_discrepancy_attr, Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(discrepancy))));

    for(const auto &target_stack : target) {
      Rete::Symbol_Identifier_Ptr_C target_stack_id = m_target_stack_ids[target_stack.begin()->id];

      wmes_current.push_back(std::make_shared<Rete::WME>(m_target_id, m_stack_attr, target_stack_id));
      for(const auto &block : target_stack) {
        const Rete::Symbol_Identifier_Ptr_C block_id = m_block_ids[block.id];
        wmes_current.push_back(std::make_shared<Rete::WME>(target_stack_id, m_block_attr, block_id));
      }
      if(std::find(blocks.begin(), blocks.end(), target_stack) == blocks.end())
        wmes_current.push_back(std::make_shared<Rete::WME>(target_stack_id, m_top_attr, m_block_ids[target_stack.rbegin()->id]));
      const Rete::Symbol_Identifier_Ptr_C target_base_id = m_block_ids[target_stack.begin()->id];
      wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_matches_top_attr, m_table_stack_id));
      if(get_total_step_count() < 5000) {
        wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_early_matches_top_attr, m_table_stack_id));
        if(xor_string(target_base_id->value) ^ xor_string(m_table_stack_id->value))
          wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_late_matches_top_attr, m_table_stack_id));
      }
      else {
        if(xor_string(target_base_id->value) ^ xor_string(m_table_stack_id->value))
          wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_early_matches_top_attr, m_table_stack_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(target_base_id, m_late_matches_top_attr, m_table_stack_id));
      }
    }

//    const auto end_time = std::chrono::high_resolution_clock::now();
//    m_feature_generation_time += std::chrono::duration_cast<dseconds>(end_time - start_time).count();

    Rete::Agenda::Locker locker(agenda);
    CPU_Accumulator cpu_accumulator(*this);

    if(get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes")) {
      for(auto wt = m_wmes_prev.begin(), wend = m_wmes_prev.end(); wt != wend; ++wt)
        remove_wme(*wt);
      m_wmes_prev.clear();
    }
    else {
      for(auto wt = m_wmes_prev.begin(), wend = m_wmes_prev.end(); wt != wend; ) {
        const auto found = std::find_if(wmes_current.begin(), wmes_current.end(), [wt](const Rete::WME_Ptr_C &wme_)->bool{return *wme_ == **wt;});
        if(found == wmes_current.end()) {
          remove_wme(*wt);
          m_wmes_prev.erase(wt++);
        }
        else {
          wmes_current.erase(found);
          ++wt;
        }
      }
    }

    for(auto &wme : wmes_current) {
      const auto found = std::find_if(m_wmes_prev.begin(), m_wmes_prev.end(), [wme](const Rete::WME_Ptr_C &wme_)->bool{return *wme_ == *wme;});
      if(found == m_wmes_prev.end()) {
        m_wmes_prev.push_back(wme);
        insert_wme(wme);
      }
    }
  }

  void Agent::update() {
    const auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(env->success())
      m_metastate = Metastate::SUCCESS;
  }

}
