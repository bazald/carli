#ifndef RETE_PREDICATE_H
#define RETE_PREDICATE_H

#include "rete_node.h"

namespace Rete {

  class Rete_Predicate : public Rete_Node {
    Rete_Predicate(const Rete_Predicate &);
    Rete_Predicate & operator=(const Rete_Predicate &);

    friend void bind_to_predicate(const Rete_Predicate_Ptr &predicate, const Rete_Node_Ptr &out);

  public:
    enum Predicate {EQ, NEQ, GT, GTE, LT, LTE};

    Rete_Predicate(const Predicate &predicate_, const WME_Token_Index lhs_index_, const WME_Token_Index rhs_index_)
     : m_predicate(predicate_),
     m_lhs_index(lhs_index_),
     m_rhs_index(rhs_index_)
    {
    }

    Rete_Predicate(const Predicate &predicate_, const WME_Token_Index lhs_index_, const Symbol_Ptr_C &rhs_)
     : m_predicate(predicate_),
     m_lhs_index(lhs_index_),
     m_rhs(rhs_)
    {
    }

    void destroy(std::list<Rete_Filter_Ptr> &filters, const Rete_Node_Ptr &output) {
      erase_output(output);
      if(outputs.empty())
        input.lock()->destroy(filters, shared());
    }

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &
#ifndef NDEBUG
                                                                                   from
#endif
                                                                                       ) {
      assert(from == input.lock());

      if(m_rhs) {
        if(!test_predicate((*wme_token)[m_lhs_index], m_rhs))
          return;
      }
      else {
        if(!test_predicate((*wme_token)[m_lhs_index], (*wme_token)[m_rhs_index]))
          return;
      }

      tokens.push_back(wme_token);

      for(auto &output : outputs)
        output->insert_wme_token(wme_token, shared());
    }

    void remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &
#ifndef NDEBUG
                                                                                   from
#endif
                                                                                       ) {
      assert(from == input.lock());

      auto found = find(tokens, wme_token);
      if(found != tokens.end()) {
        tokens.erase(found);
        for(auto &output : outputs)
          output->remove_wme_token(wme_token, shared());
      }
    }

    void pass_tokens(const Rete_Node_Ptr &output) {
      for(auto &wme_token : tokens)
        output->insert_wme_token(wme_token, shared());
    }

    bool operator==(const Rete_Node &rhs) const {
      if(auto predicate = dynamic_cast<const Rete_Predicate *>(&rhs)) {
        return m_predicate == predicate->m_predicate &&
               m_lhs_index == predicate->m_rhs_index &&
               m_rhs_index == predicate->m_rhs_index &&
               *m_rhs == *predicate->m_rhs &&
               input.lock() == predicate->input.lock();
      }
      return false;
    }

    static Rete_Predicate_Ptr find_existing(const Predicate &predicate, const WME_Token_Index &lhs_index, const WME_Token_Index &rhs_index, const Rete_Node_Ptr &out) {
      for(auto &o : out->get_outputs()) {
        if(auto existing_predicate = std::dynamic_pointer_cast<Rete_Predicate>(o)) {
          if(predicate == existing_predicate->m_predicate &&
             lhs_index == existing_predicate->m_lhs_index &&
             rhs_index == existing_predicate->m_rhs_index)
          {
            return existing_predicate;
          }
        }
      }

      return nullptr;
    }

    static Rete_Predicate_Ptr find_existing(const Predicate &predicate, const WME_Token_Index &lhs_index, const Symbol_Ptr_C &rhs, const Rete_Node_Ptr &out) {
      for(auto &o : out->get_outputs()) {
        if(auto existing_predicate = std::dynamic_pointer_cast<Rete_Predicate>(o)) {
          if(predicate == existing_predicate->m_predicate &&
             lhs_index == existing_predicate->m_lhs_index &&
             *rhs == *existing_predicate->m_rhs)
          {
            return existing_predicate;
          }
        }
      }

      return nullptr;
    }

  private:
    bool test_predicate(const Symbol_Ptr_C &lhs, const Symbol_Ptr_C &rhs) const {
      switch(m_predicate) {
        case EQ: return *lhs == *rhs;
        case NEQ: return *lhs != *rhs;
        case GT: return *lhs > *rhs;
        case GTE: return *lhs >= *rhs;
        case LT: return *lhs < *rhs;
        case LTE: return *lhs <= *rhs;
        default: abort();
      }
    }

    Predicate m_predicate;
    WME_Token_Index m_lhs_index;
    WME_Token_Index m_rhs_index;
    Symbol_Ptr_C m_rhs;
    std::weak_ptr<Rete_Node> input;
    std::list<WME_Token_Ptr_C> tokens;
  };

  inline void bind_to_predicate(const Rete_Predicate_Ptr &predicate, const Rete_Node_Ptr &out) {
    assert(predicate);
    predicate->input = out;

    out->outputs.push_back(predicate);
    out->pass_tokens(predicate);
  }

}

#endif