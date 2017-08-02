#include "agent.h"

//#include "../infinite_mario/infinite_mario.h"

namespace Carli {

#ifdef NDEBUG
  inline void dump_rules(const Agent &) {}
#else
  static void dump_rules(const Agent &agent) {
    const std::string rules_out_file = dynamic_cast<const Option_String &>(Options::get_global()["rules-out"]).get_value();
    if(!rules_out_file.empty()) {
      std::ofstream rules_out((rules_out_file + ".bak").c_str());
      rules_out << "set-total-step-count " << agent.get_total_step_count() << std::endl << std::endl;
      agent.rete_print_rules(rules_out);
    }
  }
#endif

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

  struct Feature_Dependency_Collector {
    std::map<tracked_ptr<Feature>,
      std::map<tracked_ptr<Feature>, double, Rete::compare_deref_lt>,
      Rete::compare_deref_lt> dependencies;

    void operator()(Rete::Rete_Node &rete_node) {
      if(!rete_node.data) {
        assert(!dynamic_cast<Rete::Rete_Action *>(&rete_node));
        return;
      }

      auto &node = debuggable_cast<Carli::Node &>(*rete_node.data);
      auto &feature = node.q_value_fringe->feature;
      if(!feature || !node.q_value_weight)
        return;
      const auto weight = pow(0.5, node.q_value_fringe->depth - 2);
      auto &depends = dependencies[feature];

      auto ancestor = node.parent_action.lock();
      while(ancestor) {
        auto &ancestor_node = debuggable_cast<Carli::Node &>(*ancestor->data);
        auto &ancestor_feature = ancestor_node.q_value_fringe->feature;

        if(ancestor_feature) {
          dependencies[ancestor_feature];
          depends[ancestor_feature] += weight;
        }

        ancestor = ancestor_node.parent_action.lock();
      }
    }

    void print(std::ostream &os) {
      for(const auto &dependee : dependencies)
        os << ',' << *dependee.first;
      os << std::endl;

      for(const auto &depender : dependencies) {
        os << *depender.first;
        for(const auto &dependee : dependencies)
          os << ',' << dependencies[dependee.first][depender.first];
        os << std::endl;
      }
    }
  };

  struct Fringe_Collector_All {
    struct Data {
      Rete::Rete_Node_Ptr node;
      int64_t count = 0;
      double aggregate_value = 0.0;
    };

    std::unordered_set<Rete::Rete_Action_Ptr> excise;
    std::map<tracked_ptr<Feature>,
      std::map<tracked_ptr<Feature>, Data, Rete::compare_deref_lt>,
      Rete::compare_deref_memfun_lt<Feature, Feature, &Feature::compare_axis>> features;

    Fringe_Collector_All(const Node_Split_Ptr &split)
      : root(split),
      root_feature(debuggable_cast<Carli::Node &>(*split).q_value_fringe->feature)
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
        auto &feature = node.q_value_fringe->feature;
        if(!feature)
          return;
        if(root_feature && *root_feature == *feature)
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

          auto &data = found_axis->second[feature];

          if(data.node) {
            ++data.count;
            data.aggregate_value += node.q_value_fringe->primary;
          }
          else {
            Data &datum = found_axis->second[feature];
            datum.node = rete_node.shared();
            datum.count = 1;
            datum.aggregate_value = node.q_value_fringe->primary;
          }
        }
        else {
          Data &datum = features[feature][feature];
          datum.node = rete_node.shared();
          datum.count = 1;
          datum.aggregate_value = node.q_value_fringe->primary;
        }
      }
    }

    Node_Ptr root;
    tracked_ptr<Feature> root_feature;
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
          fringe_values[fringe->q_value_fringe->feature.get()].push_back(fringe);
      }
    }

    Rete::Rete_Node_Ptr grandparent;
    Fringe_Values fringe_values;
  };

  bool Agent::respecialize(Rete::Rete_Action &rete_action) {
    if(!m_learning_rate)
      return false;

#ifndef NDEBUG
    Node_Tracker::get().validate(*this, nullptr);
#endif

    const auto parent_node = rete_action.parent_left();
    if(m_rete_nodes_evaluated.find(parent_node) != m_rete_nodes_evaluated.end())
      return false;
    m_rete_nodes_evaluated.insert(parent_node);

    auto &general_node = debuggable_cast<Node &>(*rete_action.data);
    if(general_node.q_value_weight->type != Q_Value::Type::SPLIT)
      return true;

    auto &general = debuggable_cast<Node_Split &>(general_node);

    if(general.blacklist_full)
      return false;

    if(m_concrete_update_count && general.q_value_weight->update_count > m_concrete_update_count) {
#ifdef DEBUG_OUTPUT
      std::cerr << "Cementing " << general.rete_action.lock()->get_name() << std::endl;
#endif

      general.blacklist_full = true;

      for(auto &fringe_axis : general.fringe_values) {
        for(auto &fringe : fringe_axis.second) {
          const auto rete_action = fringe.lock()->rete_action.lock();
          if(rete_action)
            excise_rule(rete_action->get_name(), false);
        }
      }

      general.fringe_values.clear();

      return false;
    }

    if(random.frand_lt() >= m_unsplit_probability)
      return false;

//#ifndef NO_COLLAPSE_DETECTION_HACK
//    if(m_experienced_n_positive_rewards_in_a_row)
//      return false;
//#endif

    //if(debuggable_cast<Node *>(rete_action.data.get())->q_value->depth < 3)
    //  return false;

    if(m_unsplit_criterion(general))
      return collapse_rete(rete_action);
    else {
//#ifndef NO_COLLAPSE_DETECTION_HACK
//      m_positive_rewards_in_a_row = 0;
//#endif

      return false;
    }
  }

  bool Agent::specialize(Rete::Rete_Action &rete_action) {
//#ifndef NDEBUG
//    Node_Tracker::get().validate(*this); {
//#endif

    auto &general = debuggable_cast<Node_Unsplit &>(*rete_action.data);

    if(general.q_value_weight->type == Q_Value::Type::SPLIT)
      return true;
    if(general.q_value_weight->depth >= m_split_max || !m_learning_rate)
      return false;

    const auto parent_node = rete_action.parent_left();
    if(m_rete_nodes_evaluated.find(parent_node) != m_rete_nodes_evaluated.end())
      return false;
    m_rete_nodes_evaluated.insert(parent_node);

    if(random.frand_lt() >= m_split_probability)
      return false;

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
      g_output_dot_exp_count = (g_output_dot_exp_count + 1) % 4;
      fname << "pre-expansion-" << g_output_dot_exp_count << ".dot";
      std::ofstream fout(fname.str());
      rete_print(fout);
    }

    auto new_node = general.create_split(general.parent_action.lock());

    if(m_unsplit_test == "none" || general.q_value_fringe->depth < m_split_min)
      new_node->blacklist_full = true;
    else if(m_resplit_bias != "none") {
      auto found = new_node->fringe_axis_selections.find(chosen->first);
      if(found == new_node->fringe_axis_selections.end())
        new_node->fringe_axis_selections[chosen->first->clone()] += ++new_node->fringe_axis_counter;
      else
        found->second += ++new_node->fringe_axis_counter;

      if(m_resplit_bias == "blacklist" && new_node->fringe_axis_selections.size() == general.fringe_values.size())
        new_node->blacklist_full = true;
    }

#ifndef NDEBUG
    Node_Tracker::get().validate(*this, nullptr);
#endif

    expand_fringe(rete_action, new_node->rete_action.lock(), chosen);

    excise_rule(rete_action.get_name(), false);

#ifndef NDEBUG
    Node_Tracker::get().validate(*this, &general);
#endif

    if(m_output_dot) {
      std::cerr << "Rete size after expansion: " << rete_size() << std::endl;

      std::ostringstream fname;
      fname << "post-expansion-" << g_output_dot_exp_count << ".dot";
      std::ofstream fout(fname.str());
      rete_print(fout);
    }

