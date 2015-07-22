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

  Q_Value::Token Node::sum_value(Q_Value::Token value_accumulator) const
  {
    const auto pa_lock = parent_action.lock();

    if(q_value->type != Q_Value::Type::FRINGE) {
      value_accumulator = Q_Value::Token(std::get<0>(value_accumulator) + q_value->primary,
                                         std::get<1>(value_accumulator) + q_value->primary_mean2,
                                         std::get<2>(value_accumulator) + q_value->primary_variance,
                                         std::get<3>(value_accumulator) + q_value->secondary,
                                         std::min(std::get<4>(value_accumulator), q_value->update_count));
    }

    if(pa_lock)
      return dynamic_cast<Node *>(pa_lock->data.get())->sum_value(value_accumulator);
    return value_accumulator;
  }

  void Node::print_flags(std::ostream &os) const {
    os << std::endl << "  :creation-time " << q_value->creation_time;
    os << std::endl << "  :feature " << q_value->depth << ' ';

    if(dynamic_cast<const Node_Fringe *>(this))
      os << "fringe";
    else if(dynamic_cast<const Node_Split *>(this))
      os << "split";
    else
      os << "unsplit";

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
    os << "  = " << Rete::to_string(q_value->primary) << ' ' << Rete::to_string(q_value->primary_mean2) << ' ' << Rete::to_string(q_value->primary_variance)
       << ' ' << Rete::to_string(q_value->secondary) << ' ' << q_value->update_count;
  }

  void Node::action(const Rete::WME_Token &token) {
    agent.m_nodes_activating.push_back(shared_from_this());
    agent.insert_q_value_next(agent.get_action(*variables, token), q_value);
  }

  void Node::retraction(const Rete::WME_Token &token) {
    auto na = std::find(agent.m_nodes_active.begin(), agent.m_nodes_active.end(), shared_from_this());
    if(na != agent.m_nodes_active.end())
      agent.m_nodes_active.erase(na);

    na = std::find(agent.m_nodes_activating.begin(), agent.m_nodes_activating.end(), shared_from_this());
    if(na != agent.m_nodes_activating.end())
      agent.m_nodes_activating.erase(na);

    agent.purge_q_value_next(agent.get_action(*variables, token), q_value);
  }

  Node_Split_Ptr Node::create_split(const Rete::Rete_Action_Ptr &parent_action_) {
    const auto ra_lock = rete_action.lock();
    const auto node_name = ra_lock->get_name();
    const auto new_name = agent.next_rule_name(node_name.substr(0, node_name.find_last_of('*') + 1) + "s");
    tracked_ptr<Q_Value> new_q_value;

    if(q_value->type == Q_Value::Type::FRINGE) {
      const auto summed = sum_value();
      new_q_value = new Q_Value(Q_Value::Token(q_value->primary - std::get<0>(summed), q_value->primary_mean2 - std::get<1>(summed), q_value->primary_variance - std::get<2>(summed),
                                               q_value->secondary - std::get<3>(summed), std::min(q_value->update_count, std::get<4>(summed))),
                                Q_Value::Type::SPLIT, q_value->depth, q_value->feature ? q_value->feature->clone() : nullptr, agent.get_total_step_count());
    }
    else {
      delete_q_value = false;
      q_value->type = Q_Value::Type::SPLIT;
      new_q_value = q_value;
    }

    auto new_leaf = agent.make_standard_action(ra_lock->parent_left(), new_name, false, ra_lock->get_variables());
    auto new_leaf_data = std::make_shared<Node_Split>(agent, parent_action_, new_leaf, new_q_value);
    new_leaf->data = new_leaf_data;

    return new_leaf_data;
  }

  Node_Unsplit_Ptr Node::create_unsplit(const Rete::Rete_Action_Ptr &parent_action_) {
    const auto ra_lock = rete_action.lock();
    const auto node_name = ra_lock->get_name();
    const auto new_name = agent.next_rule_name(node_name.substr(0, node_name.find_last_of('*') + 1) + "u");
    tracked_ptr<Q_Value> new_q_value;

    if(q_value->type == Q_Value::Type::FRINGE) {
      const auto summed = sum_value();
      new_q_value = new Q_Value(Q_Value::Token(q_value->primary - std::get<0>(summed), q_value->primary_mean2 - std::get<1>(summed), q_value->primary_variance - std::get<2>(summed),
                                               q_value->secondary - std::get<3>(summed), std::min(q_value->update_count, std::get<4>(summed))),
                                Q_Value::Type::UNSPLIT, q_value->depth, q_value->feature ? q_value->feature->clone() : nullptr, agent.get_total_step_count());
    }
    else {
      delete_q_value = false;
      q_value->type = Q_Value::Type::UNSPLIT;
      new_q_value = q_value;
    }

    auto new_leaf = agent.make_standard_action(ra_lock->parent_left(), new_name, false, ra_lock->get_variables());
    auto new_leaf_data = std::make_shared<Node_Unsplit>(agent, parent_action_, new_leaf, new_q_value);
    new_leaf->data = new_leaf_data;

    return new_leaf_data;
  }

  Node_Fringe_Ptr Node::create_fringe(Node_Unsplit &leaf, Feature * const &feature_) {
    const auto lra_lock = leaf.rete_action.lock();
    const auto leaf_node_name = lra_lock->get_name();
    const auto new_feature = feature_ ? feature_ : q_value->feature ? q_value->feature->clone() : nullptr;
    const auto new_name = agent.next_rule_name(leaf_node_name.substr(0, leaf_node_name.find_last_of('*') + 1) + "f");
    const auto feature_enumerated_data = dynamic_cast<Feature_Enumerated_Data *>(new_feature);
    const auto feature_ranged_data = dynamic_cast<Feature_Ranged_Data *>(new_feature);

#ifndef NDEBUG
    std::cerr << "Parent feature ";
    if(leaf.q_value->feature)
      std::cerr << *leaf.q_value->feature;
    else
      std::cerr << 0;
    std::cerr << ", depth " << leaf.q_value->depth << std::endl;

    std::cerr << "Creating fringe node for " << *new_feature << std::endl;
#endif

    const auto ancestor_left = lra_lock->parent_left();
    if(leaf.q_value->type != Q_Value::Type::FRINGE)
      assert(leaf.q_value->type == Q_Value::Type::UNSPLIT);
    else
      assert(leaf.q_value->depth == q_value->depth);

    const auto ra_lock = rete_action.lock();
    Rete::Rete_Node_Ptr ancestor_right = ra_lock->parent_left();

//    if(leaf.q_value->type != Q_Value::Type::FRINGE) {
//      /// Collapsing rather the fringe rather than expanding it
//      const int64_t dend = std::max(int64_t(2), leaf.q_value->depth);
//      for(int64_t d = q_value->depth; d != dend; --d) {
//        assert(dynamic_cast<Rete::Rete_Predicate *>(ancestor_right.get()) ||
//          dynamic_cast<Rete::Rete_Existential_Join *>(ancestor_right.get()));
//        ancestor_right = ancestor_right->parent_right(); ///< Pass through extra tests
//      }
//    }

    Rete::Rete_Node_Ptr new_test;
    auto old_variables = lra_lock->get_variables();
    Rete::Variable_Indices_Ptr new_variables;

    Rete::Rete_Node_Ptr_C ancestor_rightmost = ancestor_right;
    if(feature_ranged_data && feature_ranged_data->depth > 1) {
      while(!dynamic_cast<const Rete::Rete_Filter *>(ancestor_rightmost.get()) &&
            !dynamic_cast<const Rete::Rete_Predicate *>(ancestor_rightmost.get()))
      {
        ancestor_rightmost = ancestor_rightmost->parent_right();
      }
    }

    if(dynamic_cast<const Rete::Rete_Predicate *>(ancestor_rightmost.get())) {
      assert(feature_enumerated_data || feature_ranged_data);
      /// Case 1. Refining of an existing variable
#ifndef NDEBUG
      std::cerr << "Fringe Case 1" << std::endl;
#endif
      if(feature_enumerated_data)
        new_test = agent.make_predicate_vc(feature_enumerated_data->predicate(), new_feature->axis, feature_enumerated_data->symbol_constant(), ancestor_left);
      else
        new_test = agent.make_predicate_vc(feature_ranged_data->predicate(), new_feature->axis, feature_ranged_data->symbol_constant(), ancestor_left);
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
      bool ancestor_found = false;
      for(auto left = ancestor_left; !dynamic_cast<Rete::Rete_Filter *>(left.get()); left = left->parent_left()) {
        if(left->parent_left()->get_token_owner() == ancestor_right->get_token_owner()) {
          ancestor_found = true;
          break;
        }
      }

      if(ancestor_found) {
        /// Case 2: No new conditions to carry over, but must do a more involved token comparison
#ifndef NDEBUG
        std::cerr << "Fringe Case 2" << std::endl;
#endif
//          Rete::WME_Bindings bindings;
//          for(const auto &variable : *variables) { ///< NOTE: Assume all are potentially multivalued
//            const auto found = old_variables->find(variable.first);
//            assert(found != old_variables->end());
//            bindings.insert(Rete::WME_Binding(found->second, variable.second));
//          }
//
//          new_test = agent.make_existential_join(bindings, false, ancestor_left, ancestor_right);

        if(dynamic_cast<const Rete::Rete_Existential_Join *>(ancestor_right.get()))
          new_test = agent.make_existential_join(*ancestor_right->get_bindings(), false, ancestor_left, ancestor_right->parent_right());
        else
          new_test = agent.make_negation_join(*ancestor_right->get_bindings(), ancestor_left, ancestor_right->parent_right());
      }
      else {
        /// Case 3: New conditions must be joined
#ifndef NDEBUG
        std::cerr << "Fringe Case 3" << std::endl;
#endif
        if(dynamic_cast<Rete::Rete_Join *>(ancestor_right.get())) {
#ifndef NDEBUG
          std::cerr << "  Join" << std::endl;
#endif
          new_test = agent.make_join(*ancestor_right->get_bindings(), ancestor_left, ancestor_right->parent_right());
        }
        else if(const auto existential_join = dynamic_cast<Rete::Rete_Existential_Join *>(ancestor_right.get())) {
#ifndef NDEBUG
          std::cerr << "  Existential Join" << std::endl;
#endif
          new_test = agent.make_existential_join(*ancestor_right->get_bindings(), existential_join->is_matching_tokens(), ancestor_left, ancestor_right->parent_right());
        }
        else if(dynamic_cast<Rete::Rete_Negation_Join *>(ancestor_right.get())) {
#ifndef NDEBUG
          std::cerr << "  Negation Join" << std::endl;
#endif
          new_test = agent.make_negation_join(*ancestor_right->get_bindings(), ancestor_left, ancestor_right->parent_right());
        }
        else
          abort();
      }

      /// New conditions are possible for cases 2 and 3
      {
        const int64_t leaf_size = ancestor_left->get_size();
        const int64_t leaf_token_size = ancestor_left->get_token_size();
        const int64_t old_size = ra_lock->get_size();
        const int64_t old_token_size = ra_lock->get_token_size();
        const int64_t new_size = new_test->get_size();
        const int64_t new_token_size = new_test->get_token_size();

#ifndef NDEBUG
        std::cerr << "Values: " << leaf_size << ' ' << leaf_token_size << ' ' << old_size << ' ' << old_token_size << ' ' << new_size << ' ' << new_token_size << std::endl;
#endif

        for(const auto &variable : *variables) {
#ifndef NDEBUG
          std::cerr << "Considering Variable '" << variable.first << "' at " << variable.second << std::endl;
#endif
          if(variable.second.rete_row < ra_lock->parent_left()->parent_left()->get_size())
            continue;
          bool found_non_existential = false;
          {
            const auto found = old_variables->equal_range(variable.first);
            std::find_if(found.first, found.second, [&found_non_existential,&old_token_size](const std::pair<std::string, Rete::WME_Token_Index> &variable)->bool {
              return found_non_existential = !variable.second.existential;
            });
          }
          if(!found_non_existential) {
            if(!new_variables)
              new_variables = std::make_shared<Rete::Variable_Indices>(*old_variables);
            auto new_index = variable.second;

#ifndef NDEBUG
            std::cerr << "new_index(" << variable.first << ") was " << new_index << std::endl;
#endif

            if(new_size >= old_size) {
              /// Offset forward
              new_index.rete_row += new_size - old_size;
#ifndef NDEBUG
              std::cerr << "new_index.rete_row offset forward " << new_size  << '-' << old_size << " = " << new_index << std::endl;
#endif
            }
            else if(new_index.rete_row >= leaf_size) {
              /// Offset backward
              new_index.rete_row -= old_size - new_size;
#ifndef NDEBUG
              std::cerr << "new_index.rete_row offset backward " << old_size << '-' << new_size << " = " << new_index << std::endl;
#endif
            }

            if(new_token_size >= old_token_size) {
              /// Offset forward
              new_index.token_row += new_token_size - old_token_size;
#ifndef NDEBUG
              std::cerr << "new_index.token_row offset forward " << new_token_size << '-' << old_token_size << " = " << new_index << std::endl;
#endif
            }
            else if(new_index.token_row >= leaf_token_size) {
              /// Offset backward
              new_index.token_row -= new_token_size - leaf_token_size;
#ifndef NDEBUG
              std::cerr << "new_index.token_row offset backward " << new_token_size << '-' << leaf_token_size << " = " << new_index << std::endl;
#endif
              if(new_index.token_row < leaf_token_size) {
#ifndef NDEBUG
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
#ifndef NDEBUG
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
#ifndef NDEBUG
          std::cerr << "old_feature->axis = " << new_feature->axis << std::endl;
#endif
          const int64_t index_offset = new_size - old_size;
          const int64_t token_index_offset = new_token_size - old_token_size;
          new_feature->axis.rete_row = new_feature->axis.rete_row + index_offset;
          new_feature->axis.token_row = new_feature->axis.token_row + token_index_offset;
          assert(new_feature->axis.rete_row > -1);
          assert(new_feature->axis.token_row > -1);
#ifndef NDEBUG
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
    if(new_feature->axis.rete_row >= lra_lock->parent_left()->get_size())
      new_feature->axis.rete_row += ancestor_left->get_size() - lra_lock->parent_left()->get_size();
    if(new_feature->axis.token_row >= lra_lock->parent_left()->get_token_size())
      new_feature->axis.token_row += ancestor_left->get_token_size() - lra_lock->parent_left()->get_token_size();
    new_feature->indices = new_variables ? new_variables : old_variables;

    /// Create the actual action for the new fringe node
    auto new_action = agent.make_standard_action(new_test, new_name, false, new_feature->indices);
    auto new_action_data = std::make_shared<Node_Fringe>(agent, lra_lock, new_action, leaf.q_value->depth + 1, new_feature);
    new_action->data = new_action_data;

    /// Add to the appropriate parent list
    leaf.fringe_values[new_action_data->q_value->feature.get()].values.push_back(new_action_data);

    return new_action_data;
  }

  Node_Split::Node_Split(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_)
    : Node(agent_, parent_action_, rete_action_, q_value_)
  {
    assert(q_value_->type == Q_Value::Type::SPLIT);
    ++agent.q_value_count;
  }

  Node_Split::~Node_Split() {
    --agent.q_value_count;
  }

  Rete::Rete_Node_Ptr Node_Split::cluster_root_ancestor() const {
    return rete_action.lock()->parent_left();
  }

  void Node_Split::decision() {
    if(!rete_action.lock()->is_excised())
      agent.respecialize(*rete_action.lock());
  }

  Node_Unsplit::Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_)
    : Node(agent_, parent_action_, rete_action_, new Q_Value(Q_Value::Token(), Q_Value::Type::UNSPLIT, depth_, feature_, agent_.get_total_step_count()))
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
    return rete_action.lock()->parent_left();
  }

  void Node_Unsplit::decision() {
    if(!rete_action.lock()->is_excised())
      agent.specialize(*rete_action.lock());
  }

  Node_Fringe::Node_Fringe(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_)
   : Node(agent_, parent_action_, rete_action_, new Q_Value(dynamic_cast<Node *>(parent_action_->data.get())->sum_value(), Q_Value::Type::FRINGE, depth_, feature_, agent_.get_total_step_count()))
  {
    q_value->update_totals();
  }

  Rete::Rete_Node_Ptr Node_Fringe::cluster_root_ancestor() const {
    return rete_action.lock()->parent_left()->parent_left();
  }

}
