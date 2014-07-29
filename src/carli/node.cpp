#include "node.h"

#include "agent.h"

namespace Carli {

  Node::~Node() {
    if(delete_q_value) {
      agent.purge_q_value(q_value);
      agent.purge_q_value_eligible(q_value);
      q_value.delete_and_zero();
    }
  }

  int64_t Node::rank() const {
    return q_value->depth;
  }

  void Node::retraction(Agent &agent, const Rete::WME_Token &token) {
    agent.purge_q_value_next(agent.get_action(*variables, token), q_value);
  }

  Node_Split_Ptr Node::create_split(Agent &agent, const Rete::WME_Ptr_C &wme_blink, const bool &terminal) {
    const auto ra_lock = rete_action.lock();
    const auto node_name = ra_lock->get_name();
    const auto new_name = agent.next_rule_name(node_name.substr(0, node_name.find_last_of('*') + 1) + "s");
    auto parent = ra_lock->parent_left();
    tracked_ptr<Q_Value> new_q_value;

    if(q_value->type == Q_Value::Type::FRINGE) {
      if(!terminal) {
        const auto filter_blink = agent.make_filter(*wme_blink);
        parent = agent.make_existential_join(Rete::WME_Bindings(), false, parent, filter_blink);
      }

      new_q_value = new Q_Value(0.0, Q_Value::Type::SPLIT, q_value->depth, q_value->feature ? q_value->feature->clone() : nullptr);
    }
    else {
      assert(!terminal);

      delete_q_value = false;
      q_value->type = Q_Value::Type::SPLIT;
      new_q_value = q_value;
    }

    auto new_leaf = agent.make_standard_action(parent, new_name, false, ra_lock->get_variables());
    auto new_leaf_data = std::make_shared<Node_Split>(agent, parent_action.lock(), new_leaf, new_q_value, terminal);
    new_leaf->data = new_leaf_data;

    return new_leaf_data;
  }

  Node_Unsplit_Ptr Node::create_unsplit(Agent &agent, const Rete::WME_Ptr_C &wme_blink) {
    const auto ra_lock = rete_action.lock();
    const auto node_name = ra_lock->get_name();
    const auto new_name = agent.next_rule_name(node_name.substr(0, node_name.find_last_of('*') + 1) + "u");
    auto parent = ra_lock->parent_left();
    tracked_ptr<Q_Value> new_q_value;

    if(q_value->type == Q_Value::Type::FRINGE ||
      (q_value->type == Q_Value::Type::SPLIT && debuggable_cast<Node_Split *>(this)->terminal))
    {
      auto filter_blink = agent.make_filter(*wme_blink);
      parent = agent.make_existential_join(Rete::WME_Bindings(), false, parent, filter_blink);
    }

    if(q_value->type == Q_Value::Type::FRINGE)
      new_q_value = new Q_Value(0.0, Q_Value::Type::UNSPLIT, q_value->depth, q_value->feature ? q_value->feature->clone() : nullptr);
    else {
      delete_q_value = false;
      q_value->type = Q_Value::Type::UNSPLIT;
      new_q_value = q_value;
    }

    auto new_leaf = agent.make_standard_action(parent, new_name, false, ra_lock->get_variables());
    auto new_leaf_data = std::make_shared<Node_Unsplit>(agent, parent_action.lock(), new_leaf, new_q_value);
    new_leaf->data = new_leaf_data;

    return new_leaf_data;
  }

