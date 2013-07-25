#ifndef RETE_NODE_H
#define RETE_NODE_H

#include "wme_vector.h"

#include <cassert>
#include <functional>

namespace Rete {

  class Rete_Action;
  class Rete_Existential;
  class Rete_Existential_Join;
  class Rete_Filter;
  class Rete_Join;
  class Rete_Negation;
  class Rete_Negation_Join;
  class Rete_Node;
  class Rete_Predicate;

  typedef std::shared_ptr<const Rete_Action> Rete_Action_Ptr_C;
  typedef std::shared_ptr<const Rete_Existential> Rete_Existential_Ptr_C;
  typedef std::shared_ptr<const Rete_Existential_Join> Rete_Existential_Join_Ptr_C;
  typedef std::shared_ptr<const Rete_Filter> Rete_Filter_Ptr_C;
  typedef std::shared_ptr<const Rete_Join> Rete_Join_Ptr_C;
  typedef std::shared_ptr<const Rete_Negation> Rete_Negation_Ptr_C;
  typedef std::shared_ptr<const Rete_Negation_Join> Rete_Negation_Join_Ptr_C;
  typedef std::shared_ptr<const Rete_Node> Rete_Node_Ptr_C;
  typedef std::shared_ptr<const Rete_Predicate> Rete_Predicate_Ptr_C;

  typedef std::shared_ptr<Rete_Action> Rete_Action_Ptr;
  typedef std::shared_ptr<Rete_Existential> Rete_Existential_Ptr;
  typedef std::shared_ptr<Rete_Existential_Join> Rete_Existential_Join_Ptr;
  typedef std::shared_ptr<Rete_Filter> Rete_Filter_Ptr;
  typedef std::shared_ptr<Rete_Join> Rete_Join_Ptr;
  typedef std::shared_ptr<Rete_Negation> Rete_Negation_Ptr;
  typedef std::shared_ptr<Rete_Negation_Join> Rete_Negation_Join_Ptr;
  typedef std::shared_ptr<Rete_Node> Rete_Node_Ptr;
  typedef std::shared_ptr<Rete_Predicate> Rete_Predicate_Ptr;

  class Rete_Node : public std::enable_shared_from_this<Rete_Node> {
    Rete_Node(const Rete_Node &);
    Rete_Node & operator=(const Rete_Node &);

    friend void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out);
    friend void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out);
    friend void bind_to_existential_join(const Rete_Existential_Join_Ptr &existential_join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    friend void bind_to_filter(const Rete_Filter_Ptr &filter, const Rete_Filter_Ptr &out);
    friend void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    friend void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);
    friend void bind_to_negation_join(const Rete_Negation_Join_Ptr &negation_join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    friend void bind_to_predicate(const Rete_Predicate_Ptr &predicate, const Rete_Node_Ptr &out);

  public:
    Rete_Node() {}
    virtual ~Rete_Node() {}
    
    virtual void destroy(std::unordered_set<Rete_Filter_Ptr> &filters, const Rete_Node_Ptr &output) = 0;
  
    std::shared_ptr<const Rete_Node> shared() const {
      return shared_from_this();
    }
    std::shared_ptr<Rete_Node> shared() {
      return shared_from_this();
    }
    
    const std::unordered_set<Rete_Node_Ptr> & get_outputs() const {
      return outputs;
    }

    virtual void insert_wme(const WME &wme, const Rete_Node_Ptr_C &from = nullptr) {
      auto wme_vector = std::make_shared<WME_Vector>();
      wme_vector->wmes.push_back(wme);
      insert_wme_vector(wme_vector, from);
    }

    virtual void remove_wme(const WME &wme, const Rete_Node_Ptr_C &from = nullptr) {
      auto wme_vector = std::make_shared<WME_Vector>();
      wme_vector->wmes.push_back(wme);
      remove_wme_vector(wme_vector, from);
    }

    virtual void insert_wme_vector(const WME_Vector_Ptr_C &wme_vector, const Rete_Node_Ptr_C &from) = 0;
    virtual void remove_wme_vector(const WME_Vector_Ptr_C &wme_vector, const Rete_Node_Ptr_C &from) = 0;

    virtual void pass_tokens(const Rete_Node_Ptr &output) = 0;

    virtual bool operator==(const Rete_Node &rhs) const = 0;

  protected:
    std::unordered_set<Rete_Node_Ptr> outputs;
  };

}

#endif
