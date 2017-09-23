#include "node.h"

#include "agent.h"

namespace Carli {

  Fringe_Axis_Selections::~Fringe_Axis_Selections() {
    while(!empty()) {
      auto bt = begin();
      auto ptr = bt->first;
      erase(bt);
      ptr.delete_and_zero();
    }
  }

#ifndef NDEBUG
  Node_Tracker & Node_Tracker::get() {
    static Node_Tracker node_tracker;
    return node_tracker;
  }

  size_t Node_Tracker::num_nodes() const {
    return m_names_from_nodes.size();
  }

  void Node_Tracker::create(const Rete::Rete_Action &action) {
    const Node * const node = debuggable_cast<const Node *>(action.data.get());
    const std::string node_name = action.get_name();

    assert(m_names_from_nodes.find(node) == m_names_from_nodes.end());
    assert(m_nodes_from_names.find(node_name) == m_nodes_from_names.end());
//    std::cerr << "Registering " << node_name << std::endl;
    m_names_from_nodes[node] = node_name;
    m_nodes_from_names[node_name] = node;
  }

  void Node_Tracker::destroy(const Node &node) {
    const auto found = m_names_from_nodes.find(&node);
    assert(found != m_names_from_nodes.end());
    const auto found2 = m_nodes_from_names.find(found->second);
    assert(found2 != m_nodes_from_names.end());
//    std::cerr << "Unregistering " << found->second << std::endl;
    m_names_from_nodes.erase(found);
    m_nodes_from_names.erase(found2);
  }

  void Node_Tracker::validate(Rete::Rete_Agent &agent, const Node * const ignore) {
    const auto rule_names = agent.get_rule_names();

    for(const auto &name : rule_names) {
//      std::cerr << "Validating " << name << std::endl;
      auto action = agent.get_rule(name);
      if(const Node * const node = dynamic_cast<const Node *>(action->data.get())) {
        assert(node->q_value_fringe->depth == 1 || !node->parent_action.expired());
        assert(!node->rete_action.expired());
        if(const Node_Split * const node_split = dynamic_cast<const Node_Split *>(node)) {
          assert(m_names_from_nodes.find(node_split) != m_names_from_nodes.end());
          for(auto child : node_split->children)
            assert(!child.expired());
          for(auto fringe : node_split->fringe_values) {
            for(auto node2 : fringe.second)
              assert(!node2.expired());
          }
        }
        else if(const Node_Unsplit * const node_unsplit = dynamic_cast<const Node_Unsplit *>(node)) {
          assert(m_names_from_nodes.find(node_unsplit) != m_names_from_nodes.end());
          for(auto fringe : node_unsplit->fringe_values) {
            for(auto node2 : fringe.second)
              assert(!node2.expired());
          }
        }
      }
    }

    for(auto name_from_node : m_names_from_nodes) {
      if(name_from_node.first == ignore)
        continue;
//      std::cerr << name_from_node.second << " in rete?" << std::endl;
      assert(rule_names.find(name_from_node.second) != rule_names.end());
      assert(name_from_node.first->q_value_fringe->depth == 1 || !name_from_node.first->parent_action.expired());
      assert(!name_from_node.first->rete_action.expired());
    }

    ++m_good_validations;
  }
#endif

  Node::~Node() {
    const auto pa_lock = parent_action.lock();
    if(pa_lock) {
      if(const auto split = dynamic_cast<Node_Split *>(pa_lock->data.get())) {
//        const auto found = std::find_if(split->children.begin(), split->children.end(), [this](const Node_Ptr_W &node){return node.lock().get() == this;});
        const auto found = std::find_if(split->children.begin(), split->children.end(), [this](const Node_Ptr_W &node){return node.expired();});
        if(found != split->children.end()) {
//#ifndef NDEBUG
//          std::cerr << "Erasing child node from " << split->rete_action.lock()->get_name() << std::endl;
//#endif
          split->children.erase(found);
        }
//#ifndef NDEBUG
//        else {
//          std::cerr << "Children of " << pa_lock->get_name() << std::endl;
//          for(auto child : split->children)
//            std::cerr << "  " << child.lock() << std::endl;
//          std::cerr << "this is " << this << std::endl;
//        }
////        assert(found != split->children.end());
//#endif

        //const auto found2 = split->fringe_values.find(q_value_fringe->feature.get());
        //if(found2 != split->fringe_values.end()) {
        //  const auto found3 = std::find_if(found2->second.begin(), found2->second.end(), [this](const Node_Fringe_Ptr &value) {return value.get() == this; });
        //  if(found3 != found2->second.end())
        //    found2->second.erase(found3);
        //}
      }
      //else if(const auto unsplit = dynamic_cast<Node_Unsplit *>(pa_lock->data.get())) {
      //  const auto found = unsplit->fringe_values.find(q_value_fringe->feature.get());
      //  if(found != unsplit->fringe_values.end()) {
      //    const auto found2 = std::find_if(found->second.begin(), found->second.end(), [this](const Node_Fringe_Ptr &value) {return value.get() == this; });
      //    if(found2 != found->second.end())
      //      found->second.erase(found2);
      //  }
      //}
    }

    if(delete_q_value_weight && q_value_weight) {
      agent.purge_q_value(q_value_weight);
      agent.purge_q_value_eligible(q_value_weight);
      q_value_weight.delete_and_zero();
    }
    if(delete_q_value_fringe) {
      agent.purge_q_value(q_value_fringe);
      agent.purge_q_value_eligible(q_value_fringe);
      q_value_fringe.delete_and_zero();
    }
  }

