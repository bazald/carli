#include "node.h"

#include "agent.h"

namespace Carli {

  Node::~Node() {
    if(delete_q_value) {
      agent.purge_q_value(q_value);
      q_value.delete_and_zero();
    }
  }

  void Node::destroy() {
    agent.excise_rule(debuggable_pointer_cast<Rete::Rete_Action>(action->shared()));
  }

  Node_Split::Node_Split(Agent &agent_, const tracked_ptr<Q_Value> &q_value_)
   : Node(agent_, q_value_)
  {
    ++agent.q_value_count;
  }

  Node_Split::~Node_Split() {
    --agent.q_value_count;
  }

  Node_Unsplit::Node_Unsplit(Agent &agent_, const size_t &depth_)
   : Node(agent_, new Q_Value(0.0, Q_Value::Type::UNSPLIT, depth_))
  {
    ++agent.q_value_count;
  }

  Node_Unsplit::~Node_Unsplit() {
    --agent.q_value_count;
  }

  Node_Fringe::~Node_Fringe() {
    feature.delete_and_zero();
  }

}
