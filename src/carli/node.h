#ifndef NODE_H
#define NODE_H

#include "utility/tracked_ptr.h"

#include "feature.h"
#include "q_value.h"

class Agent;
class Node_Fringe;
class Node_Fringe_Ranged;

class Node : public std::enable_shared_from_this<Node>, public Zeni::Pool_Allocator<Node_Fringe_Ranged> {
  Node(const Node &) = delete;
  Node & operator=(const Node &) = delete;

public:
  Node(Agent &agent_, const tracked_ptr<Q_Value> &q_value_)
   : agent(agent_),
   q_value(q_value_)
  {
  }

  virtual ~Node();

  void destroy();

  Agent &agent;
  tracked_ptr<Q_Value> q_value;
  std::weak_ptr<Rete::Rete_Action> action;
};

class Node_Split : public Node {
  Node_Split(const Node_Split &) = delete;
  Node_Split & operator=(const Node_Split &) = delete;

public:
  Node_Split(Agent &agent_, const tracked_ptr<Q_Value> &q_value_);
};

class Node_Unsplit : public Node {
  Node_Unsplit(const Node_Unsplit &) = delete;
  Node_Unsplit & operator=(const Node_Unsplit &) = delete;

public:
  typedef std::list<std::shared_ptr<Node_Fringe>> Fringe_Values;

  Node_Unsplit(Agent &agent_, const size_t &depth_);

  Fringe_Values fringe_values;
};

class Node_Fringe : public Node {
  Node_Fringe(const Node_Fringe &) = delete;
  Node_Fringe & operator=(const Node_Fringe &) = delete;

public:
  Node_Fringe(Agent &agent_, const size_t &depth_)
   : Node(agent_, new Q_Value(0.0, Q_Value::Type::FRINGE, depth_))
  {
  }

  ~Node_Fringe();

  tracked_ptr<Feature> feature;
};

class Node_Ranged {
  Node_Ranged(const Node_Ranged &) = delete;
  Node_Ranged & operator=(const Node_Ranged &) = delete;

public:
  typedef std::pair<std::pair<double, double>, std::pair<double, double>> Range;
  typedef std::pair<std::pair<double, double>, std::pair<double, double>> Line;
  typedef std::vector<Line, Zeni::Pool_Allocator<Line>> Lines;

  Node_Ranged(const Range &range_, const Lines &lines_)
   : range(range_),
   lines(lines_)
  {
  }

  Range range;
  Lines lines;
};

class Node_Fringe_Ranged : public Node_Fringe, public Node_Ranged {
  Node_Fringe_Ranged(const Node_Fringe_Ranged &) = delete;
  Node_Fringe_Ranged & operator=(const Node_Fringe_Ranged &) = delete;

public:
  Node_Fringe_Ranged(Agent &agent_, const size_t &depth_, const Range &range_, const Lines &lines_)
   : Node_Fringe(agent_, depth_),
   Node_Ranged(range_, lines_)
  {
  }
};

inline void __node_size_check() {
  typedef typename Node::value_type pool_allocator_type;
  static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Split), "Pool size suboptimal.");
  static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Unsplit), "Pool size suboptimal.");
  static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Fringe), "Pool size suboptimal.");
  static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Fringe_Ranged), "Pool size suboptimal.");
}

#endif
