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

  void Node::retraction(Agent &agent, const Rete::WME_Token &token) {
    agent.purge_q_value_next(get_action(token), q_value);
  }

  Node_Split::Node_Split(Agent &agent_, Rete::Rete_Action &rete_action_, const std::function<Action_Ptr_C (const Rete::WME_Token &)> &get_action_, const tracked_ptr<Q_Value> &q_value_)
   : Node(agent_, rete_action_, get_action_, q_value_)
  {
    ++agent.q_value_count;
  }

  Node_Split::~Node_Split() {
    --agent.q_value_count;
  }
  
  void Node_Split::action(Agent &agent, const Rete::WME_Token &token) {
    if(!agent.respecialize(rete_action, token))
      agent.insert_q_value_next(get_action(token), q_value);
  }

  Node_Unsplit::Node_Unsplit(Agent &agent_, Rete::Rete_Action &rete_action_, const std::function<Action_Ptr_C (const Rete::WME_Token &)> &get_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_)
   : Node(agent_, rete_action_, get_action_, new Q_Value(0.0, Q_Value::Type::UNSPLIT, depth_, feature_))
  {
    ++agent.q_value_count;
  }

  Node_Unsplit::~Node_Unsplit() {
    --agent.q_value_count;
  }
  
  void Node_Unsplit::action(Agent &agent, const Rete::WME_Token &token) {
    if(!agent.specialize(rete_action, token))
      agent.insert_q_value_next(get_action(token), q_value);
  }
  
  void Node_Fringe::action(Agent &agent, const Rete::WME_Token &token) {
    agent.insert_q_value_next(get_action(token), q_value);
  }
}
