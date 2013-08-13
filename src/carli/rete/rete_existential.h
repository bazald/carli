#ifndef RETE_EXISTENTIAL_H
#define RETE_EXISTENTIAL_H

#include "rete_node.h"

namespace Rete {

  class Rete_Existential : public Rete_Node {
    Rete_Existential(const Rete_Existential &);
    Rete_Existential & operator=(const Rete_Existential &);

    friend void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out);

  public:
    Rete_Existential() : output_token(std::make_shared<WME_Token>()) {}

    void destroy(Filters &filters, const Rete_Node_Ptr &output) {
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
          output->insert_wme_token(output_token, shared());
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
            output->remove_wme_token(output_token, shared());
        }
      }
    }

    void pass_tokens(const Rete_Node_Ptr &output) {
      if(!input_tokens.empty())
        output->insert_wme_token(output_token, shared());
    }

    bool operator==(const Rete_Node &rhs) const {
      if(auto existential = dynamic_cast<const Rete_Existential *>(&rhs))
        return input.lock() == existential->input.lock();
      return false;
    }

    static Rete_Existential_Ptr find_existing(const Rete_Node_Ptr &out) {
      for(auto &o : out->get_outputs()) {
        if(auto existing_existential = std::dynamic_pointer_cast<Rete_Existential>(o))
          return existing_existential;
      }

      return nullptr;
    }

  private:
    std::weak_ptr<Rete_Node> input;
    std::list<WME_Token_Ptr_C> input_tokens;
    WME_Token_Ptr_C output_token;
  };

  inline void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out) {
    assert(existential);
    existential->input = out;

    out->insert_output(existential);
    out->pass_tokens(existential);
  }

}

#endif
