#ifndef RETE_JOIN_H
#define RETE_JOIN_H

#include "rete_node.h"

namespace Rete {

  class Rete_Join : public Rete_Node {
    Rete_Join(const Rete_Join &);
    Rete_Join & operator=(const Rete_Join &);

    friend void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

  public:
    Rete_Join(WME_Bindings bindings_) : bindings(bindings_) {}

    void destroy(std::unordered_set<Rete_Filter_Ptr> &filters, const Rete_Node_Ptr &output) {
      outputs.erase(output);
      if(outputs.empty()) {
        auto i0 = input0.lock();
        auto i1 = input1.lock();
        i0->destroy(filters, shared());
        if(i0 != i1)
          i1->destroy(filters, shared());
      }
    }

    void insert_wme_vector(const WME_Vector_Ptr_C &wme_vector, const Rete_Node_Ptr_C &from) {
      assert(from == input0.lock() || from == input1.lock());

      if(from == input0.lock()) {
        input0_tokens.insert(wme_vector);

        for(const auto &other : input1_tokens)
          join_tokens(wme_vector, other);
      }
      if(from == input1.lock()) {
        input1_tokens.insert(wme_vector);

        for(const auto &other : input0_tokens)
          join_tokens(other, wme_vector);
      }
    }

    void remove_wme_vector(const WME_Vector_Ptr_C &wme_vector, const Rete_Node_Ptr_C &from) {
      assert(from == input0.lock() || from == input1.lock());

      if(from == input0.lock()) {
        auto found = input0_tokens.find(wme_vector);
        if(found != input0_tokens.end()) {
          // TODO: Avoid looping through non-existent pairs?
          input0_tokens.erase(found);
          for(const auto &other : input1_tokens) {
            auto wme_vector_merge = join_wme_vectors(wme_vector, other);
            output_tokens.erase(wme_vector_merge);
            for(auto &output : outputs)
              output->remove_wme_vector(wme_vector_merge, shared());
          }
        }
      }
      if(from == input1.lock()) {
        auto found = input1_tokens.find(wme_vector);
        if(found != input1_tokens.end()) {
          // TODO: Avoid looping through non-existent pairs?
          input1_tokens.erase(found);
          for(const auto &other : input0_tokens) {
            auto wme_vector_merge = join_wme_vectors(other, wme_vector);
            output_tokens.erase(wme_vector_merge);
            for(auto &output : outputs)
              output->remove_wme_vector(wme_vector_merge, shared());
          }
        }
      }
    }

    bool operator==(const Rete_Node &rhs) const {
      if(auto join = dynamic_cast<const Rete_Join *>(&rhs))
        return bindings == join->bindings && input0.lock() == join->input0.lock() && input1.lock() == join->input1.lock();
      return false;
    }

    static Rete_Join_Ptr find_existing(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
      for(auto &o0 : out0->get_outputs()) {
        if(auto existing_join = std::dynamic_pointer_cast<Rete_Join>(o0)) {
          if(out1->get_outputs().find(o0) != out1->get_outputs().end()) {
            if(bindings == existing_join->bindings)
              return existing_join;
          }
        }
      }

      return nullptr;
    }

  private:
    void join_tokens(const WME_Vector_Ptr_C &lhs, const WME_Vector_Ptr_C &rhs) {
      for(auto &binding : bindings) {
        if(*lhs->wmes[binding.first.first].symbols[binding.first.second] != *rhs->wmes[binding.second.first].symbols[binding.second.second])
          return;
      }

      const WME_Vector_Ptr_C wme_vector_merge = join_wme_vectors(lhs, rhs);
      output_tokens.insert(wme_vector_merge);
      for(auto &output : outputs)
        output->insert_wme_vector(wme_vector_merge, shared());
    }

    WME_Vector_Ptr_C join_wme_vectors(const WME_Vector_Ptr_C lhs, const WME_Vector_Ptr_C &rhs) {
      auto wme_vector_merge = std::make_shared<WME_Vector>(*lhs);
      wme_vector_merge->wmes.insert(wme_vector_merge->wmes.end(), rhs->wmes.begin(), rhs->wmes.end());
      return wme_vector_merge;
    }

    void pass_tokens(const Rete_Node_Ptr &output) {
      for(auto &wme_vector : output_tokens)
        output->insert_wme_vector(wme_vector, shared());
    }

    WME_Bindings bindings;
    std::weak_ptr<Rete_Node> input0;
    std::weak_ptr<Rete_Node> input1;
    std::unordered_set<WME_Vector_Ptr_C, hash_deref<WME_Vector>, compare_deref> input0_tokens;
    std::unordered_set<WME_Vector_Ptr_C, hash_deref<WME_Vector>, compare_deref> input1_tokens;
    std::unordered_set<WME_Vector_Ptr_C, hash_deref<WME_Vector>, compare_deref> output_tokens;
  };

  inline void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    assert(join && !join->input0.lock() && !join->input1.lock());
    join->input0 = out0;
    join->input1 = out1;

    out0->outputs.insert(join);
    out1->outputs.insert(join);
    out0->pass_tokens(join);
    out1->pass_tokens(join);
  }

}

#endif
