#ifndef RETE_NEGATION_H
#define RETE_NEGATION_H

#include "rete_node.h"

namespace Rete {

  class Rete_Negation : public Rete_Node {
    Rete_Negation(const Rete_Negation &);
    Rete_Negation & operator=(const Rete_Negation &);

    friend void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);

  public:
    Rete_Negation() : output_token(std::make_shared<WME_Token>()) {}

    void destroy(std::unordered_set<Rete_Filter_Ptr> &filters, const Rete_Node_Ptr &output) {
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

      input_tokens.push_back(wme_token);

      if(input_tokens.size() == 1) {
        for(auto &output : outputs)
          output->remove_wme_token(output_token, shared());
      }
    }

    void remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &
#ifndef NDEBUG
                                                                                   from
#endif
                                                                                       ) {
      assert(from == input.lock());

      auto found = find(input_tokens, wme_token);
      if(found != input_tokens.end()) {
        input_tokens.erase(found);
        if(input_tokens.empty()) {
          for(auto &output : outputs)
            output->insert_wme_token(output_token, shared());
        }
      }
    }

    void pass_tokens(const Rete_Node_Ptr &output) {
      if(input_tokens.empty()) {
        output->insert_wme_token(output_token, shared());
      }
    }

    bool operator==(const Rete_Node &rhs) const {
      if(auto negation = dynamic_cast<const Rete_Negation *>(&rhs))
        return input.lock() == negation->input.lock();
      return false;
    }

    static Rete_Negation_Ptr find_existing(const Rete_Node_Ptr &out) {
      for(auto &o : out->get_outputs()) {
        if(auto existing_negation = std::dynamic_pointer_cast<Rete_Negation>(o))
          return existing_negation;
      }

      return nullptr;
    }

  private:
    std::weak_ptr<Rete_Node> input;
    std::list<WME_Token_Ptr_C> input_tokens;
    WME_Token_Ptr_C output_token;
  };

  inline void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out) {
    assert(negation);
    negation->input = out;

    out->outputs.push_back(negation);
    out->pass_tokens(negation);
  }

}

#endif