  int64_t Node::rank() const {
    return q_value_fringe->depth;
  }

  std::list<Node_Ptr> Node::descendants() {
    std::list<Node_Ptr> rv;
    descendants(rv);
    return rv;
  }

  Q_Value::Token Node::sum_value(Q_Value::Token value_accumulator) const
  {
    const auto pa_lock = parent_action.lock();

    if(q_value_weight) {
      value_accumulator = Q_Value::Token(std::get<0>(value_accumulator) + q_value_weight->primary,
                                         std::get<1>(value_accumulator) + q_value_weight->primary_mean2,
                                         std::get<2>(value_accumulator) + q_value_weight->primary_variance,
                                         std::get<3>(value_accumulator) + q_value_weight->secondary,
                                         std::min(std::get<4>(value_accumulator), q_value_weight->update_count));
    }

    if(pa_lock)
      return dynamic_cast<Node *>(pa_lock->data.get())->sum_value(value_accumulator);
    return value_accumulator;
  }

  std::pair<double, double> Node::value_range(const bool &include_ancestors) const {
    std::pair<double, double> child_values;

    if(const auto split = dynamic_cast<const Node_Split *>(this)) {
      child_values.first = std::numeric_limits<double>::max();
      child_values.second = std::numeric_limits<double>::lowest();

      for(auto &fringe_axis : split->fringe_values) {
        for(auto &fringe : fringe_axis.second) {
          const auto cv = fringe.lock()->value_range(false);
          child_values.first = std::min(child_values.first, cv.first);
          child_values.second = std::max(child_values.second, cv.second);
        }
      }
    }

    double ancestors = q_value_weight ? q_value_weight->primary : 0.0;

    if(include_ancestors) {
      for(auto pa_lock = parent_action.lock(); pa_lock; ) {
        const auto pan = dynamic_cast<Node *>(pa_lock->data.get());
        ancestors += pan->q_value_weight ? pan->q_value_weight->primary : 0.0;
        pa_lock = pan->parent_action.lock();
      }
    }

    child_values.first += ancestors;
    child_values.second += ancestors;

    return child_values;
  }

  void Node::print_flags(std::ostream &os) const {
    const auto &q_value = q_value_weight ? q_value_weight : q_value_fringe;

    os << std::endl << "  :creation-time " << q_value->creation_time;
    os << std::endl << "  :feature " << q_value->depth << ' ';

    if(dynamic_cast<const Node_Fringe *>(this))
      os << "fringe";
    else if(dynamic_cast<const Node_Split *>(this))
      os << "split";
    else
      os << "unsplit";

    if(q_value->feature && q_value->feature->arity > -1)
      os << ' ' << q_value->feature->arity;

    if(q_value->depth == 1)
      os << " nil";
    else {
      const auto pal = parent_action.lock();
      assert(pal);
      os << ' ' << (pal ? pal->get_name() : "nil");
    }

    if(const auto feature_ranged = dynamic_cast<const Feature_Ranged_Data *>(q_value->feature.get())) {
      os << ' ' << feature_ranged->depth << ' ';

      if(feature_ranged->integer_locked)
        os << int64_t(feature_ranged->bound_lower) << ' ' << int64_t(feature_ranged->bound_upper);
      else
        os << Rete::to_string(feature_ranged->bound_lower) << ' ' << Rete::to_string(feature_ranged->bound_upper);
    }
  }

  void Node::print_action(std::ostream &os) const {
    const auto &q_value = q_value_weight ? q_value_weight : q_value_fringe;

    os << "  = " << Rete::to_string(q_value->primary) << ' ' << Rete::to_string(q_value->primary_mean2) << ' ' << Rete::to_string(q_value->primary_variance)
       << ' ' << Rete::to_string(q_value->secondary) << ' ' << q_value->update_count;
  }

  Rete::Rete_Node_Ptr_C Node::get_suppress() const {
    return agent.terse_out ? parent_action.lock() : nullptr;
  }

  void Node::action(const Rete::WME_Token &token) {
    const auto sft = shared_from_this();

    auto na = agent.m_nodes_active.find(sft);
    if(na != agent.m_nodes_active.end())
      ++na->second;
    else {
      na = agent.m_nodes_activating.find(sft);
      if(na != agent.m_nodes_activating.end())
        ++na->second;
      else
        agent.m_nodes_activating[sft] = 1;
    }

    if(q_value_weight)
      agent.insert_q_value_next(agent.get_action(*variables, token), q_value_weight);
    if(q_value_fringe)
      agent.insert_q_value_next(agent.get_action(*variables, token), q_value_fringe);

//    if(q_value_fringe->depth != 1 && !parent_action.lock()) {
//      std::cerr << "Expired parent action found for " << rete_action.lock()->get_name() << std::endl;
//      abort();
//    }
  }

  void Node::retraction(const Rete::WME_Token &token) {
    const auto sft = shared_from_this();

    auto na = agent.m_nodes_activating.find(sft);
    if(na != agent.m_nodes_activating.end()) {
      if(--na->second == 0)
        agent.m_nodes_activating.erase(na);
    }
    else {
      na = agent.m_nodes_active.find(sft);
      if(na != agent.m_nodes_active.end()) {
        if(--na->second == 0)
          agent.m_nodes_active.erase(na);
      }
    }

    if(q_value_weight)
      agent.purge_q_value_next(agent.get_action(*variables, token), q_value_weight);
    if(q_value_fringe)
      agent.purge_q_value_next(agent.get_action(*variables, token), q_value_fringe);

//    if(q_value_fringe->depth != 1 && !parent_action.lock()) {
//      std::cerr << "Expired parent action found for " << rete_action.lock()->get_name() << std::endl;
//      abort();
//    }
  }