  Node_Fringe_Ptr Node::create_fringe(Agent &agent, Node_Unsplit &leaf, Feature * const &feature) {
    const auto lra_lock = leaf.rete_action.lock();
    const auto leaf_node_name = lra_lock->get_name();
    const auto new_feature = feature ? feature : q_value->feature ? q_value->feature->clone() : nullptr;
    const auto new_name = agent.next_rule_name(leaf_node_name.substr(0, leaf_node_name.find_last_of('*') + 1) + "f");
    const auto feature_ranged_data = dynamic_cast<Feature_Ranged_Data *>(new_feature);

    auto ancestor_left = lra_lock->parent_left();
    if(leaf.q_value->type != Q_Value::Type::FRINGE) {
      assert(leaf.q_value->type == Q_Value::Type::UNSPLIT || !debuggable_cast<Node_Split &>(leaf).terminal);
      ancestor_left = ancestor_left->parent_left(); ///< Skip blink node
    }
    else
      assert(leaf.q_value->depth == q_value->depth);

    const auto ra_lock = rete_action.lock();
    Rete::Rete_Node_Ptr ancestor_right = ra_lock->parent_left();

    if(leaf.q_value->type != Q_Value::Type::FRINGE) {
      /// Collapsing rather the fringe rather than expanding it
      if(q_value->type == Q_Value::Type::UNSPLIT ||
        (q_value->type == Q_Value::Type::SPLIT && !debuggable_cast<Node_Split *>(this)->terminal))
      {
        ancestor_right = ancestor_right->parent_left(); ///< Skip blink node
      }

      const int64_t dend = std::max(int64_t(2), leaf.q_value->depth);
      for(int64_t d = q_value->depth; d != dend; --d) {
        assert(dynamic_cast<Rete::Rete_Predicate *>(ancestor_right.get()) ||
          dynamic_cast<Rete::Rete_Existential_Join *>(ancestor_right.get()));
        ancestor_right = ancestor_right->parent_right(); ///< Pass through extra tests
      }
    }

    Rete::Rete_Node_Ptr new_test;
    auto old_variables = lra_lock->get_variables();
    Rete::Variable_Indices_Ptr new_variables;

    if(feature_ranged_data && feature) {
      /// Ranged feature -- refining of an existing variable
      new_test = agent.make_predicate_vc(feature_ranged_data->predicate(), new_feature->axis, feature_ranged_data->symbol_constant(), ancestor_left);
    }
    else {
      if(leaf.q_value->depth == 1) {
        /// Case 1. Trivial -- use the original node
        new_test = ancestor_right;
      }
      else if(lra_lock->get_token_owner() == ra_lock->get_token_owner()) {
        /// Case 2. No new conditions to carry over
        new_test = agent.make_existential_join(Rete::WME_Bindings(), true, ancestor_left, ancestor_right);
      }
      else {
        bool ancestor_found = false;
        for(auto left = ancestor_left; !dynamic_cast<Rete::Rete_Filter *>(left.get()); left = left->parent_left()) {
          if(left->parent_left()->get_token_owner() == ancestor_right->get_token_owner()) {
            ancestor_found = true;
            break;
          }
        }

        if(ancestor_found) {
          /// Case 3: No new conditions to carry over, but must do a more involved token comparison
          Rete::WME_Bindings bindings;
          for(const auto &variable : *variables) { ///< NOTE: Assume all are potentially multivalued
            const auto found = old_variables->find(variable.first);
            assert(found != old_variables->end());
            bindings.insert(Rete::WME_Binding(found->second, variable.second));
          }

          new_test = agent.make_existential_join(bindings, false, ancestor_left, ancestor_right);
        }
        else {
          /// Case 4: New conditions must be joined, new variables are assumed
          const auto join = dynamic_cast<Rete::Rete_Join *>(ancestor_right.get());
          assert(join);
          new_test = agent.make_join(q_value->feature->bindings, ancestor_left, join->parent_right());

          /// Essentially a ptrdiff to go from the the new conditions on the right token to left
          const int64_t offset = new_test->get_token_size() - join->get_token_size();

          for(const auto &variable : *variables) {
            const auto found = old_variables->find(variable.first);
            if(found == old_variables->end()) {
              if(!new_variables)
                new_variables = std::make_shared<Rete::Variable_Indices>(*old_variables);
              const auto new_index = Rete::WME_Token_Index(variable.second.first + offset, variable.second.second);
              (*new_variables)[variable.first] = new_index;
            }
          }

          new_feature->axis.first += offset;
        }
      }
    }

    /// Create the actual action for the new fringe node
    auto new_action = agent.make_standard_action(new_test, new_name, false, new_variables ? new_variables : old_variables);
    auto new_action_data = std::make_shared<Node_Fringe>(agent, lra_lock, new_action, leaf.q_value->depth + 1, new_feature);
    new_action->data = new_action_data;

    /// Add to the appropriate parent list
    leaf.fringe_values[new_action_data->q_value->feature.get()].values.push_back(new_action_data);

    return new_action_data;
  }

  Node_Split::Node_Split(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_, const bool &terminal_)
    : Node(agent_, parent_action_, rete_action_, q_value_), terminal(terminal_)
  {
    assert(q_value_->type == Q_Value::Type::SPLIT);
    ++agent.q_value_count;
  }

  Node_Split::~Node_Split() {
    --agent.q_value_count;
  }

  Rete::Rete_Node_Ptr Node_Split::cluster_root_ancestor() const {
    return terminal ? rete_action.lock()->parent_left() : rete_action.lock()->parent_left()->parent_left();
  }

  void Node_Split::action(Agent &agent, const Rete::WME_Token &token) {
    if(!rete_action.lock()->is_excised() && (terminal || !agent.respecialize(*rete_action.lock())))
      agent.insert_q_value_next(agent.get_action(*variables, token), q_value);
  }

  Node_Unsplit::Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_)
    : Node(agent_, parent_action_, rete_action_, new Q_Value(0.0, Q_Value::Type::UNSPLIT, depth_, feature_))
  {
    ++agent.q_value_count;
  }

  Node_Unsplit::Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_)
    : Node(agent_, parent_action_, rete_action_, q_value_)
  {
    assert(q_value_->type == Q_Value::Type::UNSPLIT);
    ++agent.q_value_count;
  }

  Node_Unsplit::~Node_Unsplit() {
    --agent.q_value_count;
  }

  Rete::Rete_Node_Ptr Node_Unsplit::cluster_root_ancestor() const {
    return rete_action.lock()->parent_left()->parent_left();
  }

  void Node_Unsplit::action(Agent &agent, const Rete::WME_Token &token) {
    if(!rete_action.lock()->is_excised() && !agent.specialize(*rete_action.lock(), token))
      agent.insert_q_value_next(agent.get_action(*variables, token), q_value);
  }

  void Node_Fringe::action(Agent &agent, const Rete::WME_Token &token) {
    if(!rete_action.lock()->is_excised())
      agent.insert_q_value_next(agent.get_action(*variables, token), q_value);
  }

  Rete::Rete_Node_Ptr Node_Fringe::cluster_root_ancestor() const {
    return rete_action.lock()->parent_left()->parent_left();
  }

}
