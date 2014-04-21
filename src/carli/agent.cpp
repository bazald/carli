#include "agent.h"

namespace Carli {

  bool Agent::specialize(const Rete::Rete_Action &rete_action, const Rete::WME_Token &token, const std::function<action_ptrsc (const Rete::WME_Token &)> &get_action, const std::shared_ptr<Node_Unsplit> &general) {
    if(!split_test(rete_action, general->q_value))
      return false;
    if(general->q_value->type == Q_Value::Type::SPLIT)
      return true;

    /// TODO: choose intelligently again
    auto gen = general->fringe_values.begin();
    Node_Fringe_Ptr chosen;
    assert(gen != general->fringe_values.end());
    size_t count = 0;
    for(auto &fringe : general->fringe_values) {
      if(fringe->feature->matches(token)) {
        if(!chosen || fringe->q_value->cabe > chosen->q_value->cabe) {
          chosen = fringe;
          count = 1;
        }
        else if(fringe->q_value->cabe == chosen->q_value->cabe && random.frand_lt() < 1.0 / ++count)
          chosen = fringe;
      }
    }

  //#ifdef DEBUG_OUTPUT
  //  std::cerr << "Refining : " << chosen << std::endl;
  //#endif

    expand_fringe(token, get_action, general, chosen->feature.get());

    general->delete_q_value = false;
    general->q_value->type = Q_Value::Type::SPLIT;
    auto node_split = std::make_shared<Node_Split>(*this, general->q_value);
    node_split->action = make_action_retraction([this,get_action,node_split](const Rete::Rete_Action &, const Rete::WME_Token &token) {
      const auto action = get_action(token);
      this->insert_q_value_next(action, node_split->q_value);
    }, [this,get_action,node_split](const Rete::Rete_Action &, const Rete::WME_Token &token) {
      const auto action = get_action(token);
      this->purge_q_value_next(action, node_split->q_value);
    }, general->action->parent()->parent()).get();
    general->destroy();

    return true;
  }