  Node_Split_Ptr Node::create_split(const Rete::Rete_Action_Ptr &parent_action_) {
    const auto ra_lock = rete_action.lock();
    const auto node_name = ra_lock->get_name();
    tracked_ptr<Q_Value> new_q_value_weight;

    const size_t asterisk_p1 = node_name.find_last_of('*') + 1;
    std::string new_name = node_name.substr(0, asterisk_p1) + "s";
    {
      auto tail = node_name.substr(asterisk_p1 + 1);
      std::ostringstream oss;
      oss << atoi(tail.c_str());
      if(oss.str() == tail)
        new_name += tail;
      else
        new_name = agent.next_rule_name(new_name);
    }

    if(!q_value_weight) {
//      const auto summed = sum_value();
      new_q_value_weight = new Q_Value(Q_Value::Token(),
                                Q_Value::Type::SPLIT, q_value_fringe->depth, q_value_fringe->feature ? q_value_fringe->feature->clone() : nullptr, agent.get_total_step_count());
      q_value_fringe->update_count = 0;
      q_value_fringe->catde = 0.0;
      assert(new_q_value_weight->primary_mean2 >= 0.0);
    }
    else {
      delete_q_value_weight = false;
      q_value_weight->type = Q_Value::Type::SPLIT;
      new_q_value_weight = q_value_weight;
    }

    q_value_fringe->catde_post_split = 0.0;
    delete_q_value_fringe = false;

    auto new_leaf = agent.make_standard_action(ra_lock->parent_left(), new_name, false, ra_lock->get_variables());
    auto new_leaf_data = std::make_shared<Node_Split>(agent, parent_action_, new_leaf, new_q_value_weight, q_value_fringe);
    new_leaf->data = new_leaf_data;

    if(q_value_weight) {
      new_leaf_data->fringe_axis_selections.swap(debuggable_cast<Node_Unsplit *>(this)->fringe_axis_selections);
      new_leaf_data->fringe_axis_counter = debuggable_cast<Node_Unsplit *>(this)->fringe_axis_counter;
    }

    /// Add to the appropriate parent list
    if(parent_action_) {
//#ifndef NDEBUG
//      std::cerr << "Adding child node link from " << parent_action_->get_name() << " to " << this << std::endl;
//#endif
      debuggable_pointer_cast<Node_Split>(parent_action_->data)->children.push_back(new_leaf_data);
    }

#ifndef NDEBUG
    Node_Tracker::get().create(*new_leaf);
#endif

    return new_leaf_data;
  }

  Node_Unsplit_Ptr Node::create_unsplit(const Rete::Rete_Action_Ptr &parent_action_) {
    const auto ra_lock = rete_action.lock();
    const auto node_name = ra_lock->get_name();
    tracked_ptr<Q_Value> new_q_value_weight;

    const size_t asterisk_p1 = node_name.find_last_of('*') + 1;
    std::string new_name = node_name.substr(0, asterisk_p1) + "u";
    {
      auto tail = node_name.substr(asterisk_p1 + 1);
      std::ostringstream oss;
      oss << atoi(tail.c_str());
      if(oss.str() == tail)
        new_name += tail;
      else
        new_name = agent.next_rule_name(new_name);
    }

    if(!q_value_weight) {
//      const auto summed = sum_value();
      new_q_value_weight = new Q_Value(Q_Value::Token(),
                                Q_Value::Type::UNSPLIT, q_value_fringe->depth, q_value_fringe->feature ? q_value_fringe->feature->clone() : nullptr, agent.get_total_step_count());
      q_value_fringe->update_count = 0;
    }
    else {
      delete_q_value_weight = false;
      q_value_weight->type = Q_Value::Type::UNSPLIT;
      new_q_value_weight = q_value_weight;
    }

    q_value_fringe->catde = 0.0;
    q_value_fringe->catde_post_split = 0.0;
    delete_q_value_fringe = false;

    auto new_leaf = agent.make_standard_action(ra_lock->parent_left(), new_name, false, ra_lock->get_variables());
    auto new_leaf_data = std::make_shared<Node_Unsplit>(agent, parent_action_, new_leaf, new_q_value_weight, q_value_fringe);
    new_leaf->data = new_leaf_data;

    if(q_value_weight) {
      new_leaf_data->fringe_axis_selections.swap(debuggable_cast<Node_Split *>(this)->fringe_axis_selections);
      new_leaf_data->fringe_axis_counter = debuggable_cast<Node_Split *>(this)->fringe_axis_counter;
    }

    /// Add to the appropriate parent list
    if(parent_action_) {
//#ifndef NDEBUG
//      std::cerr << "Adding child node link from " << parent_action_->get_name() << " to " << this << std::endl;
//#endif
      debuggable_pointer_cast<Node_Split>(parent_action_->data)->children.push_back(new_leaf_data);
    }

#ifndef NDEBUG
    Node_Tracker::get().create(*new_leaf);
#endif

    return new_leaf_data;
  }

