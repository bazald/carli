#include "node.h"

#include "agent.h"

Node::~Node() {
  agent.purge_q_value(q_value);
  q_value.delete_and_zero();
}

void Node::destroy() {
  agent.excise_rule(action.lock());
}

Node_Split::Node_Split(Agent &agent_, const size_t &depth_)
 : Node(agent_, new Q_Value(0.0, Q_Value::Type::SPLIT, depth_))
{
  ++agent.q_value_count;
}

Node_Unsplit::Node_Unsplit(Agent &agent_, const size_t &depth_)
 : Node(agent_, new Q_Value(0.0, Q_Value::Type::UNSPLIT, depth_))
{
  ++agent.q_value_count;
}

Node_Fringe::~Node_Fringe() {
  feature.delete_and_zero();
}
