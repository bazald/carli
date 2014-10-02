#include "agent.h"

//#include "../infinite_mario/infinite_mario.h"

namespace Carli {

  //struct Expiration_Detector {
  //  void operator()(Rete::Rete_Node &rete_node) {
  //    if(!rete_node.data)
  //      return;
  //    const Node_Unsplit * const unsplit_node = dynamic_cast<Node_Unsplit *>(rete_node.data.get());
  //    if(!unsplit_node)
  //      return;
  //    for(const auto &fringe : unsplit_node->fringe_values)
  //      assert(!fringe->rete_action.expired());
  //  }
  //};

  struct Fringe_Collector_All {
    std::unordered_set<Rete::Rete_Action_Ptr> excise;
    std::map<tracked_ptr<Feature>,
      std::map<tracked_ptr<Feature>, Rete::Rete_Node_Ptr, Rete::compare_deref_lt>,
      Rete::compare_deref_memfun_lt<Feature, Feature, &Feature::compare_axis>> features;

    Fringe_Collector_All(const Node_Split_Ptr &split)
      : root(split)
    {
    }

    void operator()(Rete::Rete_Node &rete_node) {
      if(!rete_node.data) {
        assert(!dynamic_cast<Rete::Rete_Action *>(&rete_node));
        return;
      }

      excise.insert(debuggable_pointer_cast<Rete::Rete_Action>(rete_node.shared()));

      if(rete_node.data != root) {
        auto &node = debuggable_cast<Carli::Node &>(*rete_node.data);
        auto &feature = node.q_value->feature;
        if(!feature)
          return;

        const auto found_axis = features.find(feature);
        if(found_axis != features.end()) {
#ifdef DEBUG_OUTPUT
          std::cerr << *found_axis->first << " =axis= " << *feature << std::endl;
#endif
          const int64_t depth_diff = feature->get_depth() - found_axis->second.begin()->first->get_depth();
          if(depth_diff > 0)
            return;
          else if(depth_diff < 0)
            found_axis->second.clear();

          found_axis->second[feature] = rete_node.shared();
        }
        else
          features[feature][feature] = rete_node.shared();
      }
    }

    Node_Ptr root;
  };

  struct Fringe_Collector_Immediate {
    Fringe_Collector_Immediate(const Rete::Rete_Node_Ptr &grandparent_)
      : grandparent(grandparent_)
    {
    }

    void operator()(Rete::Rete_Node &rete_node) {
      if(rete_node.data) {
        const auto fringe = std::dynamic_pointer_cast<Node_Fringe>(rete_node.data);
        if(fringe && fringe->rete_action.lock()->parent_left()->parent_left() == grandparent)
          fringe_values[fringe->q_value->feature.get()].values.push_back(fringe);
      }
    }

    Rete::Rete_Node_Ptr grandparent;
    Fringe_Values fringe_values;
  };

  bool Agent::respecialize(Rete::Rete_Action &rete_action) {
    return false;
//#ifndef NO_COLLAPSE_DETECTION_HACK
//    if(m_experienced_n_positive_rewards_in_a_row)
//      return false;
//#endif
    //if(debuggable_cast<Node *>(rete_action.data.get())->q_value->depth < 3)
    //  return false;
    if(random.rand_lt(1000) || !collapse_rete(rete_action))
      return false;
    else {
//#ifndef NO_COLLAPSE_DETECTION_HACK
//      m_positive_rewards_in_a_row = 0;
//#endif
      return true;
    }
  }

  bool Agent::specialize(Rete::Rete_Action &rete_action) {
    auto &general = debuggable_cast<Node_Unsplit &>(*rete_action.data);

    if(general.q_value->type == Q_Value::Type::SPLIT)
      return true;
#ifndef NDEBUG
    if(general.q_value->depth >= m_split_max)
      return false;
#endif

//     if(general.fringe_values.empty()) {
//       const auto grandparent = rete_action.parent_left()->parent_left();
//       auto fringe_collector = grandparent->visit_preorder(Fringe_Collector_Immediate(grandparent), true);
//       general.fringe_values.swap(fringe_collector.fringe_values);
//       assert(!general.fringe_values.empty());
//     }

    const Fringe_Values::iterator chosen = m_split_criterion(general);

    if(chosen == general.fringe_values.end())
      return false;

  //#ifdef DEBUG_OUTPUT
  //  std::cerr << "Refining : " << chosen << std::endl;
  //#endif

    if(m_output_dot) {
      std::cerr << "Rete size before expansion: " << rete_size() << std::endl;

      std::ostringstream fname;
      g_output_dot_expcon_count = m_output_dot_expanding ? g_output_dot_expcon_count + 1 : 0;
      m_output_dot_expanding = true;
      fname << "pre-expansion-" << (g_output_dot_expcon_count % 4) << ".dot";
      std::ofstream fout(fname.str());
      rete_print(fout);
    }

    auto new_node = general.create_split(*this, general.parent_action.lock());

    expand_fringe(rete_action, new_node->rete_action.lock(), chosen);

    excise_rule(rete_action.get_name(), false);

    if(m_output_dot) {
      std::cerr << "Rete size after expansion: " << rete_size() << std::endl;

      std::ostringstream fname;
      fname << "post-expansion-" << (g_output_dot_expcon_count % 4) << ".dot";
      std::ofstream fout(fname.str());
      rete_print(fout);
    }

    return true;
  }

