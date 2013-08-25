#include "rete_action.h"

namespace Rete {

  Rete_Action::Rete_Action(Agenda &agenda_,
                           const Action &action_,
                           const Action &retraction_)
    : action(action_),
    retraction(retraction_),
    agenda(agenda_)
  {
  }

  Rete_Action::~Rete_Action() {
    for(auto &wme_token : input_tokens)
      retraction(*this, *wme_token);
  }

  void Rete_Action::destroy(Filters &filters, const Rete_Node_Ptr &
#ifndef NDEBUG
                                                                    output
#endif
                                                                          ) {
    assert(!output);
    input.lock()->destroy(filters, shared());
  }

  void Rete_Action::insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &
#ifndef NDEBUG
                                                                                              from
#endif
                                                                                                  ) {
    assert(from == input.lock());

    input_tokens.push_back(wme_token);

    agenda.insert_back(shared(), [this,wme_token](){action(*this, *wme_token);});
  }

  void Rete_Action::remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &
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
      agenda.insert_front(shared(), [this,wme_token](){retraction(*this, *wme_token);});

      input_tokens.erase(found);
    }
  }

  void Rete_Action::pass_tokens(const Rete_Node_Ptr &) {
    abort();
  }

  bool Rete_Action::operator==(const Rete_Node &/*rhs*/) const {
//       if(auto rete_action = dynamic_cast<const Rete_Action *>(&rhs))
//         return action == rete_action->action && retraction == rete_action->retraction && input.lock() == rete_action->input.lock();
    return false;
  }

  Rete_Action_Ptr Rete_Action::find_existing(const Action &/*action_*/, const Action &/*retraction_*/, const Rete_Node_Ptr &/*out*/) {
//       for(auto &o : out->get_outputs()) {
//         if(auto existing_action = std::dynamic_pointer_cast<Rete_Action>(o)) {
//           if(action_ == existing_action->action && retraction_ == existing_action->retraction)
//             return existing_action;
//         }
//       }

    return nullptr;
  }

  void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out) {
    assert(action && !action->input.lock());
    action->input = out;

    out->insert_output(action);
    out->pass_tokens(action);
  }

}
