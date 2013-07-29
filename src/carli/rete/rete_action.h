#ifndef RETE_ACTION_H
#define RETE_ACTION_H

#include "rete_node.h"

namespace Rete {

  class Rete_Action : public Rete_Node {
    Rete_Action(const Rete_Action &);
    Rete_Action & operator=(const Rete_Action &);

    friend void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out);

  public:
    typedef std::function<void (const Rete_Action &rete_action, const WME_Token &wme_token)> Action;

    Rete_Action(const Action &action_ = [](const Rete_Action &, const WME_Token &){},
                const Action &retraction_ = [](const Rete_Action &, const WME_Token &){})
      : action(action_), retraction(retraction_)
    {
    }

    Rete_Node_Ptr_C parent() const {return input.lock();}
    Rete_Node_Ptr parent() {return input.lock();}

    void destroy(std::unordered_set<Rete_Filter_Ptr> &filters, const Rete_Node_Ptr &
#ifndef NDEBUG
                                                                                    output
#endif
                                                                                           = Rete_Node_Ptr()) {
      assert(!output);
      for(auto &wme_token : input_tokens)
        retraction(*this, *wme_token);
      input.lock()->destroy(filters, shared());
    }

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &
#ifndef NDEBUG
                                                                                   from
#endif
                                                                                       ) {
      assert(from == input.lock());

      input_tokens.insert(wme_token);

      action(*this, *wme_token);
    }

    void remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &
#ifndef NDEBUG
                                                                                   from
#endif
                                                                                       ) {
      assert(from == input.lock());

      auto found = find(input_tokens, wme_token);
      if(found != input_tokens.end())
      // TODO: change from the 'if' to the 'assert', ensuring that we're not wasting time on non-existent removals
      //assert(found != input_tokens.end());
      {
        retraction(*this, *wme_token);

        input_tokens.erase(found);
      }
    }

    virtual void pass_tokens(const Rete_Node_Ptr &) {
      abort();
    }

    bool operator==(const Rete_Node &/*rhs*/) const {
//       if(auto rete_action = dynamic_cast<const Rete_Action *>(&rhs))
//         return action == rete_action->action && retraction == rete_action->retraction && input.lock() == rete_action->input.lock();
      return false;
    }

    static Rete_Action_Ptr find_existing(const Action &/*action_*/, const Action &/*retraction_*/, const Rete_Node_Ptr &/*out*/) {
//       for(auto &o : out->get_outputs()) {
//         if(auto existing_action = std::dynamic_pointer_cast<Rete_Action>(o)) {
//           if(action_ == existing_action->action && retraction_ == existing_action->retraction)
//             return existing_action;
//         }
//       }

      return nullptr;
    }

  private:
    std::weak_ptr<Rete_Node> input;
    std::unordered_set<WME_Token_Ptr_C> input_tokens;
    Action action;
    Action retraction;
  };

  inline void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out) {
    assert(action && !action->input.lock());
    action->input = out;

    out->outputs.push_back(action);
    out->pass_tokens(action);
  }

}

#endif