  void Agent::expand_fringe(const Rete::WME_Token &token, const std::function<action_ptrsc (const Rete::WME_Token &)> &get_action, const std::shared_ptr<Node_Unsplit> &general, const Feature * const &specialization) {
    Node_Unsplit::Fringe_Values leaves;
    for(auto fringe = general->fringe_values.begin(), fend = general->fringe_values.end(); fringe != fend; ) {
      if(specialization->compare_axis(*(*fringe)->feature) == 0) {
        leaves.push_back(*fringe);
        general->fringe_values.erase(fringe++);
      }
      else
        ++fringe;
    }

#ifdef DEBUG_OUTPUT
    std::cerr << "Refining:";
    for(auto &leaf : leaves)
      std::cerr << ' ' << *leaf->feature;
    std::cerr << std::endl << "Carrying over detected:";
    for(auto &fringe : general->fringe_values) {
      std::cerr << ' ';
      if(fringe->feature->matches(token))
      std::cerr << '*';
      std::cerr << *fringe->feature;
    }
    std::cerr << std::endl;
#endif

    auto filter_blink = make_filter(*m_wme_blink);
    for(auto &leaf : leaves) {
      auto leaf_node_ranged = std::dynamic_pointer_cast<Node_Fringe_Ranged>(leaf);
      auto leaf_feature_ranged_data = dynamic_cast<Feature_Ranged_Data *>(leaf->feature.get());

  //    if(leaf_node_ranged) {
  //      for(auto &line : leaf_node_ranged->lines)
  //        m_lines[action].insert(line);
  //    }
      assert(!((leaf_node_ranged.get() != nullptr) ^ (leaf_feature_ranged_data != nullptr)));

      Node_Unsplit::Fringe_Values new_fringe;

      if(leaf->q_value->depth <= m_split_max) {
        Node_Unsplit::Fringe_Values node_unsplit_fringe;

        if(leaf->q_value->depth < m_split_max) {
          auto refined = leaf->feature->refined();
          if(leaf_feature_ranged_data) {
            for(auto &refined_feature : refined) {
              auto refined_ranged_data = dynamic_cast<Feature_Ranged_Data *>(refined_feature);
              assert(refined_ranged_data);
              Node_Ranged::Range range(leaf_node_ranged->range);
              Node_Ranged::Lines lines;
              if(m_generate_line_segments) {
                if(refined_ranged_data->axis.first == 0) {
                  if(!refined_ranged_data->upper) {
                    range.second.first = refined_ranged_data->bound_upper;
                    lines.push_back(Node_Ranged::Line(std::make_pair(range.second.first, range.first.second), std::make_pair(range.second.first, range.second.second)));
                  }
                  else {
                    range.first.first = refined_ranged_data->bound_lower;
                  }
                }
                else {
                  if(!refined_ranged_data->upper) {
                    range.second.second = refined_ranged_data->bound_upper;
                    lines.push_back(Node_Ranged::Line(std::make_pair(range.first.first, range.second.second), std::make_pair(range.second.first, range.second.second)));
                  }
                  else {
                    range.first.second = refined_ranged_data->bound_lower;
                  }
                }
              }
              auto rl = std::make_shared<Node_Fringe_Ranged>(*this, leaf->q_value->depth + 1, range, lines);
              rl->feature = refined_feature;
              auto predicate = make_predicate_vc(refined_ranged_data->predicate(), leaf_feature_ranged_data->axis, refined_ranged_data->symbol_constant(), leaf->action->parent());
              rl->action = make_action_retraction([this,get_action,rl](const Rete::Rete_Action &, const Rete::WME_Token &token) {
                const auto action = get_action(token);
                this->insert_q_value_next(action, rl->q_value);
              }, [this,get_action,rl](const Rete::Rete_Action &, const Rete::WME_Token &token) {
                const auto action = get_action(token);
                this->purge_q_value_next(action, rl->q_value);
              }, predicate).get();

              node_unsplit_fringe.push_back(rl);
            }
          }
          else {
            assert(refined.empty());
          }

          for(auto &fringe : general->fringe_values) {
            auto fringe_node_ranged = std::dynamic_pointer_cast<Node_Fringe_Ranged>(fringe);
            auto fringe_feature_ranged_data = dynamic_cast<Feature_Ranged_Data *>(fringe->feature.get());
            auto fringe_action = fringe->action;

            Node_Fringe_Ptr rl;
            Rete::Rete_Node_Ptr new_test;

            if(leaf_feature_ranged_data && fringe_feature_ranged_data) {
              Node_Ranged::Range range(fringe_node_ranged->range);
              Node_Ranged::Lines lines;
              if(m_generate_line_segments) {
                if(leaf_feature_ranged_data->axis.first == 0) {
                  range.first.first = leaf_node_ranged->range.first.first;
                  range.second.first = leaf_node_ranged->range.second.first;
                  for(auto &line : fringe_node_ranged->lines)
                    lines.push_back(Node_Ranged::Line(std::make_pair(range.first.first, line.first.second), std::make_pair(range.second.first, line.second.second)));
                }
                else {
                  range.first.second = leaf_node_ranged->range.first.second;
                  range.second.second = leaf_node_ranged->range.second.second;
                  for(auto &line : fringe_node_ranged->lines)
                    lines.push_back(Node_Ranged::Line(std::make_pair(line.first.first, range.first.second), std::make_pair(line.second.first, range.second.second)));
                }
              }
              rl = std::make_shared<Node_Fringe_Ranged>(*this, leaf->q_value->depth + 1, range, lines);

              new_test = make_predicate_vc(fringe_feature_ranged_data->predicate(), fringe_feature_ranged_data->axis, fringe_feature_ranged_data->symbol_constant(), leaf->action->parent());
            }
            else {
              if(fringe_feature_ranged_data)
                rl = std::make_shared<Node_Fringe_Ranged>(*this, leaf->q_value->depth + 1, fringe_node_ranged->range, fringe_node_ranged->lines);
              else
                rl = std::make_shared<Node_Fringe>(*this, leaf->q_value->depth + 1);

              new_test = make_existential_join(fringe->feature->bindings(), leaf->action->parent(), fringe_action->parent());
            }

            rl->feature = fringe->feature->clone();
            rl->action = make_action_retraction([this,get_action,rl](const Rete::Rete_Action &, const Rete::WME_Token &token) {
              const auto action = get_action(token);
              this->insert_q_value_next(action, rl->q_value);
            }, [this,get_action,rl](const Rete::Rete_Action &, const Rete::WME_Token &token) {
              const auto action = get_action(token);
              this->purge_q_value_next(action, rl->q_value);
            }, new_test).get();

            node_unsplit_fringe.push_back(rl);
          }
        }

        if(node_unsplit_fringe.empty()) {
          auto node_split = std::make_shared<Node_Split>(*this, new Q_Value(0.0, Q_Value::Type::SPLIT, leaf->q_value->depth));
          node_split->action = make_action_retraction([this,get_action,node_split](const Rete::Rete_Action &, const Rete::WME_Token &token) {
            const auto action = get_action(token);
            this->insert_q_value_next(action, node_split->q_value);
          }, [this,get_action,node_split](const Rete::Rete_Action &, const Rete::WME_Token &token) {
            const auto action = get_action(token);
            this->purge_q_value_next(action, node_split->q_value);
          }, leaf->action->parent()).get();
        }
        else {
          auto join_blink = make_existential_join(Rete::WME_Bindings(), leaf->action->parent(), filter_blink);
          auto node_unsplit = std::make_shared<Node_Unsplit>(*this, leaf->q_value->depth);
          node_unsplit->fringe_values.swap(node_unsplit_fringe);
          auto new_action = make_action_retraction([this,get_action,node_unsplit](const Rete::Rete_Action &rete_action, const Rete::WME_Token &token) {
            const auto action = get_action(token);
            if(!this->specialize(rete_action, token, get_action, node_unsplit))
              this->insert_q_value_next(action, node_unsplit->q_value);
          }, [this,get_action,node_unsplit](const Rete::Rete_Action &, const Rete::WME_Token &token) {
            const auto action = get_action(token);
            this->purge_q_value_next(action, node_unsplit->q_value);
          }, join_blink).get();
          node_unsplit->action = new_action;
        }
      }

      leaf->destroy();
    }

    for(auto &fringe : general->fringe_values)
      excise_rule(debuggable_pointer_cast<Rete::Rete_Action>(fringe->action->shared()));
  }