  void Agent::expand_fringe(Rete::Rete_Action &rete_action, const Rete::Rete_Action_Ptr &parent_action, const Fringe_Values::iterator &specialization)
  {
    auto &general = debuggable_cast<Node_Unsplit &>(*rete_action.data);

    /** Step 1: Collect new leaves from the fringe
     *          They'll have to be modified / replaced, but it's good to separate them from the remainder of the fringe
     */
    std::list<Node_Fringe_Ptr> leaves;
    leaves.swap(specialization->second.values);
    general.fringe_values.erase(specialization);

#ifdef DEBUG_OUTPUT
    std::cerr << "Refining:";
    for(auto &leaf : leaves) {
      std::cerr << ' ';
      if(leaf->rete_action.lock()->is_active())
        std::cerr << '*';
      std::cerr << *leaf->q_value->feature;
    }
    std::cerr << std::endl << "Carrying over detected:";
    for(auto &fringe_axis : general.fringe_values) {
      for(auto &fringe : fringe_axis.second.values) {
        std::cerr << ' ';
        if(fringe->rete_action.lock()->is_active())
          std::cerr << '*';
        std::cerr << *fringe->q_value->feature;
      }
    }
    std::cerr << std::endl;
#endif

    /** Step 2: For each new leaf...
     *          ...create it, clone remaining fringe entries below the new leaf, and destroy the old fringe node
     */
    for(auto &leaf : leaves) {
      if(leaf->q_value->depth <= m_split_max) {
        bool terminal = leaf->q_value->depth == m_split_max;
        std::vector<Carli::Feature *> refined;
        if(!terminal) {
          refined = leaf->q_value->feature->refined();
          terminal = refined.empty() && general.fringe_values.empty();
        }

        /** Step 2.1: Create a new split/unsplit node depending on the existence of a new fringe. */
        if(terminal)
          leaf->create_split(*this, parent_action);
        else {
          const auto node_unsplit = leaf->create_unsplit(*this, parent_action);

          /** Step 2.2: Create new ranged fringe nodes if the new leaf is refineable. */
          for(auto &refined_feature : refined)
            node_unsplit->create_fringe(*this, *node_unsplit, refined_feature);

          /** Step 2.3 Create new fringe nodes. **/
          for(auto &fringe_axis : general.fringe_values) {
            for(auto &fringe : fringe_axis.second.values)
              fringe->create_fringe(*this, *node_unsplit, nullptr);
          }
        }
      }

      /** Step 2.4: Untrack the old fringe node. */
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
      m_mean_matde.uncontribute(leaf->q_value->matde);
#endif
      if(!m_mean_catde_queue_size)
        m_mean_catde.uncontribute(leaf->q_value->catde);

      /** Step 2.5: Destroy the old leaf node. */
      excise_rule(leaf->rete_action.lock()->get_name(), false);
    }

    /** Step 3: Excise all old fringe rules from the system */
    for(auto &fringe_axis : general.fringe_values)
      for(auto &fringe : fringe_axis.second.values)
        excise_rule(fringe->rete_action.lock()->get_name(), false);
  }

