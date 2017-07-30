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
  typedef std::weak_ptr<const Node> Node_Ptr_W;
  typedef std::shared_ptr<const Node_Split> Node_Split_Ptr_C;
  typedef std::shared_ptr<const Node_Unsplit> Node_Unsplit_Ptr_C;
  typedef std::shared_ptr<const Node_Fringe> Node_Fringe_Ptr_C;

  typedef std::shared_ptr<Node> Node_Ptr;
  typedef std::shared_ptr<Node_Split> Node_Split_Ptr;
  typedef std::shared_ptr<Node_Unsplit> Node_Unsplit_Ptr;
  typedef std::shared_ptr<Node_Fringe> Node_Fringe_Ptr;
  typedef std::weak_ptr<Node_Fringe> Node_Fringe_Ptr_W;

  typedef std::map<Feature *, std::list<Node_Fringe_Ptr_W>, Rete::compare_deref_memfun_lt<Feature, Feature, &Feature::compare_axis>> Fringe_Values;

  class Fringe_Axis_Selections : public std::map<tracked_ptr<Feature>, int64_t, Rete::compare_deref_memfun_lt<Feature, Feature, &Feature::compare_axis>> {
  public:
    ~Fringe_Axis_Selections();
  };

#ifndef NDEBUG
  class CARLI_LINKAGE Node_Tracker {
    Node_Tracker() {}
    Node_Tracker(const Node_Tracker &) {}

  public:
    static Node_Tracker & get();

    size_t num_nodes() const;

    void create(const Rete::Rete_Action &action);

    void destroy(const Node &node);

    void validate(Rete::Rete_Agent &agent, const Node * const ignore);

  private:
    std::unordered_map<const Node *, std::string> m_names_from_nodes;
    std::unordered_map<std::string, const Node *> m_nodes_from_names;
    size_t m_good_validations = 0;
  };
#endif

  class CARLI_LINKAGE Node : public std::enable_shared_from_this<Node>, public Zeni::Pool_Allocator<Node_Split>, public Rete::Rete_Data {
    Node(const Node &) = delete;
    Node & operator=(const Node &) = delete;

  public:
#ifndef NDEBUG
    friend Node_Tracker;
#endif

    Node(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_weight_, const tracked_ptr<Q_Value> &q_value_fringe_)
     : agent(agent_),
     parent_action(parent_action_),
     rete_action(rete_action_),
     variables(rete_action_->get_variables()),
     q_value_weight(q_value_weight_),
     q_value_fringe(q_value_fringe_)
    {
      assert((q_value_weight && q_value_weight->depth == 1) || (q_value_fringe && q_value_fringe->depth == 1) || parent_action.lock());
    }

    virtual ~Node();

    int64_t rank() const override;

    Q_Value::Token sum_value(Q_Value::Token value_accumulator = Q_Value::Token()) const;
    std::pair<double, double> value_range(const bool &include_ancestors = true) const;

//    template <typename TYPE>
//    double min_int64(const ptrdiff_t offset, const int64_t value_accumulator = std::numeric_limits<int64_t>) const;

    void print_flags(std::ostream &os) const override;
    void print_action(std::ostream &os) const override;
    Rete::Rete_Node_Ptr_C get_suppress() const override;

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
    tracked_ptr<Q_Value> q_value_weight;
    bool delete_q_value_weight = true;
    tracked_ptr<Q_Value> q_value_fringe;
    bool delete_q_value_fringe = true;
  };

  class CARLI_LINKAGE Node_Split : public Node {
    Node_Split(const Node_Split &) = delete;
    Node_Split & operator=(const Node_Split &) = delete;

  public:
    friend class Node_Tracker;

    Node_Split(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_weight_, const tracked_ptr<Q_Value> &q_value_fringe_);
    ~Node_Split();

    Node_Split * clone() const override {
      return new Node_Split(agent, parent_action.lock(), rete_action.lock(), q_value_weight ? q_value_weight->clone() : nullptr, q_value_fringe ? q_value_fringe->clone() : nullptr);
    }

    Rete::Rete_Node_Ptr cluster_root_ancestor() const override;

    void decision() override;

    std::list<Node_Ptr_W> children; ///< Not cloned
    Fringe_Axis_Selections fringe_axis_selections;
    int64_t fringe_axis_counter = 0;
    bool blacklist_full = false;

    Fringe_Values fringe_values; ///< Not cloned
  };

  class CARLI_LINKAGE Node_Unsplit : public Node {

    Node_Unsplit(const Node_Unsplit &) = delete;
    Node_Unsplit & operator=(const Node_Unsplit &) = delete;

  public:
    friend class Node_Tracker;

    Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_);
    Node_Unsplit(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_weight_, const tracked_ptr<Q_Value> &q_value_fringe_);
    ~Node_Unsplit();

    Node_Unsplit * clone() const override {
      return new Node_Unsplit(agent, parent_action.lock(), rete_action.lock(), q_value_weight ? q_value_weight->clone() : nullptr, q_value_fringe ? q_value_fringe->clone() : nullptr);
    }

    Rete::Rete_Node_Ptr cluster_root_ancestor() const override;

    void decision() override;

    Fringe_Values fringe_values; ///< Not cloned
    Fringe_Axis_Selections fringe_axis_selections;
    int64_t fringe_axis_counter = 0;
  };

  class CARLI_LINKAGE Node_Fringe : public Node {
    Node_Fringe(const Node_Fringe &) = delete;
    Node_Fringe & operator=(const Node_Fringe &) = delete;

  public:
    Node_Fringe(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const int64_t &depth_, const tracked_ptr<Feature> &feature_);

    Node_Fringe(Agent &agent_, const Rete::Rete_Action_Ptr &parent_action_, const Rete::Rete_Action_Ptr &rete_action_, const tracked_ptr<Q_Value> &q_value_weight_, const tracked_ptr<Q_Value> &q_value_fringe_)
      : Node(agent_, parent_action_, rete_action_, q_value_weight_, q_value_fringe_)
    {
      assert((!q_value_weight_ || q_value_weight_->type == Q_Value::Type::FRINGE) && (!q_value_fringe_ || q_value_fringe_->type == Q_Value::Type::FRINGE));
    }

    Node_Fringe * clone() const override {
      return new Node_Fringe(agent, parent_action.lock(), rete_action.lock(), q_value_weight ? q_value_weight->clone() : nullptr, q_value_fringe ? q_value_fringe->clone() : nullptr);
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