//#ifndef NDEBUG
//    } Node_Tracker::get().validate(*this);
//#endif

    return true;
  }

  void Agent::expand_fringe(Rete::Rete_Action &rete_action, const Rete::Rete_Action_Ptr &parent_action, const Fringe_Values::iterator &specialization)
  {
    auto &unsplit = debuggable_cast<Node_Unsplit &>(*rete_action.data);
    auto &split = debuggable_cast<Node_Split &>(*parent_action->data);

    /** Step 1: Collect new leaves from the fringe
     *          They'll have to be modified / replaced, but it's good to separate them from the remainder of the fringe
     */
    std::list<Node_Fringe_Ptr> leaves;
    for(auto leaf : specialization->second)
      leaves.push_back(leaf.lock());
    specialization->second.clear();
    unsplit.fringe_values.erase(specialization);

#ifdef DEBUG_OUTPUT
    std::cerr << "Refining:";
    for(auto &leaf : leaves) {
      std::cerr << ' ';
      if(leaf->rete_action.lock()->is_active())
        std::cerr << '*';
      std::cerr << *leaf->q_value_fringe->feature;
    }
    std::cerr << std::endl << "Carrying over detected:";
    for(auto &fringe_axis : unsplit.fringe_values) {
      for(auto &fringe_w : fringe_axis.second) {
        auto fringe = fringe_w.lock();
        std::cerr << ' ';
        if(fringe->rete_action.lock()->is_active())
          std::cerr << '*';
        std::cerr << *fringe->q_value_fringe->feature;
      }
    }
    std::cerr << std::endl;
#endif

    /** Step 1.5: Detect whether a new & distinct variable needs to be created for Higher Order Grammar rules, along with new fringe nodes **/
    std::string old_new_var_name, prev_var_name;
    Rete::WME_Token_Index old_new_var_index(-1, -1, -1), prev_var_index(-1, -1, -1);
    if((*leaves.begin())->q_value_fringe->feature->max_arity > 0) {
      const auto &vars = (*leaves.begin())->q_value_fringe->feature->indices;
      const auto &parent_vars = parent_action->get_variables();

      int64_t num_new_vars = 0;
      {
        auto pt = vars->end();
        auto vt = vars->begin();
        const auto vend = vars->end();
        auto pvt = parent_vars->begin();
        while(vt != vend) {
          if(*vt == *pvt) {
            pt = vt;
            ++vt;
            ++pvt;
          }
          else {
            ++num_new_vars;
            assert(num_new_vars < 2); /// Not sure how to cope with more than one new variable at a time -- probably not necessary
            old_new_var_name = vt->first;
            old_new_var_index = vt->second;

            if(pt != vars->end()) {
              size_t last_hyphen_p1 = pt->first.rfind('-') + 1;
              std::ostringstream oss;
              oss << pt->first.substr(0, last_hyphen_p1) << atoi(pt->first.substr(last_hyphen_p1).c_str()) + 1;
              if(oss.str() == old_new_var_name) {
                prev_var_name = oss.str();
                prev_var_index = pt->second;
              }
            }

            pt = vt;
            ++vt;
          }
        }
      }
    }

    /** Step 2: For each new leaf...
     *          ...create it, clone remaining fringe entries below the new leaf, and destroy the old fringe node
     */
    for(auto &leaf : leaves) {
//      if(leaf->q_value_fringe->depth <= m_split_max)
      {
//        bool terminal = leaf->q_value_fringe->depth == m_split_max;
        bool terminal = false;
        std::vector<Carli::Feature *> refined;
//        if(!terminal)
        {
          refined = leaf->q_value_fringe->feature->refined();
          terminal = refined.empty() && unsplit.fringe_values.empty();
        }

        /** Step 2.1: Create a new split/unsplit node depending on the existence of a new fringe. */
        if(terminal)
          leaf->create_split(parent_action);
        else {
          const auto node_unsplit = leaf->create_unsplit(parent_action);

          /** Step 2.2: Create new ranged fringe nodes if the new leaf is refineable. */
          for(auto &refined_feature : refined)
            node_unsplit->create_fringe(*node_unsplit, refined_feature);

          /** Step 2.3 Create new fringe nodes. **/
          for(auto &fringe_axis : unsplit.fringe_values) {
            for(auto &fringe : fringe_axis.second)
              fringe.lock()->create_fringe(*node_unsplit, nullptr);
          }

          /** Step 2.4: Create new fringe nodes if demanded by the HOG. */
          if(old_new_var_index.rete_row != -1) {
            for(auto &fringe : leaves) {
              fringe->create_fringe(*node_unsplit, nullptr, old_new_var_index);

              bool binary_feature_update = false;
              auto test = fringe->rete_action.lock()->parent_left();
              if(auto bindings = test->get_bindings()) {
                for(auto binding : *bindings) {
                  if(binding.first == prev_var_index || binding.second == prev_var_index) {
                    binary_feature_update = true;
                    break;
                  }
                }
              }
              else if(auto predicate_node = dynamic_cast<Rete::Rete_Predicate *>(test.get())) {
                if(predicate_node->get_lhs_index() == prev_var_index || predicate_node->get_rhs_index() == prev_var_index)
                  binary_feature_update = true;
              }
              if(binary_feature_update)
                fringe->create_fringe(*node_unsplit, nullptr, old_new_var_index, prev_var_index);
            }
            for(auto &fringe_axis : unsplit.fringe_values) {
              if(fringe_axis.first->indices->find(old_new_var_name) != fringe_axis.first->indices->end()) {
                for(auto &fringe : fringe_axis.second) {
                  auto flock = fringe.lock();

                  flock->create_fringe(*node_unsplit, nullptr, old_new_var_index);

                  bool binary_feature_update = false;
                  auto test = flock->rete_action.lock()->parent_left();
                  if(auto bindings = test->get_bindings()) {
                    for(auto binding : *bindings) {
                      if(binding.first == prev_var_index || binding.second == prev_var_index) {
                        binary_feature_update = true;
                        break;
                      }
                    }
                  }
                  else if(auto predicate_node = dynamic_cast<Rete::Rete_Predicate *>(test.get())) {
                    if(predicate_node->get_lhs_index() == prev_var_index || predicate_node->get_rhs_index() == prev_var_index)
                      binary_feature_update = true;
                  }
                  if(binary_feature_update)
                    flock->create_fringe(*node_unsplit, nullptr, old_new_var_index, prev_var_index);
                }
              }
            }
          }
        }
      }
    }

    for(auto &leaf : leaves) {
      /** Step 2.5: Untrack the old fringe node. */
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
      m_mean_matde.uncontribute(leaf->q_value->matde);
#endif
      if(!m_mean_catde_queue_size)
        m_mean_catde.uncontribute(leaf->q_value_fringe->catde);

      /** Step 2.6: Destroy the old leaf node. */
      excise_rule(leaf->rete_action.lock()->get_name(), false);
    }

    if(split.blacklist_full) {
      /** Step 3a: Excise all old fringe rules from the system */
      for(auto &fringe_axis : unsplit.fringe_values)
        for(auto &fringe : fringe_axis.second)
          excise_rule(fringe.lock()->rete_action.lock()->get_name(), false);
    }
    else {
      /** Step 3b: Convert fringe nodes to internal fringe values **/
      split.fringe_values.swap(unsplit.fringe_values);
      for(auto &fringe_axis : split.fringe_values) {
        for(auto &fringe_w : fringe_axis.second) {
          auto fringe = fringe_w.lock();
          fringe->parent_action = split.rete_action;
          fringe->q_value_fringe->type_internal = true;
          fringe->q_value_fringe->update_count = 0;
          fringe->q_value_fringe->catde = 0.0;
          fringe->q_value_fringe->catde_post_split = 0.0;
        }
      }
    }

    if(auto grandparent_action = unsplit.parent_action.lock()) {
      auto &grandparent_node = debuggable_cast<Node_Split &>(*grandparent_action->data);
      auto found = std::find_if(grandparent_node.children.begin(), grandparent_node.children.end(), [&rete_action](const Node_Ptr_W &node){return node.lock() == rete_action.data;});
      assert(found != grandparent_node.children.end());
      grandparent_node.children.erase(found);
    }
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
      g_output_dot_col_count = (g_output_dot_col_count + 1) % 4;
      fname << "pre-collapse-" << g_output_dot_col_count << ".dot";
      std::ofstream fout(fname.str());
      rete_print(fout);
    }

#ifdef DEBUG_OUTPUT
    std::cerr << "Features: ";
    for(const auto &feature_axis : fringe_collector.features)
      for(const auto &feature_node : feature_axis.second)
        std::cerr << ' ' << *feature_node.first << ';' << feature_node.second.count << ';' << feature_node.second.aggregate_value;
    std::cerr << std::endl;
