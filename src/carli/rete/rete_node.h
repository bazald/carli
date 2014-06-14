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
  class Rete_Data;
  class Rete_Existential;
  class Rete_Existential_Join;
  class Rete_Filter;
  class Rete_Join;
  class Rete_Negation;
  class Rete_Negation_Join;
  class Rete_Node;
  class Rete_Predicate;

  typedef std::shared_ptr<const Rete_Action> Rete_Action_Ptr_C;
  typedef std::shared_ptr<const Rete_Data> Rete_Data_Ptr_C;
  typedef std::shared_ptr<const Rete_Existential> Rete_Existential_Ptr_C;
  typedef std::shared_ptr<const Rete_Existential_Join> Rete_Existential_Join_Ptr_C;
  typedef std::shared_ptr<const Rete_Filter> Rete_Filter_Ptr_C;
  typedef std::shared_ptr<const Rete_Join> Rete_Join_Ptr_C;
  typedef std::shared_ptr<const Rete_Negation> Rete_Negation_Ptr_C;
  typedef std::shared_ptr<const Rete_Negation_Join> Rete_Negation_Join_Ptr_C;
  typedef std::shared_ptr<const Rete_Node> Rete_Node_Ptr_C;
  typedef std::shared_ptr<const Rete_Predicate> Rete_Predicate_Ptr_C;

  typedef std::shared_ptr<Rete_Action> Rete_Action_Ptr;
  typedef std::shared_ptr<Rete_Data> Rete_Data_Ptr;
  typedef std::shared_ptr<Rete_Existential> Rete_Existential_Ptr;
  typedef std::shared_ptr<Rete_Existential_Join> Rete_Existential_Join_Ptr;
  typedef std::shared_ptr<Rete_Filter> Rete_Filter_Ptr;
  typedef std::shared_ptr<Rete_Join> Rete_Join_Ptr;
  typedef std::shared_ptr<Rete_Negation> Rete_Negation_Ptr;
  typedef std::shared_ptr<Rete_Negation_Join> Rete_Negation_Join_Ptr;
  typedef std::shared_ptr<Rete_Node> Rete_Node_Ptr;
  typedef std::shared_ptr<Rete_Predicate> Rete_Predicate_Ptr;

  class RETE_LINKAGE Rete_Data {
    Rete_Data(const Rete_Data &);
    Rete_Data & operator=(const Rete_Data &);

  public:
    Rete_Data() {}

    virtual ~Rete_Data() {}

    virtual Rete_Data * clone() const = 0;

    virtual int64_t rank() const = 0;
    virtual Rete_Node_Ptr cluster_root_ancestor() const = 0;
  };

  class RETE_LINKAGE Rete_Node : public std::enable_shared_from_this<Rete_Node>, public Zeni::Pool_Allocator<char [256]>
  {
    Rete_Node(const Rete_Node &);
    Rete_Node & operator=(const Rete_Node &);

    friend RETE_LINKAGE void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out);
    friend RETE_LINKAGE void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out);
    friend RETE_LINKAGE void bind_to_existential_join(const Rete_Existential_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    friend RETE_LINKAGE void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    friend RETE_LINKAGE void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);
    friend RETE_LINKAGE void bind_to_negation_join(const Rete_Negation_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    friend RETE_LINKAGE void bind_to_predicate(const Rete_Predicate_Ptr &predicate, const Rete_Node_Ptr &out);

  public:
    typedef std::list<Rete_Filter_Ptr, Zeni::Pool_Allocator<Rete_Filter_Ptr>> Filters;
    typedef std::list<Rete_Node_Ptr, Zeni::Pool_Allocator<Rete_Node_Ptr>> Output_Ptrs;
    typedef std::list<Rete_Node *, Zeni::Pool_Allocator<Rete_Node *>> Outputs;

    Rete_Node() {}
    virtual ~Rete_Node() {}

    virtual void destroy(Filters &filters, const Rete_Node_Ptr &output) = 0;

    std::shared_ptr<const Rete_Node> shared() const {
      return shared_from_this();
    }
    std::shared_ptr<Rete_Node> shared() {
      return shared_from_this();
    }

    const Output_Ptrs & get_outputs_all() const {
      return outputs_all;
    }
    
    virtual Rete_Node_Ptr_C parent_left() const = 0;
    virtual Rete_Node_Ptr_C parent_right() const = 0;
    virtual Rete_Node_Ptr parent_left() = 0;
    virtual Rete_Node_Ptr parent_right() = 0;

    virtual int64_t height() const = 0;

    virtual void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) = 0;
    virtual bool remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) = 0; ///< Returns true if removed the last

    virtual void disconnect(const Rete_Node * const &/*from*/) {}

    virtual void pass_tokens(Rete_Node * const &output) = 0;
    virtual void unpass_tokens(Rete_Node * const &output) = 0;

    virtual bool operator==(const Rete_Node &rhs) const = 0;
    
    virtual bool disabled_input(const Rete_Node_Ptr &) {return false;}

    void disable_output(Rete_Node * const &output) {
      erase_output_enabled(output);
      outputs_disabled.push_front(output);
      unpass_tokens(output);
    }

    void enable_output(Rete_Node * const &output) {
      erase_output_disabled(output);
      outputs_enabled.push_back(output);
      pass_tokens(output);
    }

    void insert_output(const Rete_Node_Ptr &output) {
      outputs_all.push_back(output);
      outputs_enabled.push_back(output.get());
    }

    void insert_output_disabled(const Rete_Node_Ptr &output) {
      outputs_all.push_front(output);
      outputs_disabled.push_front(output.get());
    }

    void erase_output(const Rete_Node_Ptr &output) {
      if(output->disabled_input(shared()))
        erase_output_disabled(output.get());
      else
        erase_output_enabled(output.get());
      outputs_all.erase(std::find(outputs_all.begin(), outputs_all.end(), output));
    }

    void erase_output_enabled(const Rete_Node * const &output) {
      const auto found = std::find(outputs_enabled.rbegin(), outputs_enabled.rend(), output);
      assert(found != outputs_enabled.rend());
      outputs_enabled.erase(--found.base());
    }

    void erase_output_disabled(const Rete_Node * const &output) {
      const auto found = std::find(outputs_disabled.begin(), outputs_disabled.end(), output);
      assert(found != outputs_disabled.end());
      outputs_disabled.erase(found);
    }

    virtual void print_details(std::ostream &os) const = 0; ///< Formatted for dot: http://www.graphviz.org/content/dot-language

    virtual void output_name(std::ostream &os, const int64_t &depth) const = 0;

    virtual bool is_active() const = 0; ///< Has the node matched and forwarded at least one token?
    
    template <typename VISITOR>
    VISITOR visit_preorder(VISITOR visitor, const bool &strict) {
      visitor_value = (intptr_t(this) & ~intptr_t(3)) | ((visitor_value & 3) != 1 ? 1 : 2);
      return visit_preorder_tail(visitor, strict);
    }
    
    template <typename VISITOR>
    VISITOR visit_preorder(VISITOR visitor, const bool &strict, const intptr_t &visitor_value_) {
      if(visitor_value == visitor_value_)
        return visitor;
      visitor_value = visitor_value_;
      return visit_preorder_tail(visitor, strict);
    }
    
    Rete_Data_Ptr data;

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

    Output_Ptrs outputs_all;
    Outputs outputs_enabled;
    Outputs outputs_disabled;

  private:
    template <typename VISITOR>
    VISITOR visit_preorder_tail(VISITOR visitor, const bool &strict) {
      if(!strict && !dynamic_cast<Rete_Filter *>(this)) {
        visitor = parent_left()->visit_preorder(visitor, strict, visitor_value);
        visitor = parent_right()->visit_preorder(visitor, strict, visitor_value);
      }
      visitor(*this);
      for(auto &o : outputs_all)
        visitor = o->visit_preorder(visitor, strict, visitor_value);
      return visitor;
    }

    intptr_t visitor_value = 0;
  };

}

#endif
