#ifndef CARLI_NODE_H
#define CARLI_NODE_H

#include "utility/tracked_ptr.h"

#include "feature.h"
#include "q_value.h"

namespace Carli {

  class Agent;

  class Node;
  class Node_Split;
  class Node_Unsplit;
  class Node_Fringe;
  class Node_Fringe_Ranged;

  typedef std::shared_ptr<const Node> Node_Ptr_C;
  typedef std::shared_ptr<const Node_Split> Node_Split_Ptr_C;
  typedef std::shared_ptr<const Node_Unsplit> Node_Unsplit_Ptr_C;
  typedef std::shared_ptr<const Node_Fringe> Node_Fringe_Ptr_C;
  typedef std::shared_ptr<const Node_Fringe_Ranged> Node_Fringe_Ranged_Ptr_C;

  typedef std::shared_ptr<Node> Node_Ptr;
  typedef std::shared_ptr<Node_Split> Node_Split_Ptr;
  typedef std::shared_ptr<Node_Unsplit> Node_Unsplit_Ptr;
  typedef std::shared_ptr<Node_Fringe> Node_Fringe_Ptr;
  typedef std::shared_ptr<Node_Fringe_Ranged> Node_Fringe_Ranged_Ptr;

  class CARLI_LINKAGE Node : public std::enable_shared_from_this<Node>, public Zeni::Pool_Allocator<Node_Fringe_Ranged> {
    Node(const Node &) = delete;
    Node & operator=(const Node &) = delete;

  public:
    Node(Agent &agent_, const tracked_ptr<Q_Value> &q_value_)
     : agent(agent_),
     q_value(q_value_)
    {
    }

    virtual Node * clone() const = 0;

    virtual ~Node();

    void destroy();
    
    virtual void action(Agent &agent, const Rete::WME_Token &token) = 0;
    virtual void retraction(Agent &agent, const Rete::WME_Token &token);

    Agent &agent;
    tracked_ptr<Q_Value> q_value;
    bool delete_q_value = true;
    Rete::Rete_Action * rete_action = nullptr;
  };

  class CARLI_LINKAGE Node_Split : public Node {
    Node_Split(const Node_Split &) = delete;
    Node_Split & operator=(const Node_Split &) = delete;

  public:
    Node_Split(Agent &agent_, const tracked_ptr<Q_Value> &q_value_);
    ~Node_Split();

    Node_Split * clone() const override {
      return new Node_Split(agent, q_value->clone());
    }

    void action(Agent &agent, const Rete::WME_Token &token) override;
  };

  class CARLI_LINKAGE Node_Unsplit : public Node {
    Node_Unsplit(const Node_Unsplit &) = delete;
    Node_Unsplit & operator=(const Node_Unsplit &) = delete;

  public:
    typedef std::list<std::shared_ptr<Node_Fringe>> Fringe_Values;

    Node_Unsplit(Agent &agent_, const int64_t &depth_, const tracked_ptr<Feature> &feature_);
    ~Node_Unsplit();

    Node_Unsplit * clone() const override {
      return new Node_Unsplit(agent, q_value->clone());
    }

    void action(Agent &agent, const Rete::WME_Token &token) override;

    Fringe_Values fringe_values; ///< Not cloned

  protected:
    Node_Unsplit(Agent &agent_, const tracked_ptr<Q_Value> &q_value_)
      : Node(agent_, q_value_)
    {
    }
  };

  class CARLI_LINKAGE Node_Fringe : public Node {
    Node_Fringe(const Node_Fringe &) = delete;
    Node_Fringe & operator=(const Node_Fringe &) = delete;
    
  public:
    Node_Fringe(Agent &agent_, const int64_t &depth_, const tracked_ptr<Feature> &feature_)
     : Node(agent_, new Q_Value(0.0, Q_Value::Type::FRINGE, depth_, feature_))
    {
    }

    Node_Fringe * clone() const override {
      return new Node_Fringe(agent, q_value->clone());
    }

    void action(Agent &agent, const Rete::WME_Token &token) override;

  protected:
    Node_Fringe(Agent &agent_, const tracked_ptr<Q_Value> &q_value_)
      : Node(agent_, q_value_)
    {
    }
  };

  class CARLI_LINKAGE Node_Ranged {
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

  class CARLI_LINKAGE Node_Fringe_Ranged : public Node_Fringe, public Node_Ranged {
    Node_Fringe_Ranged(const Node_Fringe_Ranged &) = delete;
    Node_Fringe_Ranged & operator=(const Node_Fringe_Ranged &) = delete;

  public:
    Node_Fringe_Ranged(Agent &agent_, const int64_t &depth_, const tracked_ptr<Feature> &feature_, const Range &range_, const Lines &lines_)
     : Node_Fringe(agent_, depth_, feature_),
     Node_Ranged(range_, lines_)
    {
    }

    Node_Fringe * clone() const override {
      return new Node_Fringe_Ranged(agent, q_value->clone(), range, lines);
    }

  protected:
    Node_Fringe_Ranged(Agent &agent_, const tracked_ptr<Q_Value> &q_value_, const Range &range_, const Lines &lines_)
      : Node_Fringe(agent_, q_value_),
     Node_Ranged(range_, lines_)
    {
    }
  };

  inline void __node_size_check() {
    typedef Node::value_type pool_allocator_type;
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Split), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Unsplit), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Fringe), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Fringe_Ranged), "Pool size suboptimal.");
  }

}

#endif
