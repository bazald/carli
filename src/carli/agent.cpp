#include "agent.h"

bool Agent::specialize(const action_ptrsc &action, const std::shared_ptr<RL> &general) {
  if(!general->fringe_values || !split_test(general->q_value, general->depth))
    return false;

  /// TODO: choose intelligently again
  auto gen = general->fringe_values->begin();
  for(auto count = random.rand_lt(general->fringe_values->size()); count; --count)
    ++gen;
  auto chosen = *gen;

  for(auto &fringe : *general->fringe_values) {
    if(*fringe->feature < *chosen->feature) {
      chosen = fringe;
    }
  }

//      std::cerr << "Refining : " << gen->first << std::endl;

  if(auto chosen_feature = dynamic_cast<Feature_Ranged *>(chosen->feature.get()))
    expand_fringe(action, general, chosen_feature->axis);

  auto general_action = general->action.lock();
  general_action->detach();
  general_action->set_action([this,action,general](const Rete::Rete_Action &, const Rete::WME_Token &) {
    this->m_next_q_values[action].push_back(general->q_value);
  });
  general_action->reattach();

  return true;
}

void Agent::expand_fringe(const action_ptrsc &action, const std::shared_ptr<RL> &general, const Rete::WME_Token_Index specialization) {
  assert(general->fringe_values);

  typename RL::Fringe_Values leaves;
  for(auto fringe = general->fringe_values->begin(), fend = general->fringe_values->end(); fringe != fend; ) {
    auto fringe_ranged = dynamic_cast<Feature_Ranged *>((*fringe)->feature.get());
    if(fringe_ranged && fringe_ranged->axis == specialization) {
      leaves.push_back(*fringe);
      general->fringe_values->erase(fringe++);
    }
    else
      ++fringe;
  }

  for(auto &leaf : leaves) {
    auto leaf_action = leaf->action.lock();
    leaf_action->detach();

    purge_q_value(leaf->q_value);
    leaf->q_value.delete_and_zero();
    leaf->q_value = new Q_Value(0.0f, Q_Value::Type::UNSPLIT, leaf->depth);
    ++m_q_value_count;

    for(auto &line : leaf->lines)
      m_lines[action].insert(line);
    leaf->lines.clear();

    assert(leaf->depth <= m_split_max);
    auto leaf_ranged = dynamic_cast<Feature_Ranged *>(leaf->feature.get());
    if(leaf->depth < m_split_max && leaf_ranged) {
      leaf->action.lock()->set_action([this,action,leaf](const Rete::Rete_Action &, const Rete::WME_Token &) {
        if(!this->specialize(action, leaf))
          this->m_next_q_values[action].push_back(leaf->q_value);
      });

      leaf->fringe_values = new typename RL::Fringe_Values;

      auto refined = leaf->feature->refined();
      for(auto &refined_feature : refined) {
        auto refined_ranged = dynamic_cast<Feature_Ranged *>(refined_feature);
        if(!refined_ranged)
          continue;
        typename RL::Range range(leaf->range);
        typename RL::Lines lines;
        if(refined_ranged->axis.first == 0) {
          if(!refined_ranged->upper) {
            range.second.first = refined_ranged->bound_upper;
            lines.push_back(typename RL::Line(std::make_pair(range.second.first, range.first.second), std::make_pair(range.second.first, range.second.second)));
          }
          else {
            range.first.first = refined_ranged->bound_lower;
          }
        }
        else {
          if(!refined_ranged->upper) {
            range.second.second = refined_ranged->bound_upper;
            lines.push_back(typename RL::Line(std::make_pair(range.first.first, range.second.second), std::make_pair(range.second.first, range.second.second)));
          }
          else {
            range.first.second = refined_ranged->bound_lower;
          }
        }
        auto rl = std::make_shared<RL>(*this, leaf->depth + 1, range, lines);
        rl->q_value = new Q_Value(0.0, Q_Value::Type::FRINGE, rl->depth);
        rl->feature = refined_feature;
        auto predicate = make_predicate_vc(refined_ranged->predicate(), leaf_ranged->axis, refined_ranged->symbol_constant(), leaf->action.lock()->parent());
        rl->action = make_action_retraction([this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(rl->q_value);
        }, [this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, rl->q_value);
        }, predicate);

        leaf->fringe_values->push_back(rl);
      }

      for(auto &fringe : *general->fringe_values) {
        auto fringe_ranged = dynamic_cast<Feature_Ranged *>(fringe->feature.get());
        if(!fringe_ranged)
          continue;
        typename RL::Range range(fringe->range);
        typename RL::Lines lines;
        if(leaf_ranged->axis.first == 0) {
          range.first.first = leaf->range.first.first;
          range.second.first = leaf->range.second.first;
          for(auto &line : fringe->lines)
            lines.push_back(typename RL::Line(std::make_pair(range.first.first, line.first.second), std::make_pair(range.second.first, line.second.second)));
        }
        else {
          range.first.second = leaf->range.first.second;
          range.second.second = leaf->range.second.second;
          for(auto &line : fringe->lines)
            lines.push_back(typename RL::Line(std::make_pair(line.first.first, range.first.second), std::make_pair(line.second.first, range.second.second)));
        }
        auto rl = std::make_shared<RL>(*this, leaf->depth + 1, range, lines);
        rl->q_value = new Q_Value(0.0, Q_Value::Type::FRINGE, rl->depth);
        rl->feature = fringe_ranged->clone();
        auto predicate = make_predicate_vc(rl->feature->predicate(), fringe_ranged->axis, rl->feature->symbol_constant(), leaf->action.lock()->parent());
        rl->action = make_action_retraction([this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(rl->q_value);
        }, [this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, rl->q_value);
        }, predicate);

        leaf->fringe_values->push_back(rl);
      }

      if(leaf->fringe_values->empty())
        leaf->fringe_values.delete_and_zero();

      leaf->feature.delete_and_zero();
    }

    leaf_action->reattach();
  }

  for(auto &fringe : *general->fringe_values)
    excise_rule(fringe->action.lock());
  general->fringe_values.delete_and_zero();
}