  Node_Fringe_Ptr Node::create_fringe(Node_Unsplit &leaf, Feature * const &feature_, const Grammar &grammar, const Rete::WME_Token_Index &old_new_var_index_, const Rete::WME_Token_Index &prev_var_index_) {
    Rete::WME_Token_Index old_new_var_index = old_new_var_index_;
    Rete::WME_Token_Index prev_var_index = prev_var_index_;

    const auto lra_lock = leaf.rete_action.lock();
    const auto ra_lock = rete_action.lock();
    const auto ancestor_left = lra_lock->parent_left();
    const auto ancestor_right = ra_lock->parent_left();
    const auto leaf_node_name = lra_lock->get_name();
    const auto new_feature = feature_ ? feature_ : q_value_fringe->feature ? q_value_fringe->feature->clone() : nullptr;
    const auto new_name = agent.next_rule_name(leaf_node_name.substr(0, leaf_node_name.find_last_of('*') + 1) + "f");
    const auto feature_enumerated_data = dynamic_cast<Feature_Enumerated_Data *>(new_feature);
    const auto feature_ranged_data = dynamic_cast<Feature_Ranged_Data *>(new_feature);

    if(leaf.q_value_weight)
      assert(leaf.q_value_weight->type == Q_Value::Type::UNSPLIT);
    else
      assert(leaf.q_value_fringe->depth == q_value_fringe->depth);

    std::stack<Rete::Rete_Node_Ptr> rebase_right;
    if(grammar == GRAMMAR_NULL_HOG) {
      rebase_right.push(ancestor_right);
      auto node = ancestor_right->parent_right();
      do {
        rebase_right.push(node);
        node = node->parent_left();
      } while(!dynamic_cast<Rete::Rete_Join *>(node.get()));
      rebase_right.push(node);
    }
    else {
      for(auto node = ancestor_right; node != parent_action.lock()->parent_left(); node = node->parent_left()) {
        rebase_right.push(node);
        if(dynamic_cast<Rete::Rete_Filter *>(node.get())) {
          std::cerr << "Ancestral relationship failure: " << ra_lock->get_name() << std::endl;
//          const std::string rules_out_file = dynamic_cast<const Option_String &>(Options::get_global()["rules-out"]).get_value();
//          if(!rules_out_file.empty()) {
//            std::ofstream rules_out((rules_out_file + ".bak").c_str());
//            rules_out << "set-total-step-count " << agent.get_total_step_count() << std::endl << std::endl;
//            agent.rete_print_rules(rules_out);
//          }
          abort();
        }
      }
    }

    assert(grammar == GRAMMAR_NORMAL || old_new_var_index.rete_row != -1);

#ifdef DEBUG_OUTPUT
    std::cerr << new_name << " is " << (grammar == GRAMMAR_HOG ? "" : grammar == GRAMMAR_NULL_HOG ? "null " : "not ") << "HOG" << std::endl;
#endif

    /// Handle HOG
    std::string old_new_var_name;
    Rete::WME_Token_Index new_new_var_index(-1, old_new_var_index.token_row + 1, old_new_var_index.column);
    new_new_var_index.existential = old_new_var_index.existential;
    std::string new_new_var_name;
    Rete::Rete_Node_Ptr new_test = ancestor_left;
    Rete::WME_Token_Index null_hog_left_old_new_var_index(-1, -1, -1);
    if(grammar != GRAMMAR_NORMAL) {
      old_new_var_name = std::find_if(new_feature->indices->begin(), new_feature->indices->end(), [&old_new_var_index](const std::pair<std::string, Rete::WME_Token_Index> &si)->bool {
//        std::cerr << si.first << ',' << si.second << " =? " << old_new_var_index << std::endl;
        return si.second == old_new_var_index;
      })->first;

#ifdef DEBUG_OUTPUT
      std::cerr << "Old variable: " << old_new_var_name << " at " << old_new_var_index << std::endl;
#endif

      {
        size_t last_hyphen_p1 = old_new_var_name.rfind('-') + 1;
        std::ostringstream oss;
        oss << old_new_var_name.substr(0, last_hyphen_p1) << atoi(old_new_var_name.substr(last_hyphen_p1).c_str()) + 1;
        new_new_var_name = oss.str();
      }

      new_new_var_index.rete_row = old_new_var_index.rete_row + rebase_right.size();
      if(grammar == GRAMMAR_NULL_HOG)
        --new_new_var_index.rete_row;

#ifdef DEBUG_OUTPUT
      std::cerr << "New variable: " << new_new_var_name << " at " << new_new_var_index << std::endl;
#endif

      Rete::WME_Bindings bindings;
      for(auto binding : new_feature->bindings) {
        if(binding.first == old_new_var_index)
          binding.first = new_new_var_index;
        if(binding.second == old_new_var_index)
          binding.second = new_new_var_index;
        bindings.insert(binding);
      }
      new_feature->bindings = std::move(bindings);
      if(new_feature->axis == old_new_var_index)
        new_feature->axis = new_new_var_index;
      auto indices = std::make_shared<Rete::Variable_Indices>(*new_feature->indices);
      if(grammar == GRAMMAR_NULL_HOG) {
        auto gp = ancestor_right->parent_left();
        auto nnvi = new_new_var_index;
        nnvi.rete_row += ancestor_left->get_size() - gp->get_size();
        nnvi.token_row += ancestor_left->get_token_size() - gp->get_token_size();

        auto vars = lra_lock->get_variables();
        null_hog_left_old_new_var_index = vars->find(old_new_var_name)->second;
        indices->find(old_new_var_name)->second = null_hog_left_old_new_var_index;
        indices->insert(std::make_pair(new_new_var_name, nnvi));

#ifdef DEBUG_OUTPUT
        std::cerr << "Changing feature " << dynamic_cast<Feature_NullHOG_Data *>(new_feature)->value << " to " << new_new_var_name << std::endl;
#endif
        dynamic_cast<Feature_NullHOG_Data *>(new_feature)->value = new_new_var_name;
      }
      else
        indices->insert(std::make_pair(new_new_var_name, new_new_var_index));
      new_feature->indices = indices;
    }

#ifdef DEBUG_OUTPUT
    std::cerr << "Parent feature ";
    if(leaf.q_value_fringe->feature)
      std::cerr << *leaf.q_value_fringe->feature;
    else
      std::cerr << 0;
    std::cerr << ", depth " << leaf.q_value_fringe->depth << std::endl;

    std::cerr << "Creating fringe node for " << *new_feature << std::endl;
#endif

    const auto old_variables = lra_lock->get_variables();
    Rete::Variable_Indices_Ptr new_variables;

    Rete::Rete_Node_Ptr_C ancestor_rightmost = ancestor_right;
    if(feature_ranged_data && feature_ranged_data->depth > 1) {
      while(!dynamic_cast<const Rete::Rete_Filter *>(ancestor_rightmost.get()) &&
            !dynamic_cast<const Rete::Rete_Predicate *>(ancestor_rightmost.get()))
      {
        ancestor_rightmost = ancestor_rightmost->parent_right();
      }
    }

    if(grammar == GRAMMAR_NORMAL && dynamic_cast<const Rete::Rete_Predicate *>(ancestor_rightmost.get())) {
      assert(feature_enumerated_data || feature_ranged_data);
      /// Case 1. Refining of an existing variable
#ifdef DEBUG_OUTPUT
      std::cerr << "Fringe Case 1" << std::endl;
#endif
      if(feature_enumerated_data)
        new_test = agent.make_predicate_vc(feature_enumerated_data->get_predicate(), new_feature->axis, feature_enumerated_data->symbol_constant(), ancestor_left);
      else
        new_test = agent.make_predicate_vc(feature_ranged_data->get_predicate(), new_feature->axis, feature_ranged_data->symbol_constant(), ancestor_left);
    }
    else {
//      assert(dynamic_cast<const Rete::Rete_Join *>(ancestor_right.get()) ||
//             dynamic_cast<const Rete::Rete_Existential_Join *>(ancestor_right.get()) ||
//             dynamic_cast<const Rete::Rete_Negation_Join *>(ancestor_right.get()));
//      assert(leaf.q_value->depth == 1 || ancestor_right->get_bindings());

//      else if(lra_lock->get_token_owner() == ra_lock->get_token_owner()) {
//        /// Case 2. No new conditions to carry over
//        new_test = agent.make_existential_join(*ancestor_right->get_bindings(), false, ancestor_left, ancestor_right->parent_right());
//      }

      if(grammar == GRAMMAR_NORMAL && *variables == *old_variables) {
#ifdef DEBUG_OUTPUT
        std::cerr << "Fringe Case 2" << std::endl;
#endif
        while(rebase_right.size() > 1)
          rebase_right.pop();
      }
      else {
#ifdef DEBUG_OUTPUT
        std::cerr << "Fringe Case 3" << std::endl;
        std::cerr << "Rebase node count = " << rebase_right.size() << std::endl;
#endif
      }

      const int64_t rebase_rete_offset = lra_lock->parent_left()->get_size() - rebase_right.top()->parent_left()->get_size();
      const int64_t rebase_token_offset = lra_lock->parent_left()->get_token_size() - rebase_right.top()->parent_left()->get_token_size();
      const int64_t rebase_offset_from = rebase_right.top()->parent_left()->get_size();

#ifdef DEBUG_OUTPUT
      std::cerr << "Offsets are " << rebase_rete_offset << " && " << rebase_token_offset << " starting at " << rebase_offset_from << std::endl;
#endif

      int64_t null_hog_offset = 0;
      int64_t null_hog_token_offset = 0;
      if(grammar == GRAMMAR_NULL_HOG) {
        new_test = ancestor_right->parent_right();

#ifdef DEBUG_OUTPUT
        std::cerr << "Originals: " << old_new_var_index << " " << new_new_var_index << std::endl;
#endif
        auto gp = ancestor_right->parent_left();
        null_hog_offset = gp->get_size();
        null_hog_token_offset = gp->get_token_size();
        old_new_var_index.rete_row -= null_hog_offset;
        old_new_var_index.token_row -= null_hog_token_offset;
        old_new_var_index.existential = false;
        new_new_var_index.rete_row -= null_hog_offset;
        new_new_var_index.token_row -= null_hog_token_offset;
        new_new_var_index.existential = false;
#ifdef DEBUG_OUTPUT
        std::cerr << "Replacements: " << old_new_var_index << " " << new_new_var_index << std::endl;
#endif
      }

      assert(!rebase_right.empty());
      while(!rebase_right.empty()) {
        auto test = rebase_right.top();
        rebase_right.pop();

        /// New exclusion between old_new and new_new for HOG
        if(grammar != GRAMMAR_NORMAL && rebase_right.empty()) {
#ifdef DEBUG_OUTPUT
          std::cerr << "  HOG Predicate" << std::endl;
#endif

          new_test = agent.make_predicate_vv(Rete::Rete_Predicate::Predicate::NEQ, new_new_var_index, old_new_var_index, new_test);
        }

        if(auto bindings = test->get_bindings()) {
          Rete::WME_Bindings updated_bindings;
          auto bindings_ptr = grammar != GRAMMAR_NORMAL ? &updated_bindings : bindings;
          if(grammar != GRAMMAR_NORMAL) {
            for(auto binding : *test->get_bindings()) {
              if(binding.first == old_new_var_index)
                binding.first = new_new_var_index;
              else if(rebase_right.empty() && binding.first == prev_var_index)
                binding.first = old_new_var_index;
              if(binding.second == old_new_var_index)
                binding.second = new_new_var_index;
              else if(rebase_right.empty() && binding.second == prev_var_index)
                binding.second = old_new_var_index;
              updated_bindings.insert(binding);
            }
          }
          else {
            for(auto binding : *test->get_bindings()) {
              if(binding.first.rete_row >= rebase_offset_from) {
                bindings_ptr = &updated_bindings;
                binding.first.rete_row += rebase_rete_offset;
                binding.first.token_row += rebase_token_offset;
              }
              updated_bindings.insert(binding);
            }
          }

          if(dynamic_cast<Rete::Rete_Join *>(test.get())) {
#ifdef DEBUG_OUTPUT
            std::cerr << "  Join" << std::endl;
#endif
            new_test = agent.make_join(*bindings_ptr, new_test, test->parent_right());
          }
          else if(dynamic_cast<Rete::Rete_Existential_Join *>(test.get())) {
#ifdef DEBUG_OUTPUT
            std::cerr << "  Existential Join" << std::endl;
#endif
            new_test = agent.make_existential_join(*bindings_ptr, new_test, test->parent_right());
          }
          else if(dynamic_cast<Rete::Rete_Negation_Join *>(test.get())) {
#ifdef DEBUG_OUTPUT
            std::cerr << "  Negation Join" << std::endl;
#endif
            if(grammar == GRAMMAR_NULL_HOG && rebase_right.empty()) {
              updated_bindings.insert(Rete::WME_Binding(null_hog_left_old_new_var_index, old_new_var_index));
              new_test = agent.make_negation_join(*bindings_ptr, ancestor_left, new_test);
            }
            else
              new_test = agent.make_negation_join(*bindings_ptr, new_test, test->parent_right());
          }
        }
        else if(auto predicate_node = dynamic_cast<Rete::Rete_Predicate *>(test.get())) {
#ifdef DEBUG_OUTPUT
          std::cerr << "  Predicate" << std::endl;
#endif
          if(grammar != GRAMMAR_NORMAL) {
            auto lhs_index = predicate_node->get_lhs_index();
            if(lhs_index == old_new_var_index)
              lhs_index = new_new_var_index;
            else if((grammar == GRAMMAR_NULL_HOG || rebase_right.empty()) && lhs_index == prev_var_index)
              lhs_index = old_new_var_index;

            if(predicate_node->get_rhs())
              new_test = agent.make_predicate_vc(predicate_node->get_predicate(), lhs_index, predicate_node->get_rhs(), new_test);
            else {
              auto rhs_index = predicate_node->get_rhs_index();
              if(rhs_index == old_new_var_index)
                rhs_index = new_new_var_index;
              else if((grammar == GRAMMAR_NULL_HOG || rebase_right.empty()) && rhs_index == prev_var_index)
                rhs_index = old_new_var_index;

#ifdef DEBUG_OUTPUT
              std::cerr << "New Predicate Indices: " << lhs_index << " " << rhs_index << std::endl;
#endif
              new_test = agent.make_predicate_vv(predicate_node->get_predicate(), lhs_index, rhs_index, new_test);
            }
          }
          else {
            auto new_lhs_index = predicate_node->get_lhs_index();
            if(new_lhs_index.rete_row >= rebase_offset_from) {
              new_lhs_index.rete_row += rebase_rete_offset;
              new_lhs_index.token_row += rebase_token_offset;
            }
#ifdef DEBUG_OUTPUT
            if(new_lhs_index != predicate_node->get_lhs_index())
              std::cerr << "    Updated lhs_index from " << predicate_node->get_lhs_index() << " to " << new_lhs_index << std::endl;
#endif

            if(predicate_node->get_rhs())
              new_test = agent.make_predicate_vc(predicate_node->get_predicate(), new_lhs_index, predicate_node->get_rhs(), new_test);
            else
              new_test = agent.make_predicate_vv(predicate_node->get_predicate(), new_lhs_index, predicate_node->get_rhs_index(), new_test);
          }
        }
        else
          abort();
      }

//      for(auto left = ancestor_left; !dynamic_cast<Rete::Rete_Filter *>(left.get()); left = left->parent_left()) {
//        if(left->parent_left()->get_token_owner() == ancestor_right->get_token_owner())
//          break;
//      }

      /// New conditions are possible even without HOG
      if(grammar == GRAMMAR_NORMAL) {
        const int64_t leaf_size = ancestor_left->get_size();
        const int64_t leaf_token_size = ancestor_left->get_token_size();
        const int64_t old_size = ra_lock->get_size();
        const int64_t old_token_size = ra_lock->get_token_size();
        const int64_t new_size = new_test->get_size();
        const int64_t new_token_size = new_test->get_token_size();

#ifdef DEBUG_OUTPUT
        std::cerr << "Values: " << leaf_size << ' ' << leaf_token_size << ' ' << old_size << ' ' << old_token_size << ' ' << new_size << ' ' << new_token_size << std::endl;
#endif

        for(const auto &variable : *variables) {
#ifdef DEBUG_OUTPUT
          std::cerr << "Considering Variable '" << variable.first << "' at " << variable.second << std::endl;
#endif
          if(variable.second.rete_row < parent_action.lock()->get_size())
            continue;
          bool found_non_existential = false;
          {
            const auto found = old_variables->find(variable.first);
            if(found != old_variables->end() && !found->second.existential)
              found_non_existential = true;
          }
          if(!found_non_existential) {
            if(!new_variables)
              new_variables = std::make_shared<Rete::Variable_Indices>(*old_variables);
            auto new_index = variable.second;

#ifdef DEBUG_OUTPUT
            std::cerr << "new_index(" << variable.first << ") was " << new_index << std::endl;
#endif

            if(new_size >= old_size) {
              /// Offset forward
              new_index.rete_row += new_size - old_size;
#ifdef DEBUG_OUTPUT
              std::cerr << "new_index.rete_row offset forward " << new_size  << '-' << old_size << " = " << new_index << std::endl;
#endif
            }
            else if(new_index.rete_row >= leaf_size) {
              /// Offset backward
              new_index.rete_row -= old_size - new_size;
#ifdef DEBUG_OUTPUT
              std::cerr << "new_index.rete_row offset backward " << old_size << '-' << new_size << " = " << new_index << std::endl;
#endif
            }

            if(new_token_size >= old_token_size) {
              /// Offset forward
              new_index.token_row += new_token_size - old_token_size;
#ifdef DEBUG_OUTPUT
              std::cerr << "new_index.token_row offset forward " << new_token_size << '-' << old_token_size << " = " << new_index << std::endl;
#endif
            }
            else if(new_index.token_row >= leaf_token_size) {
              /// Offset backward

              new_index.token_row -= new_token_size - leaf_token_size;
#ifdef DEBUG_OUTPUT
              std::cerr << "new_index.token_row offset backward " << new_token_size << '-' << leaf_token_size << " = " << new_index << std::endl;
#endif
              if(new_index.token_row < leaf_token_size) {
#ifdef DEBUG_OUTPUT
                std::cerr << "new_index discarded" << std::endl;
#endif
                /// Discard intermediate fringe variables which no longer exist post-collapse
                continue;
              }
            }

            assert(new_index.rete_row > -1);
            assert(new_index.token_row > -1);
            assert(new_index.rete_row < new_size);
            assert(new_index.token_row <= new_index.rete_row);
            assert(new_index.existential || new_index.token_row < new_token_size);
            assert(std::find_if(new_variables->begin(), new_variables->end(), [new_index](const std::pair<std::string, Rete::WME_Token_Index> &ind){return ind.second == new_index;}) == new_variables->end());
            if(!new_index.existential) {
              const auto found = new_variables->equal_range(variable.first);
              for(auto ft = found.first; ft != found.second; ++ft) {
                assert(ft->second.existential);
                if(!ft->second.existential) {
                  std::cerr << "New variable conflicts with existing variable." << std::endl;
                  abort();
                }
              }
            }
            new_variables->insert(std::make_pair(variable.first, new_index));

            Rete::WME_Bindings new_bindings;
            for(auto binding : new_feature->bindings) {
              if(binding.first == variable.second) {
#ifdef DEBUG_OUTPUT
                std::cerr << "Update binding from " << binding.first << " to " << new_index << std::endl;
#endif
                new_bindings.insert(std::make_pair(new_index, binding.second));
              }
              else
                new_bindings.insert(binding);
            }
            new_feature->bindings = std::move(new_bindings);

#ifdef DEBUG_OUTPUT
            std::cerr << new_feature->bindings << std::endl;
            std::cerr << "new_index(" << variable.first << ") = " << new_index << std::endl;
#endif
          }
        }

//        if(ra_lock->parent_left()->get_token_size() > ra_lock->parent_left()->parent_left()->get_token_size()) {
//          offset -= ra_lock->parent_left()->get_token_size() - ra_lock->parent_left()->parent_left()->get_token_size();
//          offset += ra_lock->parent_left()->parent_right()->get_token_size();
//        }

        assert(new_feature->axis.rete_row > -2);
        assert(new_feature->axis.token_row > -2);
        assert(new_feature->axis.token_row <= new_feature->axis.rete_row);
        assert(new_feature->axis.existential || new_feature->axis.rete_row < old_size);
//        assert(new_feature->axis.existential || new_feature->axis.token_row < new_token_size);
        if(new_feature->axis.rete_row != -1) {
#ifdef DEBUG_OUTPUT
          std::cerr << "old_feature->axis = " << new_feature->axis << std::endl;
#endif
          const int64_t index_offset = new_size - old_size;
          const int64_t token_index_offset = new_token_size - old_token_size;
          new_feature->axis.rete_row = new_feature->axis.rete_row + index_offset;
          new_feature->axis.token_row = new_feature->axis.token_row + token_index_offset;
          assert(new_feature->axis.rete_row > -1);
          assert(new_feature->axis.token_row > -1);
#ifdef DEBUG_OUTPUT
          std::cerr << "new_feature->axis = " << new_feature->axis << std::endl;
          if(new_variables)
            assert(std::find_if(new_variables->begin(), new_variables->end(), [new_feature](const std::pair<std::string, Rete::WME_Token_Index> &ind){return ind.second == new_feature->axis;}) != new_variables->end());
          else
            assert(std::find_if(old_variables->begin(), old_variables->end(), [new_feature](const std::pair<std::string, Rete::WME_Token_Index> &ind){return ind.second == new_feature->axis;}) != old_variables->end());
#endif
        }
        assert(new_feature->axis.token_row <= new_feature->axis.rete_row);
        assert(new_feature->axis.rete_row < new_size);
        assert(new_feature->axis.existential || new_feature->axis.token_row < new_token_size);
      }
    }

    /// Fix feature
    if(grammar == GRAMMAR_NORMAL) {
      if(new_feature->axis.rete_row >= lra_lock->parent_left()->get_size())
        new_feature->axis.rete_row += ancestor_left->get_size() - lra_lock->parent_left()->get_size();
      if(new_feature->axis.token_row >= lra_lock->parent_left()->get_token_size())
        new_feature->axis.token_row += ancestor_left->get_token_size() - lra_lock->parent_left()->get_token_size();
      new_feature->indices = new_variables ? new_variables : old_variables;

#ifdef DEBUG_OUTPUT
      std::cerr << q_value_fringe->feature->axis << "-->" << new_feature->axis << " && " << *q_value_fringe->feature->indices << "-->" << *new_feature->indices << std::endl;
#endif
    }

    /// Create the actual action for the new fringe node
    auto new_action = agent.make_standard_action(new_test, new_name, false, new_feature->indices);
    auto new_action_data = std::make_shared<Node_Fringe>(agent, lra_lock, new_action, leaf.q_value_fringe->depth + 1, new_feature);
    new_action->data = new_action_data;

    /// Add to the appropriate parent list
    leaf.fringe_values[new_action_data->q_value_fringe->feature.get()].push_back(new_action_data);

#ifdef DEBUG_OUTPUT
    std::cerr << *new_feature->indices << std::endl;
    std::cerr << new_feature->axis << std::endl;
    new_feature->print_axis(std::cerr);
    std::cerr << std::endl;

    new_action->print_rule(std::cerr);
#endif

    if(auto bindings = new_action->parent_left()->get_bindings()) {
      auto vars = new_action->get_variables();
      for(auto binding : *bindings) {
        if(std::find_if(vars->begin(), vars->end(), [&binding](const std::pair<std::string, Rete::WME_Token_Index> &ind){return ind.second == binding.first;}) == vars->end()
//           || std::find_if(vars->begin(), vars->end(), [&binding](const std::pair<std::string, Rete::WME_Token_Index> &ind){return ind.second == binding.second;}) == vars->end()
          )
        {
          std::cerr << "BINDING FAILURE!!!" << std::endl;

          std::cerr << *new_feature->indices << std::endl;
          std::cerr << new_feature->axis << std::endl;
          new_feature->print_axis(std::cerr);
          std::cerr << std::endl;

          lra_lock->print_rule(std::cerr);
          ra_lock->print_rule(std::cerr);
          new_action->print_rule(std::cerr);

          std::cerr << binding << " of " << *bindings << std::endl;
          std::cerr << new_new_var_index << ", " << old_new_var_index << ", " << prev_var_index << std::endl;

          abort();
        }
      }
    }

    return new_action_data;
  }