  Agent::Agent(const std::shared_ptr<Environment> &environment)
    : m_target_policy([this]()->action_ptrsc{return this->choose_greedy();}),
    m_exploration_policy([this]()->action_ptrsc{return this->choose_epsilon_greedy(m_epsilon);}),
    m_split_test([this](const Rete::Rete_Action &rete_action, Q_Value * const &q)->bool{return this->split_test(rete_action, q);}),
    m_environment(environment),
    m_credit_assignment(
      m_credit_assignment_code == "all" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_all(value_list);} :
      m_credit_assignment_code == "random" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_random(value_list);} :
      m_credit_assignment_code == "specific" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_specific(value_list);} :
      m_credit_assignment_code == "even" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_evenly(value_list);} :
      m_credit_assignment_code == "inv-update-count" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_inv_update_count(value_list);} :
      m_credit_assignment_code == "inv-log-update-count" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_inv_log_update_count(value_list);} :
      m_credit_assignment_code == "inv-root-update-count" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_inv_root_update_count(value_list);} :
      m_credit_assignment_code == "inv-depth" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_inv_depth(value_list);} :
      m_credit_assignment_code == "epsilon-even-specific" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_epsilon(value_list, &Agent::assign_credit_evenly, &Agent::assign_credit_specific);} :
      m_credit_assignment_code == "epsilon-even-depth" ?
        [this](const Q_Value_List &value_list){return this->assign_credit_epsilon(value_list, &Agent::assign_credit_evenly, &Agent::assign_credit_inv_depth);} :
      std::function<void (const Q_Value_List &)>()
    )
  //#ifdef ENABLE_WEIGHT
  //     ,
  //    m_weight_assignment(
  //      m_weight_assignment_code == "all" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_all(value_list);} :
  //      m_weight_assignment_code == "random" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_random(value_list);} :
  //      m_weight_assignment_code == "specific" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_specific(value_list);} :
  //      m_weight_assignment_code == "even" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_evenly(value_list);} :
  //      m_weight_assignment_code == "inv-update-count" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_update_count(value_list);} :
  //      m_weight_assignment_code == "inv-log-update-count" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_log_update_count(value_list);} :
  //      m_weight_assignment_code == "inv-root-update-count" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_root_update_count(value_list);} :
  //      m_weight_assignment_code == "inv-depth" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_depth(value_list);} :
  //      m_weight_assignment_code == "epsilon-even-specific" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_epsilon(value_list, &Agent::assign_credit_evenly, &Agent::assign_credit_specific);} :
  //      m_weight_assignment_code == "epsilon-even-depth" ?
  //        [this](Q_Value::List * const &value_list){return this->assign_credit_epsilon(value_list, &Agent::assign_credit_evenly, &Agent::assign_credit_inv_depth);} :
  //      std::function<void (Q_Value::List * const &)>()
  //    )
  //#endif
  {
    if(m_on_policy)
      m_target_policy = m_exploration_policy;

    if(m_value_function_map_mode == "in") {
      std::ifstream fin(m_value_function_map_filename);
      std::string line;
      while(std::getline(fin, line))
        m_value_function_map.insert(line);
    }
    else if(m_value_function_map_mode == "out")
      m_value_function_out.open(m_value_function_map_filename);
  }

  Agent::~Agent() {
    Rete_Agent::destroy();
  }

  void Agent::destroy() {
    Rete_Agent::destroy();
  //#ifdef DEBUG_OUTPUT
  //  std::cerr << *this << std::endl;
  //  for(const auto &action_value : m_next_q_values)
  //    sum_value(action_value.first.get(), action_value.second);
  //#endif
    m_next_q_values.clear();
    m_lines.clear();
  }

  void Agent::reset_statistics() {
    m_episode_number = m_episode_number ? 1 : 0;
    m_step_count = 0;
    m_total_reward = 0.0;
  }

  void Agent::init() {
    if(m_metastate != Metastate::NON_TERMINAL)
      ++m_episode_number;
    m_metastate = Metastate::NON_TERMINAL;
    m_step_count = 0;
    m_total_reward = reward_type();

    clear_eligibility_trace();

    generate_features();
    clean_features();

    if(m_metastate == Metastate::NON_TERMINAL)
      m_next = m_exploration_policy();

  //    m_current = m_next;
  //    m_current_q_value = m_next_q_values[m_next];
  }

  Agent::reward_type Agent::act() {
  //    generate_features();

    m_current = m_next;
    m_current_q_value = m_next_q_values[m_next];
    m_current_q_value.sort([](const tracked_ptr<Q_Value> &lhs, const tracked_ptr<Q_Value> &rhs)->bool{return lhs->depth < rhs->depth;});

    assert(m_current);

    const reward_type reward = m_environment->transition(*m_current);

    update();

    if(m_metastate == Metastate::NON_TERMINAL) {
      generate_features();
      clean_features();

      m_next = m_target_policy();
  #ifdef DEBUG_OUTPUT
  //      for(auto &next_q : m_next_q_values)
  //        std::cerr << "   " << *next_q.first << " is an option." << std::endl;
      std::cerr << "   " << *m_next << " is next." << std::endl;
  #endif
      auto &value_best = m_next_q_values[m_next];
      td_update(m_current_q_value, reward, value_best);

      if(!m_on_policy) {
        action_ptrsc next = m_exploration_policy();

        if(*m_next != *next) {
          if(sum_value(nullptr, m_current_q_value) < sum_value(nullptr, m_next_q_values[next]))
            clear_eligibility_trace();
          m_next = next;
        }

  #ifdef DEBUG_OUTPUT
        std::cerr << "   " << *m_next << " is next." << std::endl;
  #endif
      }
    }
    else {
      td_update(m_current_q_value, reward, Q_Value_List());
    }

    m_total_reward += reward;
    ++m_step_count;

    return reward;
  }

  void Agent::purge_q_value(const tracked_ptr<Q_Value> &q_value) {
  //#ifdef DEBUG_OUTPUT
  //  std::cerr << "Purging current value " << q_value << std::endl;
  //#endif
    assert(q_value);
    auto found = std::find(m_current_q_value.begin(), m_current_q_value.end(), q_value);
    if(found != m_current_q_value.end()) {
      m_current_q_value.erase(found);
      assert(std::find(m_current_q_value.begin(), m_current_q_value.end(), q_value) == m_current_q_value.end());
    }
  }

  void Agent::insert_q_value_next(const action_ptrsc &action, const tracked_ptr<Q_Value> &q_value) {
  //#ifdef DEBUG_OUTPUT
  //  std::cerr << "Inserting next value " << q_value << " for action " << *action << std::endl;
  //#endif
    auto &q_values = m_next_q_values[action];
  #ifndef NDEBUG
    if(std::find(q_values.begin(), q_values.end(), q_value) != q_values.end())
      increment_badness();
  #endif
    q_values.push_back(q_value);
  }

  void Agent::purge_q_value_next(const action_ptrsc &action, const tracked_ptr<Q_Value> &q_value) {
  //#ifdef DEBUG_OUTPUT
  //  std::cerr << "Purging next value " << q_value << " for action " << *action << std::endl;
  //#endif
    assert(q_value);
    auto &q_values = m_next_q_values[action];
    auto found = std::find(q_values.begin(), q_values.end(), q_value);
    if(found != q_values.end()) {
      q_values.erase(found);
  #ifndef NDEBUG
      if(std::find(q_values.begin(), q_values.end(), q_value) != q_values.end())
        decrement_badness();
  #endif
    }
  }

  void Agent::print(std::ostream &os) const {
    os << " Agent:\n";

  //    print_feature_lists(os);

    os << "  Candidates:\n  ";
    for(const auto &action_value : m_next_q_values)
      std::cerr << ' ' << *action_value.first;
    os << std::endl;

  //#if defined(DEBUG_OUTPUT) && defined(DEBUG_OUTPUT_VALUE_FUNCTION)
  //    print_value_function(os);
  //#endif
  }

  void Agent::reset_update_counts() {
  //    for(auto &vf : m_value_function) {
  //      reset_update_counts_for_trie(vf.second);
  //    }
  }

  void Agent::print_value_function_grid(std::ostream &os) const {
    std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> line_segments;
    for(auto &action_value : m_next_q_values) {
      const auto &line_segments2 = m_lines.find(action_value.first);
      if(line_segments2 != m_lines.end()) {
        os << *action_value.first << ":" << std::endl;
        print_value_function_grid_set(os, line_segments2->second);
        merge_value_function_grid_sets(line_segments, line_segments2->second);
      }
    }
    os << "all:" << std::endl;
    print_value_function_grid_set(os, line_segments);
  }

  //  void print_update_count_grid(std::ostream &os) const {
  //    std::map<line_segment_type, size_t> update_counts;
  //    for(auto &value : m_value_function) {
  //      os << *value.first << ":" << std::endl;
  //      const auto update_counts2 = generate_update_count_maps(value.second);
  //      merge_update_count_maps(update_counts, update_counts2);
  //      print_update_count_map(os, update_counts2);
  //    }
  //    os << "all:" << std::endl;
  //    print_update_count_map(os, update_counts);
  //  }

  Agent::action_ptrsc Agent::choose_epsilon_greedy(const double &epsilon) {
    if(random.frand_lt() < epsilon)
      return choose_randomly();
    else
      return choose_greedy();
  }

  Agent::action_ptrsc Agent::choose_greedy() {
  #ifdef DEBUG_OUTPUT
    std::cerr << "  choose_greedy" << std::endl;
  #endif

    int32_t count = 0;
    double value = double();
    action_ptrsc action;
    for(const auto &action_q : m_next_q_values) {
      const double value_ = sum_value(action_q.first.get(), action_q.second);

      if(!action || value_ > value) {
        action = action_q.first;
        value = value_;
        count = 1;
      }
      else if(value_ == value && random.rand_lt(++count) == 0)
        action = action_q.first;
    }

    return action;
  }

  Agent::action_ptrsc Agent::choose_randomly() {
  #ifdef DEBUG_OUTPUT
    std::cerr << "  choose_randomly" << std::endl;
  #endif

    int32_t counter = int32_t(m_next_q_values.size());
  //    for(const auto &action_q : m_next_q_values) {
  //      ++counter;
  //      this->get_value(action_, Q_Value::next_offset()); ///< Trigger additional feature generation, as needed
  //    }

    counter = random.rand_lt(counter) + 1;
    action_ptrsc action;
    for(const auto &action_q : m_next_q_values) {
      if(!--counter)
        action = action_q.first;
    }

    return action;
  }

  void Agent::td_update(const Q_Value_List &current, const reward_type &reward, const Q_Value_List &next) {
    assert(!m_badness);

    const double target_next = m_discount_rate * sum_value(nullptr, next);
    const double target_value = reward + target_next;

    double q_old = double();
  #ifdef DEBUG_OUTPUT
    std::cerr << " current :";
  #endif
    for(const auto &q : current) {
      ++q->update_count;

      if(q->type != Q_Value::Type::FRINGE) {
        q_old += q->value /* * q.weight */;
  #ifdef DEBUG_OUTPUT
        std::cerr << ' ' << q;
  #endif
      }
    }
  #ifdef DEBUG_OUTPUT
    std::cerr << std::endl;
    std::cerr << " fringe  :";
    for(const auto &q : current) {
      if(q->type == Q_Value::Type::FRINGE)
        std::cerr << ' ' << q;
    }
    std::cerr << std::endl;
    std::cerr << " next    :";
    for(const auto &q : next) {
      if(q->type != Q_Value::Type::FRINGE)
        std::cerr << ' ' << q;
    }
    std::cerr << std::endl;
  #endif

    m_credit_assignment(current);

    for(const auto &q : current) {
      const double credit = this->m_learning_rate * q->credit;
  //       const double credit_accum = credit + (q.eligibility < 0.0 ? 0.0 : q.eligibility);

      if(credit >= q->eligibility) {
        if(q->eligibility < 0.0) {
          q->eligible.erase_hard();
          q->eligible.insert_before(this->m_eligible);
        }

        q->eligibility_init = true;
        q->eligibility = credit;
      }
    }

  #ifdef DEBUG_OUTPUT
    double q_new = 0.0;
  #endif
    const double delta = target_value - q_old;
  #ifdef ENABLE_WEIGHT
    const bool weight_assignment_all = m_weight_assignment_code == "all";
  #else
    const bool weight_assignment_all = true;
  #endif
    for(Q_Value::List * q_ptr = m_eligible; q_ptr; ) {
      Q_Value &q = **q_ptr;
      q_ptr = q.eligible.next();

      const double ldelta = weight_assignment_all && q.type != Q_Value::Type::FRINGE ? delta : target_value - q.value;
      const double edelta = q.eligibility * ldelta;

      q.value += edelta;
  #ifdef DEBUG_OUTPUT
      q_new += q.value /* * q.weight */;
  #endif

      if(q.type == Q_Value::Type::SPLIT) {
  #ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        this->m_mean_mabe.uncontribute(q.mabe);
  #endif
        if(!m_mean_cabe_queue_size)
          this->m_mean_cabe.uncontribute(q.cabe);
      }
      else {
        const double abs_edelta = std::abs(edelta);

        if(q.eligibility_init) {
          if(q.last_episode_fired != this->m_episode_number) {
            ++q.pseudoepisode_count;
            q.last_episode_fired = this->m_episode_number;
          }
          else if(this->m_step_count - q.last_step_fired > m_pseudoepisode_threshold)
            ++q.pseudoepisode_count;
          q.last_step_fired = this->m_step_count;

  #ifdef WHITESON_ADAPTIVE_TILE
          if(q.type == Q_Value::Type::UNSPLIT) {
            if(abs_edelta < q.minbe) {
              q.minbe = abs_edelta;
              this->m_steps_since_minbe = 0;
            }
            else
              ++this->m_steps_since_minbe;
          }
  #endif
        }

        q.cabe += abs_edelta;
  #ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        q.mabe.set_value(q.cabe / q.update_count);
  #endif

        if(q.type == Q_Value::Type::UNSPLIT && q.update_count > m_contribute_update_count) {
          if(m_mean_cabe_queue_size) {
            if(this->m_mean_cabe_queue.size() == uint64_t(m_mean_cabe_queue_size))
              this->m_mean_cabe_queue.pop();
            this->m_mean_cabe_queue.push(q.cabe);
          }
          else
            this->m_mean_cabe.contribute(q.cabe);

  #ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
          this->m_mean_mabe.contribute(q.mabe);
  #endif
        }
      }

      assert(q.eligibility >= 0.0);
      q.eligibility_init = false;
      q.eligibility *= this->m_eligibility_trace_decay_rate;
      if(q.eligibility < m_eligibility_trace_decay_threshold) {
        if(&q.eligible == this->m_eligible)
          this->m_eligible = this->m_eligible->next();
        q.eligible.erase();
        q.eligibility = -1.0;
      }
    }

  #ifdef DEBUG_OUTPUT
    std::cerr.unsetf(std::ios_base::floatfield);
    std::cerr << " td_update: " << q_old << " <" << m_learning_rate << "= " << reward << " + " << m_discount_rate << " * " << target_next << std::endl;
    std::cerr << "            " << delta << " = " << target_value << " - " << q_old << std::endl;
    std::cerr << "            " << q_new << std::endl;
    std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);

    for(const auto &q : current) {
      if(q->type == Q_Value::Type::UNSPLIT) {
        std::cerr << " updates:  " << q->update_count << std::endl;
        if(m_mean_cabe_queue_size)
          std::cerr << " cabe q:   " << q->cabe << " of " << this->m_mean_cabe_queue.mean() << ':' << this->m_mean_cabe_queue.mean().get_stddev() << std::endl;
        else
          std::cerr << " cabe:     " << q->cabe << " of " << this->m_mean_cabe << ':' << this->m_mean_cabe.get_stddev() << std::endl;
  #ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        std::cerr << " mabe:     " << q->mabe << " of " << this->m_mean_mabe << ':' << this->m_mean_mabe.get_stddev() << std::endl;
  #endif
      }
    }
  #endif
  }

  void Agent::clear_eligibility_trace() {
    for(Q_Value &q : *m_eligible) {
      q.eligibility_init = false;
      q.eligibility = -1.0;
    }

    m_eligible = nullptr;
  }

  //#ifdef ENABLE_WEIGHT
  //  void assign_weight(Q_Value::List * const &value_list) {
  //    m_weight_assignment(value_list);
  //
  //    for(Q_Value &q : *value_list)
  //      q.weight = q.type == Q_Value::Type::FRINGE ? 0.0 : q.credit;
  //  }
  //#endif

  void Agent::assign_credit_epsilon(const Q_Value_List &value_list,
                             void (Agent::*exploration)(const Q_Value_List &),
                             void (Agent::*target)(const Q_Value_List &))
  {
    (this->*exploration)(value_list);

    for(const auto &q : value_list)
      q->t0 = q->credit;

    (this->*target)(value_list);

    const double inverse = 1.0 - this->m_credit_assignment_epsilon;
    for(const auto &q : value_list)
      q->credit = this->m_credit_assignment_epsilon * q->credit + inverse * q->t0;
  }

  void Agent::assign_credit_random(const Q_Value_List &value_list) {
    int32_t count = 0;
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE)
        ++count;
    }

    count = random.rand_lt(count) + 1;

    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE)
        q->credit = --count ? 0.0 : 1.0;
      else
        q->credit = m_fringe_learning_scale;
    }
  }

  void Agent::assign_credit_specific(const Q_Value_List &value_list) {
    tracked_ptr<Q_Value> last;
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 0.0;
        last = q;
      }
      else
        q->credit = m_fringe_learning_scale;
    }

    if(last)
      last->credit = 1.0;
  }

  void Agent::assign_credit_evenly(const Q_Value_List &value_list) {
    double count = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE)
        ++count;
    }

    for(const auto &q : value_list)
      q->credit = q->type == Q_Value::Type::FRINGE ? m_fringe_learning_scale : 1.0 / count;
  }

  void Agent::assign_credit_all(const Q_Value_List &value_list) {
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE)
        q->credit = 1.0;
      else
        q->credit = m_fringe_learning_scale;
    }
  }

  void Agent::assign_credit_inv_update_count(const Q_Value_List &value_list) {
    double sum = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 1.0 / q->update_count;
        sum += q->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void Agent::assign_credit_inv_log_update_count(const Q_Value_List &value_list) {
    double sum = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 1.0 / (std::log(double(q->update_count)) / this->m_credit_assignment_log_base_value + 1.0);
        sum += q->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void Agent::assign_credit_inv_root_update_count(const Q_Value_List &value_list) {
    double sum = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 1.0 / std::pow(double(q->update_count), this->m_credit_assignment_root_value);
        sum += q->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void Agent::assign_credit_inv_depth(const Q_Value_List &value_list) {
    size_t depth = 0;
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE)
        ++depth;
    }

    double sum = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 1.0 / std::pow(2.0, double(--depth));
        sum += q->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void Agent::assign_credit_normalize(const Q_Value_List &value_list, const double &sum) {
    if(m_credit_assignment_normalize || sum > 1.0) {
      for(const auto &q : value_list) {
        if(q->type == Q_Value::Type::FRINGE)
          q->credit = m_fringe_learning_scale;
        else
          q->credit /= sum;
      }
    }
  }

  bool Agent::split_test(const Rete::Rete_Action &rete_action, const tracked_ptr<Q_Value> &q) const {
    assert(q);

    if(q->type == Q_Value::Type::SPLIT || q->depth < m_split_min)
      return true;
  //    if(q->depth >= m_split_max)
  //      return false;

  //    if(!q)
  //      return false;
  //    if(q->type == Q_Value::Type::SPLIT)
  //      return true;

    if(m_value_function_cap && q_value_count >= m_value_function_cap)
      return false;

    if(m_value_function_map_mode == "in") {
      std::ostringstream oss;
      rete_action.output_name(oss);
      return m_value_function_map.find(oss.str()) != m_value_function_map.end();
    }

    if(q->update_count > m_split_update_count &&
       q->pseudoepisode_count > m_split_pseudoepisodes &&
       (m_mean_cabe_queue_size ? m_mean_cabe_queue.mean().outlier_above(q->cabe, m_split_cabe + m_split_cabe_qmult * q_value_count)
                               : m_mean_cabe.outlier_above(q->cabe, m_split_cabe + m_split_cabe_qmult * q_value_count)))
    {
      if(m_value_function_map_mode == "out") {
        rete_action.output_name(m_value_function_out);
        m_value_function_out << std::endl;
      }
      return true;
    }
    else
      return false;
  }

  double Agent::sum_value(const action_type * const &
  #ifdef DEBUG_OUTPUT
                                                     action
  #endif
                                                           , const Q_Value_List &value_list) {
  #ifdef DEBUG_OUTPUT
    if(action) {
      std::cerr.unsetf(std::ios_base::floatfield);
      std::cerr << "   sum_value(" << *action << ") = {";
    }
  #endif

  //    assert((&value_list - static_cast<Q_Value::List *>(nullptr)) > ptrdiff_t(sizeof(Q_Value)));

    double sum = double();
    for(auto &q : value_list) {
  #ifdef DEBUG_OUTPUT
      if(action) {
        std::cerr << ' ' << q->value /* * q.weight */ << ':' << q->depth;
        if(q->type == Q_Value::Type::FRINGE)
          std::cerr << "f[" << *q->node_fringe->feature << ']';
      }
  #endif
      if(q->type != Q_Value::Type::FRINGE)
        sum += q->value /* * q->weight */;
    }

  #ifdef DEBUG_OUTPUT
    if(action) {
      std::cerr << " } = " << sum << std::endl;
      std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
    }
  #endif

    return sum;
  }

  void Agent::print_value_function_grid_set(std::ostream &os, const std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &line_segments) const {
    for(const auto &line_segment : line_segments)
      os << line_segment.first.first << ',' << line_segment.first.second << '/' << line_segment.second.first << ',' << line_segment.second.second << std::endl;
  }

  //  void print_update_count_map(std::ostream &os, const std::map<line_segment_type, size_t> &update_counts) const {
  //    for(const auto &rect : update_counts)
  //      os << rect.first.first.first << ',' << rect.first.first.second << '/' << rect.first.second.first << ',' << rect.first.second.second << '=' << rect.second << std::endl;
  //  }

  void Agent::merge_value_function_grid_sets(std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &combination, const std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &additions) const {
    for(const auto &line_segment : additions)
      combination.insert(line_segment);
  }

  //  void merge_update_count_maps(std::map<line_segment_type, size_t> &combination, const std::map<line_segment_type, size_t> &additions) const {
  //    for(const auto &rect : additions)
  //      combination[rect.first] += rect.second;
  //  }

  //  static void reset_update_counts_for_trie(const feature_trie_type * const &trie) {
  //    for(const feature_trie_type &trie2 : *trie) {
  //      if(trie2.get())
  //        trie2.get()->update_count = 0;
  //      reset_update_counts_for_trie(trie2.get_deeper());
  //    }
  //  }

  //  void collapse_fringe(feature_trie &leaf_fringe, feature_trie head) {
  //    if(m_fringe) {
  //      purge_from_eligibility_trace(leaf_fringe);
  //
  //      feature_trie choice = nullptr;
  //
  //      if(leaf_fringe && leaf_fringe->get() && leaf_fringe->get()->type == Q_Value::Type::FRINGE) {
  //#ifdef DEBUG_OUTPUT
  //        std::cerr.unsetf(std::ios_base::floatfield);
  //        std::cerr << " Collapsing fringe:" << std::endl;
  //        for(feature_trie_type &node : *leaf_fringe) {
  //          std::cerr << "         " << node.get() << ' ' << *node.get_key() << " = " << node->value;
  //          std::cerr << "; update_count = " << node->update_count;
  //          std::cerr << "; cabe = " << node->cabe;
  //          std::cerr << "; mabe = " << node->cabe / node->update_count;
  //          if(auto ranged = dynamic_cast<Feature_Ranged_Data *>(node.get_key().get()))
  //            std::cerr << "; split = " << fabs(ranged->midpt_raw - 0.5);
  //          std::cerr << std::endl;
  //        }
  //#endif
  //
  //        size_t min_depth = std::numeric_limits<size_t>::max();
  //        for(feature_trie_type &node : *leaf_fringe) {
  //          const auto feature = node.get_key().get();
  //          const auto ranged = dynamic_cast<Feature_Ranged_Data *>(feature);
  //          if(ranged->depth < min_depth)
  //            min_depth = ranged->depth;
  //        }
  //
  //        feature_trie_type * prev = nullptr;
  //        Feature_Ranged_Data * ranged_prev = nullptr;
  ////        size_t choice_update_count_min = 0u;
  ////        size_t ranged_update_count_min = 0u;
  ////        size_t choice_axis_in_a_row = 0u;
  ////        size_t axis_in_a_row = 0u;
  //        double choice_delta = 0.0;
  ////        size_t choice_depth = 0;
  //        for(feature_trie_type &node : *leaf_fringe) {
  //          const auto feature = node.get_key().get();
  //          const auto ranged = dynamic_cast<Feature_Ranged_Data *>(feature);
  //
  ////          if(!choice) {
  ////            choice = &node;
  ////            choice_update_count_min = ranged ? ranged->midpt_update_count : 0u;
  ////            choice_axis_in_a_row = ranged ? 1u : 0u;
  ////          }
  //
  //          if(ranged_prev && ranged_prev->depth == min_depth) {
  //            if(ranged && ranged_prev && ranged->axis == ranged_prev->axis &&
  //               ranged->midpt_update_count && ranged_prev->midpt_update_count)
  //            {
  //              const double delta = fabs(prev->get()->value - node.get()->value);
  //              if(delta > choice_delta) {
  //                choice_delta = delta;
  ////                choice_depth = ranged->depth;
  //                choice = &node;
  //              }
  //            }
  //
  ////            if(!ranged || ranged->axis != ranged_prev->axis) {
  ////              if(axis_in_a_row >  choice_axis_in_a_row ||
  ////                (axis_in_a_row == choice_axis_in_a_row && ranged_update_count_min > choice_update_count_min)) {
  ////                choice = prev;
  ////                choice_update_count_min = ranged_update_count_min;
  ////              }
  ////
  ////              if(ranged) {
  ////                ranged_update_count_min = ranged->midpt_update_count;
  ////                axis_in_a_row = 1u;
  ////              }
  ////            }
  ////            else {
  ////              ranged_update_count_min = std::min(ranged_update_count_min, ranged->midpt_update_count);
  ////              ++axis_in_a_row;
  ////            }
  //          }
  ////          else if(ranged) {
  ////            ranged_update_count_min = ranged->midpt_update_count;
  ////            axis_in_a_row = 1u;
  ////          }
  //
  //          prev = &node;
  //          ranged_prev = ranged;
  //        }
  ////        if(axis_in_a_row >  choice_axis_in_a_row ||
  ////          (axis_in_a_row == choice_axis_in_a_row && ranged_update_count_min > choice_update_count_min)) {
  ////          choice = prev;
  ////          choice_update_count_min = ranged_update_count_min;
  ////        }
  //
  //#ifdef DEBUG_OUTPUT
  //        std::cerr << "  Choice " << *choice->get_key() << std::endl;
  //        std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
  //#endif
  //
  //        if(choice)
  //          choice = new feature_trie_type(std::unique_ptr<feature_type>(choice->get_key()->clone()));
  //
  //        leaf_fringe->destroy(leaf_fringe);
  //
  //        if(choice)
  //          choice->map_insert_into_unique(leaf_fringe);
  //      }
  //    }
  //    /// Only necessary when lesioning codebase for debugging purposes (i.e. disabling previous block of code)
  //    else if(leaf_fringe && leaf_fringe->get() && leaf_fringe->get()->type == Q_Value::Type::FRINGE) {
  //      purge_from_eligibility_trace(leaf_fringe);
  //      leaf_fringe->destroy(leaf_fringe);
  //    }
  //  }

  void Agent::clean_features() {
    for(auto it = m_next_q_values.begin(), iend = m_next_q_values.end(); it != iend; ) {
      if(it->second.empty())
        m_next_q_values.erase(it++);
      else
        ++it;
    }
  }

}

std::ostream & operator<<(std::ostream &os, const Carli::Agent &agent) {
  agent.print(os);
  return os;
}
