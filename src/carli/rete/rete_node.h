#ifndef RETE_NODE_H
#define RETE_NODE_H

#include "../utility/linked_list.h"
#include "wme_token.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <list>
#include <unordered_set>
#include <deque>

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

    class Output;
    class Output : public Zeni::Pool_Allocator<Output> {
      Output(const Output &);
      Output & operator=(const Output &);

    public:
      typedef Zeni::Linked_List<Output> List;

      Rete_Node * const ptr;

      Output(Rete_Node * const &ptr_)
        : ptr(ptr_),
        list(this)
      {
      }
      
      operator Rete_Node * const & () const {return ptr;}
      
      Rete_Node & operator*() const {return *ptr;}
      Rete_Node * operator->() const {return ptr;}

      bool operator<(const Output &rhs) const {
        return ptr < rhs.ptr;
      }

      List list;
    };
    typedef Output::List::list_pointer_type Outputs;

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
      erase_output_enabled(output)->insert_before(outputs_disabled);
      unpass_tokens(output);
    }

    void enable_output(Rete_Node * const &output) {
      erase_output_disabled(output)->insert_before(outputs_enabled);
      pass_tokens(output);
    }

    void insert_output_enabled(const Rete_Node_Ptr &output) {
      outputs_all.push_back(output);
      (new Output(output.get()))->list.insert_before(outputs_enabled);
    }

    void insert_output_disabled(const Rete_Node_Ptr &output) {
      outputs_all.push_back(output);
      (new Output(output.get()))->list.insert_before(outputs_disabled);
    }

    void erase_output(const Rete_Node_Ptr &output) {
      Output::List::list_pointer_type found;
      if(output->disabled_input(shared()))
        found = erase_output_disabled(output.get());
      else
        found = erase_output_enabled(output.get());
      Output * const ptr = &**found;
      found.zero();
      delete ptr;

      outputs_all.erase(std::find(outputs_all.begin(), outputs_all.end(), output));
    }

    Output::List::list_pointer_type erase_output_enabled(Rete_Node * const &output) {
      const Output::List::list_pointer_type found = outputs_enabled->find(Output(output));
      assert(found);
      found->erase_from(outputs_enabled);
      return found;
    }

    Output::List::list_pointer_type erase_output_disabled(Rete_Node * const &output) {
      const Output::List::list_pointer_type found = outputs_disabled->find(Output(output));
      assert(found);
      found->erase_from(outputs_disabled);
      return found;
    }

    virtual void print_details(std::ostream &os) const = 0; ///< Formatted for dot: http://www.graphviz.org/content/dot-language

    virtual void output_name(std::ostream &os, const int64_t &depth) const = 0;

    virtual bool is_active() const = 0; ///< Has the node matched and forwarded at least one token?
    
    template <typename VISITOR>
    VISITOR visit_preorder(VISITOR visitor, const bool &strict) {
      return visit_preorder(visitor, strict, (intptr_t(this) & ~intptr_t(3)) | ((visitor_value & 3) != 1 ? 1 : 2));
    }
    
    template <typename VISITOR>
    VISITOR visit_preorder(VISITOR visitor, const bool &strict, const intptr_t &visitor_value_) {
      std::deque<Rete_Node *> nodes;
      nodes.push_back(this);

      while(!nodes.empty()) {
        Rete_Node * const node = nodes.front();
        nodes.pop_front();

        if(node->visitor_value == visitor_value_)
          continue;

        if(!strict && !dynamic_cast<Rete_Filter *>(node)) {
          if(node->parent_left()->visitor_value != visitor_value_) {
            nodes.push_front(node->parent_left().get());
            continue;
          }
          else if(node->parent_right()->visitor_value != visitor_value_) {
            nodes.push_front(node->parent_right().get());
            continue;
          }
        }

        node->visitor_value = visitor_value_;
        visitor(*node);
        for(auto &o : node->outputs_all)
          nodes.push_back(o.get());
      }

      return visitor;
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
    intptr_t visitor_value = 0;
  };

}

#endif