  bool Agent::collapse_rete(Rete::Rete_Action &rete_action) {
    assert(rete_action.data);
    auto split = debuggable_pointer_cast<Node_Split>(rete_action.data);
    assert(split);

//#ifndef NDEBUG
//    rete_action.visit_preorder(Expiration_Detector(), false);
//#endif

    auto fringe_collector = rete_action.parent_left()->visit_preorder(Fringe_Collector_All(split), true);

    if(fringe_collector.features.empty())
      return false;

    if(m_output_dot) {
      std::cerr << "Rete size before collapse: " << rete_size() << std::endl;

      std::ostringstream fname;
      g_output_dot_expcon_count = m_output_dot_expanding ? 0 : g_output_dot_expcon_count + 1;
      m_output_dot_expanding = false;
      fname << "pre-collapse-" << (g_output_dot_expcon_count % 4) << ".dot";
      std::ofstream fout(fname.str());
      rete_print(fout);
    }

#ifdef DEBUG_OUTPUT
    std::cerr << "Features: ";
    for(const auto &feature_axis : fringe_collector.features)
      for(const auto &feature_node : feature_axis.second)
        std::cerr << ' ' << *feature_node.first;
    std::cerr << std::endl;
#endif

    /// Make new unsplit node
    const auto unsplit = split->create_unsplit(*this, split->parent_action.lock());
#ifdef DEBUG_OUTPUT
    std::cerr << "Collapsing " << split << " to " << unsplit << std::endl;
#endif

    /// Preserve some learning
    unsplit->q_value->value = split->q_value->value;

    /// Make new fringe
    Fringe_Values node_unsplit_fringe;
    for(const auto &feature_axis : fringe_collector.features) {
      for(const auto &feature_node : feature_axis.second) {
        auto &node = debuggable_cast<Node &>(*feature_node.second->data.get());

        auto new_fringe = node.create_fringe(*this, *unsplit, nullptr);

//        std::cerr << "Collapsing: ";
//        node.rete_action.lock()->output_name(std::cerr, 3);
//        std::cerr << std::endl;
//
//        std::cerr << "Creating  : ";
//        new_fringe->rete_action.lock()->output_name(std::cerr, 3);
//        std::cerr << std::endl;
      }
    }

    fringe_collector.features.clear();

//    std::cerr << "Collapsing: ";
//    rete_action.output_name(std::cerr, 3);
//    std::cerr << std::endl;
//
//    std::cerr << "Creating  : ";
//    unsplit->rete_action.lock()->output_name(std::cerr, 3);
//    std::cerr << std::endl;

#ifdef DEBUG_OUTPUT
    std::cerr << "Excising " << fringe_collector.excise.size() << " actions." << std::endl;
#endif

    for(const auto &excise : fringe_collector.excise)
      excise_rule(excise->get_name(), false);
    fringe_collector.excise.clear();

    if(m_output_dot) {
      std::cerr << "Rete size after collapse: " << rete_size() << std::endl;

      std::ostringstream fname;
      fname << "post-collapse-" << (g_output_dot_expcon_count % 4) << ".dot";
      std::ofstream fout(fname.str());
      rete_print(fout);
    }

//#ifndef NDEBUG
//    unsplit->rete_action.lock()->visit_preorder(Expiration_Detector(), false);
//#endif

    return true;
  }

  Agent::Agent(const std::shared_ptr<Environment> &environment, const std::function<Carli::Action_Ptr_C (const Rete::Variable_Indices &variables, const Rete::WME_Token &token)> &get_action_)
    : get_action(get_action_),
    m_target_policy([this]()->Action_Ptr_C{return this->choose_greedy(nullptr).first;}),
    m_exploration_policy([this]()->Action_Ptr_C{return this->choose_epsilon_greedy(m_epsilon, nullptr);}),
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