#endif

    /// Make new unsplit node
    const auto unsplit = split->create_unsplit(split->parent_action.lock());
#ifdef DEBUG_OUTPUT
    std::cerr << "Collapsing " << split << " to " << unsplit << std::endl;
#endif

    /// Preserve some learning
    /// Already being done in node.cpp, but should attempt to do better
//    unsplit->q_value->value = split->q_value->value;

    /// Make new fringe
    Fringe_Values node_unsplit_fringe;
    for(const auto &feature_axis : fringe_collector.features) {
      for(const auto &feature_node : feature_axis.second) {
        auto &node = debuggable_cast<Node &>(*feature_node.second.node->data.get());

        /// Throw away nodes considering 2 or more new variables
        if(node.variables->size() > unsplit->variables->size() + 1)
          continue;

        auto new_fringe = node.create_fringe(*unsplit, nullptr);

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

    if(auto grandparent_action = split->parent_action.lock()) {
      auto &grandparent_node = debuggable_cast<Node_Split &>(*grandparent_action->data);
      auto found = std::find_if(grandparent_node.children.begin(), grandparent_node.children.end(), [&split](const Node_Ptr_W &node){return node.lock() == split;});
      assert(found != grandparent_node.children.end());
      grandparent_node.children.erase(found);
    }

    if(m_output_dot) {
      std::cerr << "Rete size after collapse: " << rete_size() << std::endl;

      std::ostringstream fname;
      fname << "post-collapse-" << g_output_dot_col_count << ".dot";
      std::ofstream fout(fname.str());
      rete_print(fout);
    }

//#ifndef NDEBUG
//    unsplit->rete_action.lock()->visit_preorder(Expiration_Detector(), false);
//#endif

    ++m_unrefinements[split->rank()];

    return true;
  }

  Agent::Agent(const std::shared_ptr<Environment> &environment, const std::function<Carli::Action_Ptr_C (const Rete::Variable_Indices &variables, const Rete::WME_Token &token)> &get_action_)
    : get_action(get_action_),
    m_target_policy([this]()->Action_Ptr_C{return this->choose_greedy(nullptr, std::numeric_limits<int64_t>::max());}),
    m_exploration_policy(
#ifdef ENABLE_T_TEST
		                 dynamic_cast<const Option_Itemized &>(Options::get_global()["exploration"]).get_value() == "t-test" ?
		                 std::function<Action_Ptr_C ()>([this]()->Action_Ptr_C{return this->choose_t_test(nullptr, std::numeric_limits<int64_t>::max());}) :
#endif
		                 std::function<Action_Ptr_C ()>([this]()->Action_Ptr_C{return this->choose_epsilon_greedy(nullptr, std::numeric_limits<int64_t>::max());})),
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
    else if(m_split_test == "utile") {
      m_split_criterion = [this](Node_Unsplit &general)->Fringe_Values::iterator{
        return this->split_test_utile(general);
      };
    }
    else
      abort();

    if(m_unsplit_test == "none") {
      m_unsplit_criterion = [](Node_Split &)->bool{
        return false;
      };
    }
    else if(m_unsplit_test == "catde") {
      m_unsplit_criterion = [this](Node_Split &general)->bool{
        return this->unsplit_test_catde(general);
      };
    }
    else if(m_unsplit_test == "policy") {
      m_unsplit_criterion = [this](Node_Split &general)->bool{
        return this->unsplit_test_policy(general);
      };
    }
    else if(m_unsplit_test == "value") {
      m_unsplit_criterion = [this](Node_Split &general)->bool{
        return this->unsplit_test_value(general);
      };
    }
    else if(m_unsplit_test == "utile") {
      m_unsplit_criterion = [this](Node_Split &general)->bool{
        return this->unsplit_test_utile(general);
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

    m_unrefinements[1] = 0;
  }

  Agent::~Agent() {
    m_rete_nodes_evaluated.clear();
    excise_all();
  }

  void Agent::destroy() {
    const std::string rules_out_file = dynamic_cast<const Option_String &>(Options::get_global()["rules-out"]).get_value();
    if(!rules_out_file.empty()) {
      std::ofstream rules_out(rules_out_file.c_str());
      rules_out << "# CPU time = " << rete_cpu_time() << " seconds" << std::endl << std::endl;
      rules_out << "set-total-step-count " << m_total_step_count << std::endl << std::endl;
      rete_print_rules(rules_out);
    }

    const std::string dependencies_out_file = dynamic_cast<const Option_String &>(Options::get_global()["dependencies-out"]).get_value();
    if(!dependencies_out_file.empty()) {
      auto dependency_collector = visit_preorder(Feature_Dependency_Collector(), true);
      std::ofstream dependencies_out(dependencies_out_file);
      dependency_collector.print(dependencies_out);
    }

    m_rete_nodes_evaluated.clear();
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
    /// Calculate \rho
    double rho = 1.0;
    if(!m_on_policy)
      rho = probability_greedy(m_next, nullptr, std::numeric_limits<int64_t>::max()) / probability_epsilon_greedy(m_next, m_epsilon, nullptr, std::numeric_limits<int64_t>::max());

    m_current = m_next;
    m_current_q_value = m_next_q_values[m_next];
//    m_current_q_value.sort([](const tracked_ptr<Q_Value> &lhs, const tracked_ptr<Q_Value> &rhs)->bool{return lhs->depth < rhs->depth;});

    if(!m_current) {
      std::cerr << "No action selected. Terminating." << std::endl;
      abort();
    }

    const std::pair<reward_type, reward_type> reward = m_environment->transition(*m_current);

#ifndef NO_COLLAPSE_DETECTION_HACK
    if(reward.first > 0.0) {
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
      td_update(m_current_q_value, reward.first, value_best, rho, 1.0);

      if(!m_on_policy) {
        const auto next = m_exploration_policy();

        if(!m_secondary_learning_rate && *m_next != *next &&
           std::get<0>(sum_value(nullptr, m_next_q_values[next], nullptr, std::numeric_limits<int64_t>::max())) < std::get<0>(sum_value(nullptr, m_next_q_values[m_next], nullptr, std::numeric_limits<int64_t>::max())))
        {
          clear_eligibility_trace();
        }

        m_next = next;

#ifdef DEBUG_OUTPUT
        std::cerr << "   " << *m_next << " is next." << std::endl;
#endif
      }
    }
    else {
      td_update(m_current_q_value, reward.first, Q_Value_List(), rho, 1.0);
    }

    m_total_reward += reward.second;
    ++m_step_count;
    ++m_total_step_count;

    return reward.second;
  }

  Rete::Rete_Action_Ptr Agent::make_standard_action(const Rete::Rete_Node_Ptr &parent, const std::string &name, const bool &user_command, const Rete::Variable_Indices_Ptr_C &variables) {
    return make_action_retraction(name, user_command,
      [this](const Rete::Rete_Action &action, const Rete::WME_Token &token) {
        debuggable_cast<Node *>(action.data.get())->action(token);
      }, [this](const Rete::Rete_Action &action, const Rete::WME_Token &token) {
        debuggable_cast<Node *>(action.data.get())->retraction(token);
      }, parent, variables);
  }

  Rete::Rete_Action_Ptr Agent::make_standard_fringe(const Rete::Rete_Node_Ptr &parent, const std::string &name, const bool &user_command, const Node_Unsplit_Ptr &root_action_data, const tracked_ptr<Feature> &feature, const Rete::Variable_Indices_Ptr_C &variables) {
    auto new_leaf = make_standard_action(parent, name, user_command, variables);
    auto new_leaf_data = std::make_shared<Node_Fringe>(*this, root_action_data->rete_action.lock(), new_leaf, 2, feature);
    new_leaf->data = new_leaf_data;
    root_action_data->fringe_values[new_leaf_data->q_value_fringe->feature.get()].push_back(new_leaf_data);
    return new_leaf;
  }

  void Agent::purge_q_value(const tracked_ptr<Q_Value> &q_value) {
  //#ifdef DEBUG_OUTPUT
  //  std::cerr << "Purging current value " << q_value << std::endl;
  //#endif
    assert(q_value);
//    auto found = std::find(m_current_q_value.begin(), m_current_q_value.end(), q_value);
    auto found = m_current_q_value.find(q_value);
    if(found != m_current_q_value.end()) {
      m_current_q_value.erase(found);
//      assert(std::find(m_current_q_value.begin(), m_current_q_value.end(), q_value) == m_current_q_value.end());
    }
  }

  void Agent::insert_q_value_next(const Action_Ptr_C &action, const tracked_ptr<Q_Value> &q_value) {
  //#ifdef DEBUG_OUTPUT
  //  std::cerr << "Inserting next value " << q_value << " for action " << *action << std::endl;
  //#endif
    auto &q_values = m_next_q_values[action];

//#ifdef DEBUG_OUTPUT
//    std::cerr << "insert_q_value_next(" << *action << ',' << q_value.get() << ") into {";
//    for(auto &q : q_values) {
//      if(action) {
//        std::cerr << ' ' << q.first->value /* * q.weight */ << ':' << q.first->depth;
//        if(q.first->type == Q_Value::Type::FRINGE)
//          std::cerr << 'f';
//        if(q.first->feature)
//          std::cerr << ':' << *q.first->feature;
//      }
//    }
//    std::cerr << " }." << std::endl;
//#endif

//#ifndef NDEBUG
//    if(std::find(q_values.begin(), q_values.end(), q_value) != q_values.end()) {
//      std::cerr << "BADNESS INCREMENT FOR " << *action << ": ";
//        if(q_value->feature)
//          std::cerr << *q_value->feature;
//        std::cerr << std::endl;
//      increment_badness();
//    }
//#endif
    ++q_values[q_value];
  }

  void Agent::purge_q_value_next(const Action_Ptr_C &action, const tracked_ptr<Q_Value> &q_value) {
  //#ifdef DEBUG_OUTPUT
  //  std::cerr << "Purging next value " << q_value << " for action " << *action << std::endl;
  //#endif
    assert(q_value);
    auto &q_values = m_next_q_values[action];
//    auto found = std::find(q_values.begin(), q_values.end(), q_value);
    auto found = q_values.find(q_value);
    assert(found != q_values.end());
    if(found != q_values.end()) {
      if(!--found->second)
        q_values.erase(found);
//#ifndef NDEBUG
//      if(std::find(q_values.begin(), q_values.end(), q_value) != q_values.end()) {
//        std::cerr << "BADNESS DECREMENT FOR " << *action << ": ";
//        if(q_value->feature)
//          std::cerr << *q_value->feature;
//        std::cerr << std::endl;
//        decrement_badness();
//      }
//#endif
    }

    if(q_values.empty())
      m_next_q_values.erase(action);

//#ifdef DEBUG_OUTPUT
//    std::cerr << "purge_q_value_next(" << *action << ',' << q_value.get() << ") ";
//    std::cerr << (found != q_values.end() ? "succeeded" : "failed");
//    std::cerr << " from {";
//    for(auto &q : q_values) {
//      if(action) {
//        std::cerr << ' ' << q.first->value /* * q.weight */ << ':' << q.first->depth;
//        if(q.first->type == Q_Value::Type::FRINGE)
//          std::cerr << 'f';
//        if(q.first->feature)
//          std::cerr << ':' << *q.first->feature;
//      }
//    }
//    std::cerr << " }." << std::endl;
//#endif
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
        if(node.q_value_weight)
          ++node.q_value_weight->depth;
        ++node.q_value_fringe->depth;
      }
    };

    visit_preorder(visitor, true);
  }

  void Agent::visit_reset_update_count() {
    std::function<void (Rete::Rete_Node &)> visitor = [](Rete::Rete_Node &rete_node) {
      if(rete_node.data) {
        auto &node = debuggable_cast<Carli::Node &>(*rete_node.data);
        if(node.q_value_weight)
          node.q_value_weight->update_count = 0;
        node.q_value_fringe->update_count = 0;
      }
    };

    visit_preorder(visitor, true);
  }

  Action_Ptr_C Agent::choose_epsilon_greedy(const Node_Fringe * const &fringe, const int64_t &fringe_depth) {
    if(random.frand_lt() < m_epsilon)
      return choose_randomly();
    else
      return choose_greedy(fringe, fringe_depth);
  }

