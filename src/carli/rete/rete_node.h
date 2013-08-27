#ifndef RETE_NODE_H
#define RETE_NODE_H

#include "wme_token.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <list>
#include <unordered_set>

namespace Rete {

  class Agenda;
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

  class Rete_Node : public std::enable_shared_from_this<Rete_Node>, public Zeni::Pool_Allocator<char [256]>
  {
    Rete_Node(const Rete_Node &);
    Rete_Node & operator=(const Rete_Node &);

    friend void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out);
    friend void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out);
    friend void bind_to_existential_join(const Rete_Existential_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    friend void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    friend void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);
    friend void bind_to_negation_join(const Rete_Negation_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    friend void bind_to_predicate(const Rete_Predicate_Ptr &predicate, const Rete_Node_Ptr &out);

  public:
    typedef std::list<Rete_Filter_Ptr, Zeni::Pool_Allocator<Rete_Filter_Ptr>> Filters;
    typedef std::list<Rete_Node_Ptr, Zeni::Pool_Allocator<Rete_Node_Ptr>> Outputs;

    Rete_Node() {}
    virtual ~Rete_Node() {}

    virtual void destroy(Filters &filters, const Rete_Node_Ptr &output) = 0;

    std::shared_ptr<const Rete_Node> shared() const {
      return shared_from_this();
    }
    std::shared_ptr<Rete_Node> shared() {
      return shared_from_this();
    }

    const Outputs & get_outputs() const {
      return outputs;
    }

    virtual Rete_Node_Ptr_C parent() const = 0;
    virtual Rete_Node_Ptr parent() = 0;

    virtual void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) = 0;
    virtual bool remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) = 0; ///< Returns true if removed the last

    virtual void disconnect(const Rete_Node * const &/*from*/) {}

    virtual void pass_tokens(const Rete_Node_Ptr &output) = 0;
    virtual void unpass_tokens(const Rete_Node_Ptr &output) = 0;

    virtual bool operator==(const Rete_Node &rhs) const = 0;

    void insert_output(const Rete_Node_Ptr &output) {
//      outputs.insert(output);
      outputs.push_back(output);
    }

    void erase_output(const Rete_Node_Ptr &output) {
//      outputs.erase(output);
      outputs.erase(std::find(outputs.begin(), outputs.end(), output));
    }

    void disable_output(const Rete_Node_Ptr &output) {
      ++outputs_disabled;
      unpass_tokens(output);
      erase_output(output);
    }

    void enable_output(const Rete_Node_Ptr &output) {
      insert_output(output);
      pass_tokens(output);
      --outputs_disabled;
    }

  protected:
    template<typename CONTAINER, typename KEY>
    typename CONTAINER::iterator find(CONTAINER &tokens, const KEY &token) {
//      return tokens.find(token);
      return std::find(tokens.begin(), tokens.end(), token);
    }

    template<typename CONTAINER, typename KEY>
    typename CONTAINER::iterator find_deref(CONTAINER &tokens, const KEY &token) {
//      return tokens.find(token);
      return std::find_if(tokens.begin(), tokens.end(), [&token](const WME_Token_Ptr_C &tok){return *tok == *token;});
    }

    template<typename CONTAINER, typename KEY>
    typename CONTAINER::iterator find_key(CONTAINER &tokens, const KEY &token) {
//      return tokens.find(token);
      return std::find_if(tokens.begin(), tokens.end(), [&token](const typename CONTAINER::value_type &tok){return tok.first == token;});
    }

    Outputs outputs;
    size_t outputs_disabled = 0u;
  };

}

#endif