Agent::Agent(const std::shared_ptr<Environment> &environment)
  : m_target_policy([this]()->action_ptrsc{return this->choose_greedy();}),
  m_exploration_policy([this]()->action_ptrsc{return this->choose_epsilon_greedy(m_epsilon);}),
  m_split_test([this](Q_Value * const &q, const size_t &depth)->bool{return this->split_test(q, depth);}),
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
}

Agent::~Agent() {
  Rete_Agent::destroy();
}

void Agent::destroy() {
  Rete_Agent::destroy();
  m_current.zero();
  m_next.zero();
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
  auto found = std::find(m_current_q_value.begin(), m_current_q_value.end(), q_value);
  if(found != m_current_q_value.end())
    m_current_q_value.erase(found);
}

void Agent::purge_q_value_next(const action_ptrsc &action, const tracked_ptr<Q_Value> &q_value) {
  auto &q_values = m_next_q_values[action];
  auto found = std::find(q_values.begin(), q_values.end(), q_value);
  if(found != q_values.end())
    q_values.erase(found);
}

void Agent::print(std::ostream &os) const {
  os << " Agent:\n";

//    print_feature_lists(os);

  os << "  Candidates:\n  ";
  for(const auto &action_value : m_next_q_values)
    std::cerr << ' ' << *action_value.first;

//#if defined(DEBUG_OUTPUT) && defined(DEBUG_OUTPUT_VALUE_FUNCTION)
//    print_value_function(os);
//#endif
}

size_t Agent::get_value_function_size() const {
  return m_q_value_count;
}

void Agent::reset_update_counts() {
//    for(auto &vf : m_value_function) {
//      reset_update_counts_for_trie(vf.second);
//    }
}

void Agent::print_value_function_grid(std::ostream &os) const {
  std::set<typename RL::Line, std::less<typename RL::Line>, Zeni::Pool_Allocator<typename RL::Line>> line_segments;
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

  int counter = m_next_q_values.size();
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
          if(this->m_mean_cabe_queue.size() == m_mean_cabe_queue_size)
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
  size_t count = double();
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

bool Agent::split_test(const tracked_ptr<Q_Value> &q, const size_t &depth) const {
  assert(q);
  assert(q->type == Q_Value::Type::UNSPLIT);

  if(depth < m_split_min) {
    if(q)
      q->type = Q_Value::Type::SPLIT;
    return true;
  }
//    if(depth >= m_split_max)
//      return false;

//    if(!q)
//      return false;
//    if(q->type == Q_Value::Type::SPLIT)
//      return true;

  if(m_value_function_cap && get_value_function_size() >= m_value_function_cap)
    return false;

  if(q->update_count > m_split_update_count &&
     q->pseudoepisode_count > m_split_pseudoepisodes &&
     (m_mean_cabe_queue_size ? m_mean_cabe_queue.mean().outlier_above(q->cabe, m_split_cabe + m_split_cabe_qmult * m_q_value_count)
                             : m_mean_cabe.outlier_above(q->cabe, m_split_cabe + m_split_cabe_qmult * m_q_value_count)))
  {
    q->type = Q_Value::Type::SPLIT;
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
  if(value_list.empty())
    return double();

  double sum = double();
  for(auto &q : value_list) {
#ifdef DEBUG_OUTPUT
      if(action)
        std::cerr << ' ' << q->value /* * q.weight */ << ':' << q->depth << (q->type == Q_Value::Type::FRINGE ? "f" : "");
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

void Agent::print_value_function_grid_set(std::ostream &os, const std::set<typename RL::Line, std::less<typename RL::Line>, Zeni::Pool_Allocator<typename RL::Line>> &line_segments) const {
  for(const auto &line_segment : line_segments)
    os << line_segment.first.first << ',' << line_segment.first.second << '/' << line_segment.second.first << ',' << line_segment.second.second << std::endl;
}

//  void print_update_count_map(std::ostream &os, const std::map<line_segment_type, size_t> &update_counts) const {
//    for(const auto &rect : update_counts)
//      os << rect.first.first.first << ',' << rect.first.first.second << '/' << rect.first.second.first << ',' << rect.first.second.second << '=' << rect.second << std::endl;
//  }

void Agent::merge_value_function_grid_sets(std::set<typename RL::Line, std::less<typename RL::Line>, Zeni::Pool_Allocator<typename RL::Line>> &combination, const std::set<typename RL::Line, std::less<typename RL::Line>, Zeni::Pool_Allocator<typename RL::Line>> &additions) const {
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

std::ostream & operator<<(std::ostream &os, const Agent &agent) {
  agent.print(os);
  return os;
}