    if(m_split_test == "catde") {
      m_split_criterion = [this](Node_Unsplit &general)->Fringe_Values::iterator{
        return this->split_test_catde(general);
      };
    }
    else if(m_split_test == "policy") {
      m_split_criterion = [this](Node_Unsplit &general)->Fringe_Values::iterator{
        return this->split_test_policy(general);
      };
    }
    else if(m_split_test == "value") {
      m_split_criterion = [this](Node_Unsplit &general)->Fringe_Values::iterator{
        return this->split_test_value(general);
      };
    }
    else
      abort();

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
    excise_all();
  }

  void Agent::destroy() {
    const std::string rules_out_file = dynamic_cast<const Option_String &>(Options::get_global()["rules-out"]).get_value();
    if(!rules_out_file.empty()) {
      std::ofstream rules_out(rules_out_file.c_str());
      rete_print_rules(rules_out);
    }

    excise_all();
  //#ifdef DEBUG_OUTPUT
  //  std::cerr << *this << std::endl;
  //  for(const auto &action_value : m_next_q_values)
  //    sum_value(action_value.first.get(), action_value.second);
  //#endif
    m_next_q_values.clear();
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

    generate_all_features();

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

    if(!m_current) {
      std::cerr << "No action selected. Terminating." << std::endl;
      abort();
    }

    const reward_type reward = m_environment->transition(*m_current);

#ifndef NO_COLLAPSE_DETECTION_HACK
    if(reward > 0.0) {
      if(++m_positive_rewards_in_a_row > 30)
        m_experienced_n_positive_rewards_in_a_row = true;
    }
    else
      m_positive_rewards_in_a_row = 0;
#endif

    update();

    if(m_metastate == Metastate::NON_TERMINAL) {
      generate_all_features();

      m_next = m_target_policy();
#ifdef DEBUG_OUTPUT
  //      for(auto &next_q : m_next_q_values)
  //        std::cerr << "   " << *next_q.first << " is an option." << std::endl;
      std::cerr << "   " << *m_next << " is next." << std::endl;
#endif
      auto &value_best = m_next_q_values[m_next];
      td_update(m_current_q_value, reward, value_best);

      if(!m_on_policy) {
        Action_Ptr_C next = m_exploration_policy();

        if(*m_next != *next) {
          if(sum_value(nullptr, m_current_q_value, nullptr) < sum_value(nullptr, m_next_q_values[next], nullptr))
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

  Rete::Rete_Action_Ptr Agent::make_standard_action(const Rete::Rete_Node_Ptr &parent, const std::string &name, const bool &user_command, const Rete::Variable_Indices_Ptr_C &variables) {
    return make_action_retraction(name, user_command,
      [this](const Rete::Rete_Action &action, const Rete::WME_Token &token) {
        debuggable_cast<Node *>(action.data.get())->action(*this, token);
      }, [this](const Rete::Rete_Action &action, const Rete::WME_Token &token) {
        debuggable_cast<Node *>(action.data.get())->retraction(*this, token);
      }, parent, variables);
  }

  Rete::Rete_Action_Ptr Agent::make_standard_fringe(const Rete::Rete_Node_Ptr &parent, const std::string &name, const bool &user_command, const Node_Unsplit_Ptr &root_action_data, const tracked_ptr<Feature> &feature, const Rete::Variable_Indices_Ptr_C &variables) {
    auto new_leaf = make_standard_action(parent, name, user_command, variables);
    auto new_leaf_data = std::make_shared<Node_Fringe>(*this, root_action_data->rete_action.lock(), new_leaf, 2, feature);
    new_leaf->data = new_leaf_data;
    root_action_data->fringe_values[new_leaf_data->q_value->feature.get()].values.push_back(new_leaf_data);
    return new_leaf;
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

  void Agent::insert_q_value_next(const Action_Ptr_C &action, const tracked_ptr<Q_Value> &q_value) {
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

  void Agent::purge_q_value_next(const Action_Ptr_C &action, const tracked_ptr<Q_Value> &q_value) {
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

  void Agent::purge_q_value_eligible(const tracked_ptr<Q_Value> &q_value) {
    q_value->eligible.erase_from(m_eligible);
  }

  void Agent::print(std::ostream &os) const {
    os << " Agent:\n";

  //    print_feature_lists(os);

    os << "  Candidates:\n  ";
    for(const auto &action_value : m_next_q_values)
      os << ' ' << *action_value.first;
    os << std::endl;

  //#if defined(DEBUG_OUTPUT) && defined(DEBUG_OUTPUT_VALUE_FUNCTION)
  //    print_value_function(os);
  //#endif
  }

  //void Agent::print_value_function_grid(std::ostream &os) const {
  //  std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> line_segments;
  //  for(auto &action_value : m_next_q_values) {
  //    const auto &line_segments2 = m_lines.find(action_value.first);
  //    if(line_segments2 != m_lines.end()) {
  //      os << *action_value.first << ":" << std::endl;
  //      print_value_function_grid_set(os, line_segments2->second);
  //      merge_value_function_grid_sets(line_segments, line_segments2->second);
  //    }
  //  }
  //  os << "all:" << std::endl;
  //  print_value_function_grid_set(os, line_segments);
  //}

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

  void Agent::visit_increment_depth() {
    std::function<void (Rete::Rete_Node &)> visitor = [](Rete::Rete_Node &rete_node) {
      if(rete_node.data) {
        auto &node = debuggable_cast<Carli::Node &>(*rete_node.data);
        ++node.q_value->depth;
      }
    };

    visit_preorder(visitor, true);
  }

  void Agent::visit_reset_update_count() {
    std::function<void (Rete::Rete_Node &)> visitor = [](Rete::Rete_Node &rete_node) {
      if(rete_node.data) {
        auto &node = debuggable_cast<Carli::Node &>(*rete_node.data);
        node.q_value->update_count = 0;
      }
    };

    visit_preorder(visitor, true);
  }

  Carli::Action_Ptr_C Agent::choose_epsilon_greedy(const double &epsilon, const Feature * const &axis) {
    if(random.frand_lt() < epsilon)
      return choose_randomly();
    else
      return choose_greedy(axis).first;
  }

  std::pair<Action_Ptr_C, double> Agent::choose_greedy(const Feature * const &axis) {
#ifdef DEBUG_OUTPUT
    std::cerr << "  choose_greedy(";
    if(axis)
      axis->print_axis(std::cerr);
    std::cerr << ')' << std::endl;
#endif

    int32_t count = 0;
    double value = double();
    Action_Ptr_C action;
    double distance = double();
    for(const auto &action_q : m_next_q_values) {
      const double value_ = sum_value(action_q.first.get(), action_q.second, axis);

      if(!action || value_ > value) {
        distance = value_ - value;
        action = action_q.first;
        value = value_;
        count = 1;
      }
      else if(value_ == value) {
        distance = 0;
        if(random.rand_lt(++count) == 0)
          action = action_q.first;
      }
    }

    return std::make_pair(action, distance);
  }

  Carli::Action_Ptr_C Agent::choose_randomly() {
#ifdef DEBUG_OUTPUT
    std::cerr << "  choose_randomly" << std::endl;
#endif

    int32_t counter = int32_t(m_next_q_values.size());
  //    for(const auto &action_q : m_next_q_values) {
  //      ++counter;
  //      this->get_value(action_, Q_Value::next_offset()); ///< Trigger additional feature generation, as needed
  //    }

    counter = random.rand_lt(counter) + 1;
    Action_Ptr_C action;
    for(const auto &action_q : m_next_q_values) {
      if(!--counter)
        action = action_q.first;
    }

    return action;
  }

  void Agent::td_update(const Q_Value_List &current, const reward_type &reward, const Q_Value_List &next) {
    assert(!m_badness);

    const double target_next = m_discount_rate * sum_value(nullptr, next, nullptr);
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
        std::cerr << ' ' << q->value;
#endif
      }
    }
#ifdef DEBUG_OUTPUT
    std::cerr << std::endl;
    std::cerr << " fringe  :";
    for(const auto &q : current) {
      if(q->type == Q_Value::Type::FRINGE)
        std::cerr << ' ' << q->value;
    }
    std::cerr << std::endl;
    std::cerr << " next    :";
    for(const auto &q : next) {
      if(q->type != Q_Value::Type::FRINGE)
        std::cerr << ' ' << q->value;
    }
    std::cerr << std::endl;
#endif

    m_credit_assignment(current);

    for(const auto &q : current) {
      const double credit = this->m_learning_rate * q->credit;
  //       const double credit_accum = credit + (q.eligibility < 0.0 ? 0.0 : q.eligibility);

      if(credit >= q->eligibility) {
        if(q->eligibility < 0.0)
          q->eligible.insert_before(m_eligible);

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
    for(Q_Value::List::list_pointer_type q_ptr = m_eligible; q_ptr; ) {
      Q_Value &q = **q_ptr;
      const auto next = q_ptr->next();

      const double ldelta = weight_assignment_all && q.type != Q_Value::Type::FRINGE ? delta : target_value - q.value;
      const double edelta = q.eligibility * ldelta;

      q.value += edelta;
#ifdef DEBUG_OUTPUT
      q_new += q.value /* * q.weight */;
#endif

      if(q.type == Q_Value::Type::FRINGE) {
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

        q.catde += abs_edelta;
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        q.matde.set_value(q.catde / q.update_count);
#endif

        if(q.update_count > m_contribute_update_count) {
          if(m_mean_catde_queue_size) {
            if(this->m_mean_catde_queue.size() == uint64_t(m_mean_catde_queue_size))
              this->m_mean_catde_queue.pop();
            this->m_mean_catde_queue.push(q.catde);
          }
          else
            this->m_mean_catde.contribute(q.catde);

#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
          this->m_mean_matde.contribute(q.matde);
#endif
        }
      }

      assert(q.eligibility >= 0.0);
      q.eligibility_init = false;
      q.eligibility *= m_eligibility_trace_decay_rate;
      if(q.eligibility < m_eligibility_trace_decay_threshold) {
        q.eligible.erase_from(m_eligible);
        q.eligibility = -1.0;
      }

      q_ptr = next;
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
        if(m_mean_catde_queue_size)
          std::cerr << " catde q:   " << q->catde << " of " << this->m_mean_catde_queue.mean() << ':' << this->m_mean_catde_queue.mean().get_stddev() << std::endl;
        else
          std::cerr << " catde:     " << q->catde << " of " << this->m_mean_catde << ':' << this->m_mean_catde.get_stddev() << std::endl;
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        std::cerr << " matde:     " << q->matde << " of " << this->m_mean_matde << ':' << this->m_mean_matde.get_stddev() << std::endl;
#endif
      }
    }
#endif
  }

  void Agent::clear_eligibility_trace() {
    for(auto q = m_eligible->begin(), next = q; q != m_eligible->end(); q = next) {
      ++next;
      q->eligible.erase_hard();
      q->eligibility_init = false;
      q->eligibility = -1.0;
    }

    m_eligible.zero();
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

  Fringe_Values::iterator Agent::split_test_catde(Node_Unsplit &general) {
    assert(general.q_value);
    assert(general.q_value->type != Q_Value::Type::SPLIT);

    /// Must obey the value function cap unless below the minimum split depth
    if(general.q_value->depth >= m_split_min && m_value_function_cap && q_value_count >= m_value_function_cap)
      return general.fringe_values.end();

    if(m_value_function_map_mode == "in") {
      for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
        for(auto &fringe : fringe_axis->second.values) {
          std::ostringstream oss;
          fringe->rete_action.lock()->output_name(oss, -1);
          if(m_value_function_map.find(oss.str()) != m_value_function_map.end())
            return fringe_axis;
        }
      }
    }

    /// Classic CATDE criterion
//    if(!(m_mean_catde_queue_size ? m_mean_catde_queue.mean() : m_mean_catde).outlier_above(general.q_value->catde, m_split_catde + m_split_catde_qmult * q_value_count))
//      return nullptr;

    assert(general.q_value->depth < m_split_max);
    assert(!general.fringe_values.empty());
    Fringe_Values::iterator chosen_axis = general.fringe_values.end();
    Node_Fringe_Ptr chosen;
    size_t count = 0;
    size_t matches = 0;
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
      for(auto &fringe : fringe_axis->second.values) {
        if(!fringe->rete_action.lock()->is_active())
          continue;
        ++matches;

        if(general.q_value->depth < m_split_min ||
          (fringe->q_value->pseudoepisode_count > m_split_pseudoepisodes &&
          (fringe->q_value->update_count > m_split_update_count &&
          (m_mean_catde_queue_size ? m_mean_catde_queue.mean() : m_mean_catde).outlier_above(fringe->q_value->catde, m_split_catde + m_split_catde_qmult * q_value_count))))
        {
//#ifndef NDEBUG
//          std::cerr << " matches: " << *fringe->q_value->feature << std::endl;
//#endif
          const int64_t depth_diff = chosen ? fringe->q_value->feature->get_depth() - chosen->q_value->feature->get_depth() : 0;
          const double catde_diff = chosen ? fringe->q_value->catde - chosen->q_value->catde : 0.0;
          if(!chosen || depth_diff < 0 || (depth_diff == 0 && catde_diff < 0))
            count = 1;
          else if(depth_diff > 0 || catde_diff > 0 || random.frand_lt() >= 1.0 / ++count)
            continue;

          chosen_axis = fringe_axis;
          chosen = fringe;
        }
//#ifndef NDEBUG
//        else
//          std::cerr << "!matches: " << *fringe->q_value->feature << std::endl;
//#endif
      }
    }

    //for(auto &fringe : general.fringe_values) {
    //  const auto button = dynamic_cast<const Mario::Feature_Button * const>(fringe->q_value->feature.get());
    //  if(button && button->axis.first == Mario::Feature_Button::OUT_JUMP)
    //    chosen = fringe;
    //}

    //for(auto &fringe : general.fringe_values) {
    //  const auto button = dynamic_cast<const Mario::Feature_Button * const>(fringe->q_value->feature.get());
    //  if(button && button->axis.first == Mario::Feature_Button::OUT_DPAD)
    //    chosen = fringe;
    //}

    if(!chosen) {
      if(!matches) {
        std::cerr << "WARNING: No feature in the fringe matches the current token!" << std::endl;
#if !defined(NDEBUG) && defined(_WINDOWS)
        __debugbreak();
#elif !defined(NDEBUG)
        assert(false);
#endif
      }

      return general.fringe_values.end();
    }

    if(m_value_function_map_mode == "out") {
      chosen->rete_action.lock()->output_name(m_value_function_out, -1);
      m_value_function_out << std::endl;
    }

    return chosen_axis;
  }

  Fringe_Values::iterator Agent::split_test_policy(Node_Unsplit &general) {
    assert(general.q_value);
    assert(general.q_value->type != Q_Value::Type::SPLIT);

    /// Must obey the value function cap unless below the minimum split depth
    if(general.q_value->depth >= m_split_min && m_value_function_cap && q_value_count >= m_value_function_cap)
      return general.fringe_values.end();

    assert(general.q_value->depth < m_split_max);
    assert(!general.fringe_values.empty());
    size_t matches = 0;
    std::unordered_set<tracked_ptr<Feature>> touched;
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
      for(auto &fringe : fringe_axis->second.values) {
        if(!fringe->rete_action.lock()->is_active())
          continue;
        ++matches;

        if(general.q_value->depth >= m_split_min &&
          (fringe->q_value->pseudoepisode_count <= m_split_pseudoepisodes ||
           fringe->q_value->update_count <= m_split_update_count))
        {
          return general.fringe_values.end(); ///< Wait to gather more data
        }

        touched.insert(fringe_axis->first);
      }
    }

    Fringe_Values::iterator chosen_axis = general.fringe_values.end();
    auto greedy_action = choose_greedy(nullptr);
    greedy_action.second = 0; ///< If there's anything new, we want to split, even if the difference is small.
    size_t count = 1;
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
      const auto found = touched.find(fringe_axis->first);
      if(found == touched.end())
        continue;

      const auto greedy_axis = choose_greedy(fringe_axis->first);
      if(*greedy_action.first != *greedy_axis.first) {
//        std::cerr << "Greedy mismatch for axis: ";
//        fringe_axis->first->print_axis(std::cerr);
//        std::cerr << std::endl;

        const int64_t depth_diff = chosen_axis != general.fringe_values.end() ? fringe_axis->first->get_depth() - chosen_axis->first->get_depth() : 0;
        if(depth_diff < 0 || (depth_diff == 0 && greedy_axis.second > greedy_action.second))
          count = 1;
        else if(depth_diff > 0 || greedy_axis.second < greedy_action.second || random.frand_lt() >= 1.0 / ++count)
          continue;

        chosen_axis = fringe_axis;
        greedy_action = greedy_axis;
      }
    }

    if(chosen_axis == general.fringe_values.end()) {
      if(!matches) {
        std::cerr << "WARNING: No feature in the fringe matches the current token!" << std::endl;
#if !defined(NDEBUG) && defined(_WINDOWS)
        __debugbreak();
#elif !defined(NDEBUG)
        assert(false);
#endif
      }

      return general.fringe_values.end();
    }

    return chosen_axis;
  }

  Fringe_Values::iterator Agent::split_test_value(Node_Unsplit &general) {
    assert(general.q_value);
    assert(general.q_value->type != Q_Value::Type::SPLIT);

    /// Must obey the value function cap unless below the minimum split depth
    if(general.q_value->depth >= m_split_min && m_value_function_cap && q_value_count >= m_value_function_cap)
      return general.fringe_values.end();

    assert(general.q_value->depth < m_split_max);
    assert(!general.fringe_values.empty());
    size_t matches = 0;
    std::unordered_set<tracked_ptr<Feature>> touched;
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
      for(auto &fringe : fringe_axis->second.values) {
        if(!fringe->rete_action.lock()->is_active())
          continue;
        ++matches;

        if(general.q_value->depth >= m_split_min &&
          (fringe->q_value->pseudoepisode_count <= m_split_pseudoepisodes ||
           fringe->q_value->update_count <= m_split_update_count))
        {
          return general.fringe_values.end(); ///< Wait to gather more data
        }

        touched.insert(fringe_axis->first);
      }
    }

    Fringe_Values::iterator chosen_axis = general.fringe_values.end();
    size_t count = 0;
    int64_t depth_min = std::numeric_limits<int64_t>::max();
    double delta_max = std::numeric_limits<double>::lowest();
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
      const auto found = touched.find(fringe_axis->first);
      if(found == touched.end())
        continue;

      double lowest = std::numeric_limits<double>::max();
      double highest = std::numeric_limits<double>::lowest();
      for(auto &fringe : fringe_axis->second.values) {
        lowest = std::min(lowest, fringe->q_value->value);
        highest = std::max(highest, fringe->q_value->value);
      }
      fringe_axis->second.value_delta_max = m_learning_rate * (highest - lowest) + (1 - m_learning_rate) * fringe_axis->second.value_delta_max;

      //++fringe_axis->first->value_delta_update_count;
      //if(fringe_axis->first->value_delta_update_count <= m_split_update_count)
      //  continue;

      int64_t depth_diff = fringe_axis->first->get_depth() - depth_min;
      double delta_diff = delta_max - fringe_axis->second.value_delta_max;
      if(chosen_axis == general.fringe_values.end() || depth_diff < 0 || (depth_diff == 0 && delta_diff < 0))
        count = 1;
      else if(depth_diff > 0 || delta_diff > 0 || random.frand_lt() >= 1.0 / ++count)
        continue;

      chosen_axis = fringe_axis;
      depth_min = fringe_axis->first->get_depth();
      delta_max = fringe_axis->second.value_delta_max;
    }

    if(chosen_axis == general.fringe_values.end()) {
      if(!matches) {
        std::cerr << "WARNING: No feature in the fringe matches the current token!" << std::endl;
#if !defined(NDEBUG) && defined(_WINDOWS)
        __debugbreak();
#elif !defined(NDEBUG)
        assert(false);
#endif
      }

      return general.fringe_values.end();
    }

    return chosen_axis;
  }

  double Agent::sum_value(const action_type * const &
#ifdef DEBUG_OUTPUT
                                                     action
#endif
                                                           , const Q_Value_List &value_list, const Feature * const &axis) {
#ifdef DEBUG_OUTPUT
    if(action) {
      std::cerr.unsetf(std::ios_base::floatfield);
      std::cerr << "   sum_value(" << *action << ") = {";
    }
#endif

  //    assert((&value_list - static_cast<Q_Value::List *>(nullptr)) > ptrdiff_t(sizeof(Q_Value)));

    double sum = double();
    size_t touched = 0u;
    for(auto &q : value_list) {
#ifdef DEBUG_OUTPUT
      if(action) {
        std::cerr << ' ' << q->value /* * q.weight */ << ':' << q->depth;
        if(q->type == Q_Value::Type::FRINGE)
          std::cerr << 'f';
        if(q->feature)
          std::cerr << ':' << *q->feature;
      }
#endif
      if(axis ? (q->feature && q->feature->compare_axis(*axis) == 0) : q->type != Q_Value::Type::FRINGE) {
        sum += q->value /* * q->weight */;
        ++touched;
      }
    }

//#ifdef DEBUG_OUTPUT
//    std::cerr << std::endl << "Touched ";
//    if(axis)
//      std::cerr << *axis;
//    else
//      std::cerr << 0;
//    std::cerr << '?' << std::endl;
//#endif
    assert(touched || value_list.empty());

#ifdef DEBUG_OUTPUT
    if(action) {
      std::cerr << " } = " << sum << std::endl;
      std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
    }
#endif

    return sum;
  }

  //void Agent::print_value_function_grid_set(std::ostream &os, const std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &line_segments) const {
  //  for(const auto &line_segment : line_segments)
  //    os << line_segment.first.first << ',' << line_segment.first.second << '/' << line_segment.second.first << ',' << line_segment.second.second << std::endl;
  //}

  //  void print_update_count_map(std::ostream &os, const std::map<line_segment_type, size_t> &update_counts) const {
  //    for(const auto &rect : update_counts)
  //      os << rect.first.first.first << ',' << rect.first.first.second << '/' << rect.first.second.first << ',' << rect.first.second.second << '=' << rect.second << std::endl;
  //  }

  //void Agent::merge_value_function_grid_sets(std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &combination, const std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &additions) const {
  //  for(const auto &line_segment : additions)
  //    combination.insert(line_segment);
  //}

  //  void merge_update_count_maps(std::map<line_segment_type, size_t> &combination, const std::map<line_segment_type, size_t> &additions) const {
  //    for(const auto &rect : additions)
  //      combination[rect.first] += rect.second;
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
  //          std::cerr << "; catde = " << node->catde;
  //          std::cerr << "; matde = " << node->catde / node->update_count;
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

  void Agent::generate_all_features() {
    m_nodes_active.swap(m_nodes_activating);

    generate_features();

    while(!m_nodes_activating.empty()) {
      const auto node = m_nodes_activating.front();
      m_nodes_active.push_back(node);
      m_nodes_activating.pop_front();
      Rete::Agenda::Locker lock(agenda);
      node->decision(*this);
    }

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