#ifdef ENABLE_T_TEST

  Action_Ptr_C Agent::choose_t_test(const Node_Fringe * const &fringe, const int64_t &fringe_depth) {
    const Q_Value * const parent_q = fringe ? dynamic_cast<Node *>(fringe->parent_action.lock()->data.get())->q_value_weight.get() : nullptr;
    const auto greedy = choose_greedy(fringe, fringe_depth);
    const auto greedy_weights = m_next_q_values[greedy];
    const auto greedy_value = parent_q && std::find(greedy_weights.begin(), greedy_weights.end(), parent_q) == greedy_weights.end()
                            ? sum_value(greedy.get(), greedy_weights, nullptr, fringe_depth)
                            : sum_value(greedy.get(), greedy_weights, fringe ? fringe->q_value_fringe->feature.get() : nullptr, fringe_depth);

    Action_Ptr_C gtewp;
    int32_t gtewp_count = 0;
    for(const auto &action_q : m_next_q_values) {
      const auto value = parent_q && std::find(action_q.second.begin(), action_q.second.end(), parent_q) == action_q.second.end()
                       ? sum_value(action_q.first.get(), action_q.second, nullptr, fringe_depth)
                       : sum_value(action_q.first.get(), action_q.second, fringe ? fringe->q_value_fringe->feature.get() : nullptr, fringe_depth);

      if(probability_gte(std::get<2>(value), std::get<0>(value), std::get<1>(value),
                         std::get<2>(greedy_value), std::get<0>(greedy_value), std::get<1>(greedy_value)))
      {
        ++gtewp_count;
        if(gtewp_count == 1 || random.rand_lt(gtewp_count) == 0)
          gtewp = action_q.first;
      }
    }

    return gtewp;
  }

