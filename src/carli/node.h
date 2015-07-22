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
  typedef std::map<Feature *, Fringe_Value_Data, Rete::compare_deref_memfun_lt<Feature, Feature, &Feature::compare_axis>> Fringe_Values;

  class CARLI_LINKAGE Node : public std::enable_shared_from_this<Node>, public Zeni::Pool_Allocator<Node_Unsplit>, public Rete::Rete_Data {
    Node(const Node &) = delete;
    Node & operator=(const Node &) = delete;

  public:
    Node(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_)
     : agent(agent_),
     parent_action(parent_action_),
     rete_action(rete_action_),
     variables(rete_action_->get_variables()),
     q_value(q_value_)
    {
      assert(q_value->depth == 1 || parent_action.lock());
    }

    virtual ~Node();

    int64_t rank() const override;

    Q_Value::Token sum_value(Q_Value::Token value_accumulator = Q_Value::Token()) const;

//    template <typename TYPE>
//    double min_int64(const ptrdiff_t offset, const int64_t value_accumulator = std::numeric_limits<int64_t>) const;

    void print_flags(std::ostream &os) const override;
    void print_action(std::ostream &os) const override;

    virtual void action(const Rete::WME_Token &token);
    virtual void decision() = 0;
    virtual void retraction(const Rete::WME_Token &token);

    Node_Split_Ptr create_split(const Rete::Rete_Action_Ptr &parent_action_);
    Node_Unsplit_Ptr create_unsplit(const Rete::Rete_Action_Ptr &parent_action_);
    Node_Fringe_Ptr create_fringe(Node_Unsplit &leaf, Feature * const &feature_);

    Agent &agent;
    std::weak_ptr<Rete::Rete_Action> parent_action;
    std::weak_ptr<Rete::Rete_Action> rete_action;
    Rete::Variable_Indices_Ptr_C variables;
    tracked_ptr<Q_Value> q_value;
    bool delete_q_value = true;
  };

  class CARLI_LINKAGE Node_Split : public Node {
    Node_Split(const Node_Split &) = delete;
    Node_Split & operator=(const Node_Split &) = delete;

  public:
    Node_Split(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_);
    ~Node_Split();

    Node_Split * clone() const override {
      return new Node_Split(agent, parent_action.lock(), rete_action.lock(), q_value->clone());
    }

    Rete::Rete_Node_Ptr cluster_root_ancestor() const override;

    void decision() override;
  };
  class CARLI_LINKAGE Node_Unsplit : public Node {

    Node_Unsplit(const Node_Unsplit &) = delete;
    Node_Unsplit & operator=(const Node_Unsplit &) = delete;

  public:
    Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_);
    Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_);
    ~Node_Unsplit();

    Node_Unsplit * clone() const override {
      return new Node_Unsplit(agent, parent_action.lock(), rete_action.lock(), q_value->clone());
    }

    Rete::Rete_Node_Ptr cluster_root_ancestor() const override;

    void decision() override;

    Fringe_Values fringe_values; ///< Not cloned
  };

  class CARLI_LINKAGE Node_Fringe : public Node {
    Node_Fringe(const Node_Fringe &) = delete;
    Node_Fringe & operator=(const Node_Fringe &) = delete;

  public:
    Node_Fringe(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_);

    Node_Fringe(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_)
      : Node(agent_, parent_action_, rete_action_, q_value_)
    {
      assert(q_value_->type == Q_Value::Type::FRINGE);
    }

    Node_Fringe * clone() const override {
      return new Node_Fringe(agent, parent_action.lock(), rete_action.lock(), q_value->clone());
    }

    Rete::Rete_Node_Ptr cluster_root_ancestor() const override;

    void decision() override {}
  };

  inline void __node_size_check() {
    typedef Node::value_type pool_allocator_type;
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Split), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Unsplit), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Node_Fringe), "Pool size suboptimal.");
  }

}

#endif