  Node_Split::Node_Split(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_weight_, const tracked_ptr<Q_Value> &q_value_fringe_)
    : Node(agent_, parent_action_, rete_action_, q_value_weight_, q_value_fringe_)
  {
    assert(q_value_weight_->type == Q_Value::Type::SPLIT);
    ++agent.q_value_count;
    q_value_fringe->type_internal = true;
  }

  Node_Split::~Node_Split() {
    --agent.q_value_count;
#ifndef NDEBUG
    Node_Tracker::get().destroy(*this);
#endif
  }

  Rete::Rete_Node_Ptr Node_Split::cluster_root_ancestor() const {
    return rete_action.lock()->parent_left();
  }

  void Node_Split::descendants(std::list<Node_Ptr> &descendants_) {
    for(auto &child : children) {
      auto clock = child.lock();
      descendants_.push_back(clock);
      clock->descendants(descendants_);
    }
  }

  void Node_Split::decision() {
    const auto locked = rete_action.lock();
    if(locked && !locked->is_excised())
      agent.respecialize(*rete_action.lock());
  }

  Node_Unsplit::Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_)
    : Node(agent_, parent_action_, rete_action_, new Q_Value(Q_Value::Token(), Q_Value::Type::UNSPLIT, depth_, feature_, agent_.get_total_step_count()), new Q_Value(Q_Value::Token(), Q_Value::Type::FRINGE, depth_, feature_->clone(), agent_.get_total_step_count()))
  {
    ++agent.q_value_count;
    q_value_fringe->type_internal = true;
  }

  Node_Unsplit::Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_weight_, const tracked_ptr<Q_Value> &q_value_fringe_)
    : Node(agent_, parent_action_, rete_action_, q_value_weight_, q_value_fringe_)
  {
    assert(q_value_weight_->type == Q_Value::Type::UNSPLIT);
    ++agent.q_value_count;
    q_value_fringe->type_internal = true;
  }

  Node_Unsplit::~Node_Unsplit() {
    --agent.q_value_count;
#ifndef NDEBUG
    Node_Tracker::get().destroy(*this);
#endif
  }

  Rete::Rete_Node_Ptr Node_Unsplit::cluster_root_ancestor() const {
    return rete_action.lock()->parent_left();
  }

  void Node_Unsplit::descendants(std::list<Node_Ptr> &descendants_) {
    for(auto &fringe_axis : fringe_values) {
      for(auto &fringe : fringe_axis.second)
        descendants_.push_back(fringe.lock());
    }
  }

  void Node_Unsplit::decision() {
    const auto locked = rete_action.lock();
    if(locked && !locked->is_excised())
      agent.specialize(*rete_action.lock());
  }

  Node_Fringe::Node_Fringe(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_)
   : Node(agent_, parent_action_, rete_action_, nullptr, new Q_Value(dynamic_cast<Node *>(parent_action_->data.get())->sum_value(), Q_Value::Type::FRINGE, depth_, feature_, agent_.get_total_step_count()))
  {
    q_value_fringe->update_totals();
  }

  Rete::Rete_Node_Ptr Node_Fringe::cluster_root_ancestor() const {
    return rete_action.lock()->parent_left()->parent_left();
  }

}