#endif

  Action_Ptr_C Agent::choose_greedy(const Node_Fringe * const &fringe, const int64_t &fringe_depth) {
    auto greedies = choose_greedies(fringe, fringe_depth);
    int32_t count = random.rand_lt(int32_t(greedies.size()));
    while(count--)
      greedies.pop_front();
    return greedies.front();
  }

  std::list<Action_Ptr_C, Zeni::Pool_Allocator<Action_Ptr_C>> Agent::choose_greedies(const Node_Fringe * const &fringe, const int64_t &fringe_depth) {
#ifdef DEBUG_OUTPUT
    std::cerr << "  choose_greedy(";
    if(fringe)
      fringe->q_value_fringe->feature->print_axis(std::cerr);
    std::cerr << ')' << std::endl;
#endif

    std::list<Action_Ptr_C, Zeni::Pool_Allocator<Action_Ptr_C>> greedies;
    double value = double();
    const tracked_ptr<Q_Value> parent_q = fringe ? dynamic_cast<Node *>(fringe->parent_action.lock()->data.get())->q_value_weight : nullptr;
    for(const auto &action_q : m_next_q_values) {
      double value_;
//      if(parent_q && std::find(action_q.second.begin(), action_q.second.end(), parent_q) == action_q.second.end())
      if(parent_q && action_q.second.find(parent_q) == action_q.second.end())
        value_ = std::get<0>(sum_value(action_q.first.get(), action_q.second, nullptr, fringe_depth));
      else
        value_ = std::get<0>(sum_value(action_q.first.get(), action_q.second, fringe ? fringe->q_value_fringe->feature.get() : nullptr, fringe_depth));

      if(greedies.empty() || value_ > value) {
        greedies = {{action_q.first}};
        value = value_;
      }
      else if(value_ == value)
        greedies.push_back(action_q.first);
    }

    return greedies;
  }

  Action_Ptr_C Agent::choose_randomly() {
#ifdef DEBUG_OUTPUT
    std::cerr << "  choose_randomly" << std::endl;
#endif

    int32_t counter = int32_t(m_next_q_values.size());
    counter = random.rand_lt(counter) + 1;
    Action_Ptr_C action;
    for(const auto &action_q : m_next_q_values) {
      if(!--counter)
        action = action_q.first;
    }

    return action;
  }

  double Agent::probability_epsilon_greedy(const Action_Ptr_C &action, const double &epsilon, const Node_Fringe * const &fringe, const int64_t &fringe_depth) {
    return (1 - epsilon) * probability_greedy(action, fringe, fringe_depth) + epsilon * probability_random();
  }

  double Agent::probability_greedy(const Action_Ptr_C &action, const Node_Fringe * const &fringe, const int64_t &fringe_depth) {
    const tracked_ptr<Q_Value> parent_q = fringe ? dynamic_cast<Node *>(fringe->parent_action.lock()->data.get())->q_value_weight : nullptr;
    const auto &next_q_values_action = m_next_q_values[action];
    if(parent_q && next_q_values_action.find(parent_q) == next_q_values_action.end())
      return 0.0;

    double count = double();
    const double value = std::get<0>(sum_value(action.get(), next_q_values_action, fringe ? fringe->q_value_fringe->feature.get() : nullptr, fringe_depth));

    for(const auto &action_q : m_next_q_values) {
      if(parent_q && action_q.second.find(parent_q) == action_q.second.end())
        continue;
      const double value_ = std::get<0>(sum_value(action_q.first.get(), action_q.second, fringe ? fringe->q_value_fringe->feature.get() : nullptr, fringe_depth));

      if(value_ > value)
        return 0.0;
      else if(value_ == value)
        ++count;
    }

    return 1.0 / count;
  }

  double Agent::probability_random() {
    return 1.0 / m_next_q_values.size();
  }

  void Agent::td_update(const Q_Value_List &current, const reward_type &reward, const Q_Value_List &next, const double &rho, const double &I) {
    if(!m_learning_rate)
      return;

//    dump_rules(*this);
    assert(!m_badness);

    const double target_next = m_discount_rate * std::get<0>(sum_value(nullptr, next, nullptr, std::numeric_limits<int64_t>::max()));
    const double target_value = reward + target_next;

    double q_old = 0.0;
#ifdef DEBUG_OUTPUT
    std::cerr << " current :";
#endif
    for(const auto &q : current) {
      ++q.first->update_count;

      if(q.first->type != Q_Value::Type::FRINGE) {
        q_old += q.first->primary /* * q.weight */;
#ifdef DEBUG_OUTPUT
        std::cerr << ' ' << q.first->primary;
#endif
      }
    }
#ifdef DEBUG_OUTPUT
    std::cerr << std::endl;
    std::cerr << " fringe  :";
    for(const auto &q : current) {
      if(q.first->type == Q_Value::Type::FRINGE)
        std::cerr << ' ' << q.first->primary;
    }
    std::cerr << std::endl;
    std::cerr << " next    :";
    for(const auto &q : next) {
      if(q.first->type != Q_Value::Type::FRINGE)
        std::cerr << ' ' << q.first->primary;
    }
    std::cerr << std::endl;
#endif

    m_credit_assignment(current);

    double dot_w_phi = 0.0;
    if(m_secondary_learning_rate) {
      for(Q_Value::List::list_pointer_type q_ptr = m_eligible; q_ptr; q_ptr = q_ptr->next())
        (*q_ptr)->eligibility *= rho;
    }
    for(const auto &q : current) {
      if(!q.first->credit)
        continue;

      if(q.first->eligibility < 0.0)
        q.first->eligible.insert_before(m_eligible);

      q.first->eligibility = (q.first->eligibility == -1.0 ? 0.0 : q.first->eligibility) + (m_secondary_learning_rate ? I * q.first->credit : q.first->credit);
      q.first->eligibility_init = true;

      if(q.first->type != Q_Value::Type::FRINGE)
        dot_w_phi += q.first->secondary;

      if(q.first->update_count > 1) {
        const double delta = (target_value - q_old) * m_learning_rate;
        const double q_new = q_old + delta; ///< Could calculate like q_old post-update -- might be different
        const double mdelta = std::max(0.0, (target_value - q_old) * (target_value - q_new));

        assert(q.first->primary_mean2 >= 0.0);
        q.first->primary_mean2 += mdelta * q.first->credit;
        assert(q.first->primary_mean2 >= 0.0);
        q.first->primary_variance = q.first->primary_mean2 / (q.first->update_count - 1);
      }
    }

    const double delta = target_value - q_old;
#ifdef ENABLE_WEIGHT
    const bool weight_assignment_all = m_weight_assignment_code == "all";
#else
    const bool weight_assignment_all = true;
#endif
    double dot_w_e = 0.0;
    for(Q_Value::List::list_pointer_type q_ptr = m_eligible; q_ptr; q_ptr = q_ptr->next()) {
      Q_Value &q = **q_ptr;

      const double ldelta = weight_assignment_all && q.type != Q_Value::Type::FRINGE ? delta : target_value - q.primary;
      const double edelta = q.eligibility * ldelta;

      if(q.type != Q_Value::Type::FRINGE)
        dot_w_e += q.secondary * q.eligibility;

      q.primary += m_learning_rate * edelta;
      if(m_secondary_learning_rate && q.credit)
        q.secondary += m_learning_rate * m_secondary_learning_rate * edelta;
      q.update_totals();

      if(q.type == Q_Value::Type::FRINGE) {
        if(q.type_internal) {
          const double abs_delta = fabs(target_value - q.primary);
#ifdef DEBUG_OUTPUT
          std::cerr << "Absolute Delta = " << abs_delta << std::endl;
#endif
          q.catde += abs_delta;
          q.catde_post_split += abs_delta;
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
      }
    }

    if(m_secondary_learning_rate) {
      for(auto &q : next) {
        if(q.first->credit) {
          q.first->primary -= m_learning_rate * m_discount_rate * (1 - m_eligibility_trace_decay_rate) * dot_w_e;
          q.first->update_totals();
        }
      }

      for(auto &q : current) {
        if(q.first->credit) {
          q.first->secondary -= m_learning_rate * m_secondary_learning_rate * dot_w_phi;
          q.first->update_totals();
        }
      }
    }

    for(Q_Value::List::list_pointer_type q_ptr = m_eligible; q_ptr; ) {
      Q_Value &q = **q_ptr;
      const auto q_next = q_ptr->next();

      assert(q.eligibility >= 0.0);
      q.eligibility_init = false;
      q.eligibility *= m_discount_rate * m_eligibility_trace_decay_rate;
      if(q.eligibility < m_eligibility_trace_decay_threshold) {
        q.eligible.erase_from(m_eligible);
        q.eligibility = -1.0;
      }

      q_ptr = q_next;
    }

#ifdef DEBUG_OUTPUT
    double q_new = double();
    for(const auto &q : current) {
      if(q.first->type != Q_Value::Type::FRINGE)
        q_new += q.first->primary /* * q.weight */;
    }

    std::cerr.unsetf(std::ios_base::floatfield);
    std::cerr << " td_update: " << q_old << " <" << m_learning_rate << "= " << reward << " + " << m_discount_rate << " * " << target_next << std::endl;
    std::cerr << "            " << delta << " = " << target_value << " - " << q_old << std::endl;
    std::cerr << "            " << q_new << std::endl;
    std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);

    for(const auto &q : current) {
      if(q.first->type == Q_Value::Type::UNSPLIT) {
        std::cerr << " updates:  " << q.first->update_count << std::endl;
        if(m_mean_catde_queue_size)
          std::cerr << " catde q:   " << q.first->catde << " of " << this->m_mean_catde_queue.mean() << ':' << this->m_mean_catde_queue.mean().get_stddev() << std::endl;
        else
          std::cerr << " catde:     " << q.first->catde << " of " << this->m_mean_catde << ':' << this->m_mean_catde.get_stddev() << std::endl;
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        std::cerr << " matde:     " << q.first->matde << " of " << this->m_mean_matde << ':' << this->m_mean_matde.get_stddev() << std::endl;
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
      q.first->t0 = q.first->credit;

    (this->*target)(value_list);

    const double inverse = 1.0 - this->m_credit_assignment_epsilon;
    for(const auto &q : value_list)
      q.first->credit = this->m_credit_assignment_epsilon * q.first->credit + inverse * q.first->t0;
  }

  void Agent::assign_credit_random(const Q_Value_List &value_list) {
    int32_t count = 0;
    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE)
        ++count;
    }

    count = random.rand_lt(count) + 1;

    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE)
        q.first->credit = --count ? 0.0 : 1.0;
      else
        q.first->credit = m_fringe_learning_scale;
    }
  }

  void Agent::assign_credit_specific(const Q_Value_List &value_list) {
    tracked_ptr<Q_Value> last;
    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE) {
        q.first->credit = 0.0;
        last = q.first;
      }
      else
        q.first->credit = m_fringe_learning_scale;
    }

    if(last)
      last->credit = 1.0;
  }

  void Agent::assign_credit_evenly(const Q_Value_List &value_list) {
    double count = double();
    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE)
        ++count;
    }

    for(const auto &q : value_list)
      q.first->credit = q.first->type == Q_Value::Type::FRINGE ? m_fringe_learning_scale : 1.0 / count;
  }

  void Agent::assign_credit_all(const Q_Value_List &value_list) {
    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE)
        q.first->credit = 1.0;
      else
        q.first->credit = m_fringe_learning_scale;
    }
  }

  void Agent::assign_credit_inv_update_count(const Q_Value_List &value_list) {
    double sum = double();
    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE) {
        q.first->credit = 1.0 / q.first->update_count;
        sum += q.first->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void Agent::assign_credit_inv_log_update_count(const Q_Value_List &value_list) {
    double sum = double();
    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE) {
        q.first->credit = 1.0 / (std::log(double(q.first->update_count)) / this->m_credit_assignment_log_base_value + 1.0);
        sum += q.first->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void Agent::assign_credit_inv_root_update_count(const Q_Value_List &value_list) {
    double sum = double();
    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE) {
        q.first->credit = 1.0 / std::pow(double(q.first->update_count), this->m_credit_assignment_root_value);
        sum += q.first->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void Agent::assign_credit_inv_depth(const Q_Value_List &value_list) {
    size_t depth = 0;
    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE)
        ++depth;
    }

    double sum = double();
    for(const auto &q : value_list) {
      if(q.first->type != Q_Value::Type::FRINGE) {
        q.first->credit = 1.0 / std::pow(2.0, double(--depth));
        sum += q.first->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void Agent::assign_credit_normalize(const Q_Value_List &value_list, const double &sum) {
    if(m_credit_assignment_normalize || sum > 1.0) {
      for(const auto &q : value_list) {
        if(q.first->type == Q_Value::Type::FRINGE)
          q.first->credit = m_fringe_learning_scale;
        else
          q.first->credit /= sum;
      }
    }
  }

  Fringe_Values::iterator Agent::split_test_catde(Node_Unsplit &general) {
    assert(general.q_value_weight);
    assert(general.q_value_weight->type != Q_Value::Type::SPLIT);

    /// Must obey the value function cap unless below the minimum split depth
    if(general.q_value_weight->depth >= m_split_min && m_value_function_cap && q_value_count >= m_value_function_cap)
      return general.fringe_values.end();

    if(m_value_function_map_mode == "in") {
      for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
        for(auto &fringe : fringe_axis->second) {
          std::ostringstream oss;
          fringe.lock()->rete_action.lock()->output_name(oss, -1);
          if(m_value_function_map.find(oss.str()) != m_value_function_map.end())
            return fringe_axis;
        }
      }
    }

    /// Classic CATDE criterion
//    if(!(m_mean_catde_queue_size ? m_mean_catde_queue.mean() : m_mean_catde).outlier_above(general.q_value->catde, m_split_catde + m_split_catde_qmult * q_value_count))
//      return nullptr;

    assert(general.q_value_weight->depth < m_split_max);
    assert(!general.fringe_values.empty());
    assert(general.rete_action.lock()->is_active());
    Fringe_Values::iterator chosen_axis = general.fringe_values.end();
    Node_Fringe_Ptr chosen;
    size_t count = 0;
    size_t matches = 0;
//    std::cerr << general.fringe_values.size() << std::endl;
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
//      std::cerr << "  " << fringe_axis->second.size() << std::endl;
      for(auto &fringe_w : fringe_axis->second) {
        auto fringe = fringe_w.lock();
        if(!fringe->rete_action.lock()->is_active())
          continue;
        ++matches;

        double boost = 1.0;
        if(m_resplit_bias == "blacklist") {
          if(general.fringe_axis_selections.find(fringe->q_value_fringe->feature) != general.fringe_axis_selections.end())
            continue;
        }
        else if(m_resplit_bias == "boost") {
          const auto fast = general.fringe_axis_selections.find(fringe->q_value_fringe->feature);
          if(fast != general.fringe_axis_selections.end())
            boost += fast->second * m_resplit_boost_scale;
        }

        if(general.q_value_weight->depth < m_split_min ||
          (fringe->q_value_fringe->pseudoepisode_count >= m_split_pseudoepisodes &&
          (fringe->q_value_fringe->update_count >= m_split_update_count &&
          (m_mean_catde_queue_size ? m_mean_catde_queue.mean() : m_mean_catde).outlier_above(fringe->q_value_fringe->catde * boost, m_split_catde + m_split_catde_qmult * q_value_count))))
        {
//#ifdef DEBUG_OUTPUT
//          std::cerr << " matches: " << *fringe->q_value->feature << std::endl;
//#endif
          const int64_t depth_diff = chosen ? fringe->q_value_fringe->feature->get_depth() - chosen->q_value_fringe->feature->get_depth() : 0;
          const double catde_diff = chosen ? fringe->q_value_fringe->catde - chosen->q_value_fringe->catde : 0.0;
          if(!chosen || depth_diff < 0 || (depth_diff == 0 && catde_diff < 0))
            count = 1;
          else if(depth_diff > 0 || catde_diff > 0 || random.frand_lt() >= 1.0 / ++count)
            continue;

          chosen_axis = fringe_axis;
          chosen = fringe;
        }
//#ifdef DEBUG_OUTPUT
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
        /// HOG Check
        bool hog = true;
        for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
          for(auto &fringe : fringe_axis->second) {
            if(fringe.lock()->q_value_fringe->feature->arity == 0)
              hog = false;
          }
        }

        if(!hog) {
          std::cerr << "WARNING: No feature in the fringe matches the current token for " << general.rete_action.lock()->get_name() << "!" << std::endl;

          for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
            for(auto &fringe : fringe_axis->second) {
              if(fringe.lock()->q_value_fringe->feature->arity == 0) {
                fringe.lock()->rete_action.lock()->print_rule(std::cerr);
              }
            }
          }

          dump_rules(*this);

#if !defined(NDEBUG) && defined(_WINDOWS)
          __debugbreak();
#elif !defined(NDEBUG)
          assert(false);
//#else
          abort();
#endif
        }
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
    assert(general.q_value_weight);
    assert(general.q_value_weight->type != Q_Value::Type::SPLIT);

    /// Must obey the value function cap unless below the minimum split depth
    if(general.q_value_weight->depth >= m_split_min && m_value_function_cap && q_value_count >= m_value_function_cap)
      return general.fringe_values.end();

    assert(general.q_value_weight->depth < m_split_max);
    assert(!general.fringe_values.empty());
    size_t matches = 0;
    std::unordered_set<tracked_ptr<Feature>> touched;
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
      for(auto &fringe_w : fringe_axis->second) {
        auto fringe = fringe_w.lock();

        if(!fringe->rete_action.lock()->is_active())
          continue;
        ++matches;

        if(general.q_value_weight->depth >= m_split_min &&
          (fringe->q_value_fringe->pseudoepisode_count < m_split_pseudoepisodes ||
           fringe->q_value_fringe->update_count < m_split_update_count))
        {
          return general.fringe_values.end(); ///< Wait to gather more data
        }

        if(m_resplit_bias == "blacklist" && general.fringe_axis_selections.find(fringe->q_value_fringe->feature) != general.fringe_axis_selections.end())
          continue;

        touched.insert(fringe_axis->first);
        break;
      }
    }

    const auto greedies = choose_greedies(nullptr, std::numeric_limits<int64_t>::max());
    Fringe_Values::iterator chosen_axis = general.fringe_values.end();
    double chosen_score = 0.0;
    int64_t count = 0;
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
      if(touched.find(fringe_axis->first) == touched.end())
        continue;

      double boost = 0.0;
      if(m_resplit_bias == "boost") {
        const auto fast = general.fringe_axis_selections.find(fringe_axis->first);
        if(fast != general.fringe_axis_selections.end())
          boost += fast->second * m_resplit_boost_scale;
      }

      auto new_greedies = choose_greedies(fringe_axis->second.begin()->lock().get(), std::numeric_limits<int64_t>::max());
      int64_t removed = 0, unchanged = 0;
      for(auto greedy : greedies) {
        const auto found = std::find(new_greedies.begin(), new_greedies.end(), greedy);
        if(found == new_greedies.end())
          ++removed;
        else {
          ++unchanged;
          new_greedies.erase(found);
        }
      }
      const int64_t added = int64_t(new_greedies.size());
      const double new_score = (added + removed) / (2.0 * unchanged + added + removed) + boost;

      if(new_score > chosen_score)
        count = 1;
      else if(new_score < chosen_score || random.frand_lt() >= 1.0 / ++count)
        continue;

#ifdef DEBUG_OUTPUT
      std::cerr << "Greedy mismatch for axis: ";
      fringe_axis->first->print_axis(std::cerr);
      std::cerr << " by ";
      if(chosen_axis != general.fringe_values.end())
        chosen_axis->first->print_axis(std::cerr);
      else
        std::cerr << "nullptr";
      std::cerr << std::endl;
#endif

      chosen_axis = fringe_axis;
      chosen_score = new_score;
    }

    if(chosen_axis == general.fringe_values.end()) {
      if(!matches) {
        /// HOG Check
        bool hog = true;
        for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
          for(auto &fringe : fringe_axis->second) {
            if(fringe.lock()->q_value_fringe->feature->arity == 0)
              hog = false;
          }
        }

        if(!hog) {
          std::cerr << "WARNING: No feature in the fringe matches the current token for " << general.rete_action.lock()->get_name() << "!" << std::endl;

          for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
            for(auto &fringe : fringe_axis->second) {
              if(fringe.lock()->q_value_fringe->feature->arity == 0) {
                fringe.lock()->rete_action.lock()->print_rule(std::cerr);
              }
            }
          }

          dump_rules(*this);

#if !defined(NDEBUG) && defined(_WINDOWS)
          __debugbreak();
#elif !defined(NDEBUG)
          assert(false);
//#else
          abort();
#endif
        }
      }

      return general.fringe_values.end();
    }

    return chosen_axis;
  }

  Fringe_Values::iterator Agent::split_test_value(Node_Unsplit &general) {
    assert(general.q_value_weight);
    assert(general.q_value_weight->type != Q_Value::Type::SPLIT);

    /// Must obey the value function cap unless below the minimum split depth
    if(general.q_value_weight->depth >= m_split_min && m_value_function_cap && q_value_count >= m_value_function_cap)
      return general.fringe_values.end();

    assert(general.q_value_weight->depth < m_split_max);
    assert(!general.fringe_values.empty());
    size_t matches = 0;
    std::unordered_set<tracked_ptr<Feature>> touched;
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
      for(auto &fringe_w : fringe_axis->second) {
        auto fringe = fringe_w.lock();

        if(!fringe->rete_action.lock()->is_active())
          continue;
        ++matches;

        if(general.q_value_weight->depth >= m_split_min &&
          (fringe->q_value_fringe->pseudoepisode_count < m_split_pseudoepisodes ||
           fringe->q_value_fringe->update_count < m_split_update_count))
        {
          return general.fringe_values.end(); ///< Wait to gather more data
        }

        if(m_resplit_bias == "blacklist" && general.fringe_axis_selections.find(fringe->q_value_fringe->feature) != general.fringe_axis_selections.end())
          continue;

        touched.insert(fringe_axis->first);
        break;
      }
    }

    Fringe_Values::iterator chosen_axis = general.fringe_values.end();
    size_t count = 0;
    int64_t depth_min = std::numeric_limits<int64_t>::max();
    double delta_max = std::numeric_limits<double>::lowest();
    for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
      if(touched.find(fringe_axis->first) == touched.end())
        continue;

      double boost = 1.0;
      if(m_resplit_bias == "boost") {
        const auto fast = general.fringe_axis_selections.find(fringe_axis->first);
        if(fast != general.fringe_axis_selections.end())
          boost += fast->second * m_resplit_boost_scale;
      }

      double lowest = std::numeric_limits<double>::max();
      double highest = std::numeric_limits<double>::lowest();
      for(auto &fringe_w : fringe_axis->second) {
        auto fringe = fringe_w.lock();
        lowest = std::min(lowest, fringe->q_value_fringe->primary);
        highest = std::max(highest, fringe->q_value_fringe->primary);
      }
      const double value_delta_max = (highest - lowest) * boost;

      int64_t depth_diff = fringe_axis->first->get_depth() - depth_min;
      double delta_diff = delta_max - value_delta_max;
      if(chosen_axis == general.fringe_values.end() || depth_diff < 0 || (depth_diff == 0 && delta_diff < 0))
        count = 1;
      else if(depth_diff > 0 || delta_diff > 0 || random.frand_lt() >= 1.0 / ++count)
        continue;

      chosen_axis = fringe_axis;
      depth_min = fringe_axis->first->get_depth();
      delta_max = value_delta_max;
    }

    if(chosen_axis == general.fringe_values.end()) {
      if(!matches) {
        /// HOG Check
        bool hog = true;
        for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
          for(auto &fringe : fringe_axis->second) {
            if(fringe.lock()->q_value_fringe->feature->arity == 0)
              hog = false;
          }
        }

        if(!hog) {
          std::cerr << "WARNING: No feature in the fringe matches the current token for " << general.rete_action.lock()->get_name() << "!" << std::endl;

          for(auto fringe_axis = general.fringe_values.begin(), fend = general.fringe_values.end(); fringe_axis != fend; ++fringe_axis) {
            for(auto &fringe : fringe_axis->second) {
              if(fringe.lock()->q_value_fringe->feature->arity == 0) {
                fringe.lock()->rete_action.lock()->print_rule(std::cerr);
              }
            }
          }

          dump_rules(*this);

#if !defined(NDEBUG) && defined(_WINDOWS)
          __debugbreak();
#elif !defined(NDEBUG)
          assert(false);
//#else
          abort();
#endif
        }
      }

      return general.fringe_values.end();
    }

    return chosen_axis;
  }

  Fringe_Values::iterator Agent::split_test_utile(Node_Unsplit &general) {
    return general.fringe_values.end();
  }

  bool Agent::unsplit_test_catde(Node_Split &general) {
    assert(general.q_value_weight);
    assert(general.q_value_weight->type == Q_Value::Type::SPLIT);
//    assert(!general.children.empty());
    assert(!general.blacklist_full);

    if(general.children.empty())
      return false;

    double boost_general = 1.0;
    double boost_children = 1.0;
    if(m_resplit_bias == "boost") {
      auto fast = general.q_value_fringe->feature ? general.fringe_axis_selections.find(general.q_value_fringe->feature) : general.fringe_axis_selections.end();
      if(fast != general.fringe_axis_selections.end())
        boost_general += fast->second * m_resplit_boost_scale;
      fast = general.fringe_axis_selections.find((*general.children.begin()).lock()->q_value_fringe->feature);
      if(fast != general.fringe_axis_selections.end())
        boost_children += fast->second * m_resplit_boost_scale;
    }

    double sum_error = 0.0;
    for(auto &child : general.children)
      sum_error += child.lock()->q_value_fringe->catde;

    const double improvement = general.q_value_fringe->catde_post_split * boost_general - sum_error * boost_children;

#ifdef DEBUG_OUTPUT
    std::cerr << "CATDE Improvement = " << general.q_value_fringe->catde_post_split << " - " << sum_error << " = " << improvement << std::endl;
#endif

    /// Counterintuitive: actually unsplit if error is reduced in the children?
    return improvement > 0.0 && general.q_value_fringe->update_count > m_unsplit_update_count;
  }

  bool Agent::unsplit_test_policy(Node_Split &general) {
    assert(general.q_value_weight);
    assert(general.q_value_weight->type == Q_Value::Type::SPLIT);
//    assert(!general.children.empty());
    assert(!general.blacklist_full);

//    if(general.children.empty())
//      return false;
    if(general.fringe_values.empty())
      return false;

    if(general.q_value_fringe->update_count < m_unsplit_update_count)
      return false;

    bool update_count_passed = false;
    for(auto &child : general.children) {
      if(child.lock()->q_value_fringe->update_count >= m_unsplit_update_count)
        update_count_passed = true;
    }

    if(!update_count_passed)
      return false;

    const int64_t fringe_depth = general.fringe_values.begin()->second.begin()->lock().get()->q_value_fringe->depth;
    const auto greedies = choose_greedies(nullptr, fringe_depth);

    double chosen_score = 0.0;
    {
      double boost = 0.0;
      if(m_resplit_bias == "boost") {
        const auto fast = general.fringe_axis_selections.find((*general.children.begin()).lock()->q_value_fringe->feature);
        if(fast != general.fringe_axis_selections.end())
          boost += fast->second * m_resplit_boost_scale;
      }

      auto new_greedies = choose_greedies(nullptr, std::numeric_limits<int64_t>::max());
      int64_t removed = 0, unchanged = 0;
      for(auto greedy : greedies) {
        const auto found = std::find(new_greedies.begin(), new_greedies.end(), greedy);
        if(found == new_greedies.end())
          ++removed;
        else {
          ++unchanged;
          new_greedies.erase(found);
        }
      }
      const int64_t added = int64_t(new_greedies.size());

      chosen_score = (added + removed) / (2.0 * unchanged + added + removed) + boost;
    }

    double max_score = 0.0;
    for(auto &fringe_axis : general.fringe_values) {
      double boost = 0.0;
      if(m_resplit_bias == "boost") {
        const auto fast = general.fringe_axis_selections.find(fringe_axis.first);
        if(fast != general.fringe_axis_selections.end())
          boost += fast->second * m_resplit_boost_scale;
      }

      auto new_greedies = choose_greedies(fringe_axis.second.begin()->lock().get(), fringe_depth);
      int64_t removed = 0, unchanged = 0;
      for(auto greedy : greedies) {
        const auto found = std::find(new_greedies.begin(), new_greedies.end(), greedy);
        if(found == new_greedies.end())
          ++removed;
        else {
          ++unchanged;
          new_greedies.erase(found);
        }
      }
      const int64_t added = int64_t(new_greedies.size());
      const double new_score = (added + removed) / (2.0 * unchanged + added + removed) + boost;

      max_score = std::max(max_score, new_score);
    }

    const double improvement = max_score - chosen_score;

#ifdef DEBUG_OUTPUT
    std::cerr << "Policy Improvement = " << max_score << " - " << chosen_score << " = " << improvement << ')' << std::endl;
#endif

    return improvement > 0.0;
  }

  bool Agent::unsplit_test_value(Node_Split &general) {
    assert(general.q_value_weight);
    assert(general.q_value_weight->type == Q_Value::Type::SPLIT);
//    assert(!general.children.empty());
    assert(!general.blacklist_full);

    if(general.children.empty())
      return false;

    if(general.q_value_fringe->update_count < m_unsplit_update_count)
      return false;

    double boost_general = 1.0;
    if(m_resplit_bias == "boost") {
      const auto fast = general.fringe_axis_selections.find((*general.children.begin()).lock()->q_value_fringe->feature);
      if(fast != general.fringe_axis_selections.end())
        boost_general += fast->second * m_resplit_boost_scale;
    }

    bool update_count_passed = false;
    std::pair<double, double> child_range = std::make_pair(std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest());
    for(auto &child_w : general.children) {
      auto child = child_w.lock();
      if(child->q_value_fringe->update_count >= m_unsplit_update_count)
        update_count_passed = true;
      const auto cr = child->value_range();
      child_range.first = std::min(child_range.first, cr.first);
      child_range.second = std::max(child_range.second, cr.second);
    }

    if(!update_count_passed)
      return false;

    double fringe_spread = 0.0;
    for(auto &fringe_axis : general.fringe_values) {
      double boost_fringe = 1.0;
      if(m_resplit_bias == "boost") {
        const auto fast = general.fringe_axis_selections.find(fringe_axis.first);
        if(fast != general.fringe_axis_selections.end())
          boost_fringe += fast->second * m_resplit_boost_scale;
      }

      std::pair<double, double> fringe_range = std::make_pair(std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest());
      for(auto &fringe_w : fringe_axis.second) {
        auto fringe = fringe_w.lock();
        fringe_range.first = std::min(fringe_range.first, fringe->q_value_fringe->primary);
        fringe_range.second = std::max(fringe_range.second, fringe->q_value_fringe->primary);
      }
      fringe_spread = std::max(fringe_spread, (fringe_range.second - fringe_range.first) * boost_fringe);
    }

    const double improvement = fringe_spread - (child_range.second - child_range.first) * boost_general;

#ifdef DEBUG_OUTPUT
    std::cerr << "Value Improvement = " << fringe_spread
              << " - (" << child_range.second << " - " << child_range.first << " = " << improvement << ')' << std::endl;
#endif

    return improvement > 0.0;
  }

  bool Agent::unsplit_test_utile(Node_Split &general) {
    assert(general.q_value_weight);
    assert(general.q_value_weight->type == Q_Value::Type::SPLIT);

    if(general.children.empty() || general.blacklist_full || general.q_value_fringe->update_count < m_unsplit_update_count)
      return false;

    return random.rand_lt(100) != 0;
  }

  std::tuple<double, double, int64_t> Agent::sum_value(const action_type * const &
#ifdef DEBUG_OUTPUT
                                                       action
#endif
                                                     , const Q_Value_List &value_list,
                                                       const Feature * const &axis,
                                                       const int64_t &fringe_depth) const {
#ifdef DEBUG_OUTPUT
    if(action) {
      std::cerr.unsetf(std::ios_base::floatfield);
      std::cerr << "   sum_value(" << *action << ") = " << value_list.size() << " {";
    }
#endif

  //    assert((&value_list - static_cast<Q_Value::List *>(nullptr)) > ptrdiff_t(sizeof(Q_Value)));

    double sum = double();
    double sum_variance = double();
//    int64_t avg_update_count = std::numeric_limits<int64_t>::max();
    int64_t min_update_count = std::numeric_limits<int64_t>::max();
    size_t touched = 0u;
    for(auto &q : value_list) {
      const bool fringe_contribution = axis && ((fringe_depth == std::numeric_limits<int64_t>::max() && !q.first->type_internal) || q.first->depth == fringe_depth);
      const bool normal_contribution = q.first->type != Q_Value::Type::FRINGE && q.first->depth <= fringe_depth;
      const bool any_contribution = fringe_contribution || normal_contribution;
#ifdef DEBUG_OUTPUT
      if(action && (!axis || any_contribution)) {
        std::cerr << ' ' << q.first->primary << ';' << q.first->primary_variance << ':' << q.first->depth;
        if(q.first->type == Q_Value::Type::FRINGE)
          std::cerr << (q.first->type_internal ? 'i' : 'f');
        if(q.first->feature)
          std::cerr << ':' << *q.first->feature;
      }
#endif
      if(any_contribution) {
        sum += q.first->primary /* * q.first->weight */;
        sum_variance += q.first->primary_variance;
//        avg_update_count *= touched / (touched + 1.0);
        min_update_count = std::min(min_update_count, q.first->update_count);
        ++touched;
//        avg_update_count += q.first->update_count / double(touched);
      }
    }

    const double stddev = sqrt(sum_variance);

//#ifdef DEBUG_OUTPUT
//    std::cerr << std::endl << "Touched ";
//    if(axis)
//      std::cerr << *axis;
//    else
//      std::cerr << 0;
//    std::cerr << '?' << std::endl;
//#endif
#ifdef DEBUG_OUTPUT
    if(action)
      std::cerr << " } = ";

    if(!(touched || value_list.empty())) {
      dump_rules(*this);
      assert(touched || value_list.empty());
//      avg_update_count = 0;
      min_update_count = 0;
    }

    if(action) {
      std::cerr << sum << " + " << stddev << " = " << sum + stddev << std::endl;
      std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
    }
#endif

    return std::make_tuple(sum, stddev, min_update_count);
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
    m_rete_nodes_evaluated.clear();

    generate_features();

#ifndef NDEBUG
      Node_Tracker::get().validate(*this, nullptr);
#endif

    while(!m_nodes_activating.empty()) {
//#ifndef NDEBUG
//      Node_Tracker::get().validate(*this);
//#endif

      const auto node = m_nodes_activating.front();
      m_nodes_active.push_back(node);
      m_nodes_activating.pop_front();
      Rete::Agenda::Locker lock(agenda);
//      std::cerr << "Testing action " << node->rete_action.lock()->get_name() << std::endl;
//      assert(node->rete_action.lock()->is_active());
      node->decision();
    }

#ifndef NDEBUG
    Node_Tracker::get().validate(*this, nullptr);
#endif

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
