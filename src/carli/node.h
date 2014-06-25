#ifndef CARLI_NODE_H
#define CARLI_NODE_H

#include "utility/tracked_ptr.h"

#include "action.h"
#include "feature.h"
#include "q_value.h"

#include <map>

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

  typedef std::shared_ptr<Node> Node_Ptr;
  typedef std::shared_ptr<Node_Split> Node_Split_Ptr;
  typedef std::shared_ptr<Node_Unsplit> Node_Unsplit_Ptr;
  typedef std::shared_ptr<Node_Fringe> Node_Fringe_Ptr;

  struct Fringe_Value_Data {
    std::list<Node_Fringe_Ptr> values;
    double value_delta_max = 0.0;
    int64_t value_delta_update_count = 0;
  };
  typedef std::map<Feature *, Fringe_Value_Data, compare_deref_memfun_lt<Feature, Feature, &Feature::compare_axis>> Fringe_Values;

  class CARLI_LINKAGE Node : public std::enable_shared_from_this<Node>, public Zeni::Pool_Allocator<Node_Unsplit>, public Rete::Rete_Data {
    Node(const Node &) = delete;
    Node & operator=(const Node &) = delete;

  public:
    Node(Agent &agent_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_)
     : agent(agent_),
     rete_action(rete_action_),
     q_value(q_value_)
    {
    }

    virtual ~Node();
    
    int64_t rank() const override;

    virtual void action(Agent &agent, const Rete::WME_Token &token) = 0;
    virtual void retraction(Agent &agent, const Rete::WME_Token &token);
    
    Node_Split_Ptr create_split(Agent &agent, const Rete::WME_Ptr_C &wme_blink, const bool &terminal);
    Node_Unsplit_Ptr create_unsplit(Agent &agent, const Rete::WME_Ptr_C &wme_blink, Fringe_Values &node_unsplit_fringe);
    Node_Fringe_Ptr create_fringe(Agent &agent, Node &leaf);

    Agent &agent;
    std::weak_ptr<Rete::Rete_Action> rete_action;
    tracked_ptr<Q_Value> q_value;
    bool delete_q_value = true;
  };
  
  class CARLI_LINKAGE Node_Split : public Node {
    Node_Split(const Node_Split &) = delete;
    Node_Split & operator=(const Node_Split &) = delete;

  public:
    Node_Split(Agent &agent_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_, const bool &terminal_);
    ~Node_Split();

    Node_Split * clone() const override {
      return new Node_Split(agent, rete_action.lock(), q_value->clone(), terminal);
    }
    
    Rete::Rete_Node_Ptr cluster_root_ancestor() const override;

    void action(Agent &agent, const Rete::WME_Token &token) override;

    bool terminal;
  };

  class CARLI_LINKAGE Node_Unsplit : public Node {
    Node_Unsplit(const Node_Unsplit &) = delete;
    Node_Unsplit & operator=(const Node_Unsplit &) = delete;

  public:
    Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_);
    Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_);
    ~Node_Unsplit();

    Node_Unsplit * clone() const override {
      return new Node_Unsplit(agent, rete_action.lock(), q_value->clone());
    }
    
    Rete::Rete_Node_Ptr cluster_root_ancestor() const override;

    void action(Agent &agent, const Rete::WME_Token &token) override;

    Fringe_Values fringe_values; ///< Not cloned
  };

  class CARLI_LINKAGE Node_Fringe : public Node {
    Node_Fringe(const Node_Fringe &) = delete;
    Node_Fringe & operator=(const Node_Fringe &) = delete;
    
  public:
    Node_Fringe(Agent &agent_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_)
     : Node(agent_, rete_action_, new Q_Value(0.0, Q_Value::Type::FRINGE, depth_, feature_))
    {
    }

    Node_Fringe * clone() const override {
      return new Node_Fringe(agent, rete_action.lock(), q_value->clone());
    }
    
    Rete::Rete_Node_Ptr cluster_root_ancestor() const override;

    void action(Agent &agent, const Rete::WME_Token &token) override;

  protected:
    Node_Fringe(Agent &agent_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_)
      : Node(agent_, rete_action_, q_value_)
    {
    }
  };

  inline void __node_size_check() {
    typedef Node::value_type pool_allocator_type;
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Split), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Unsplit), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Fringe), "Pool size suboptimal.");
  }

}

#endif
