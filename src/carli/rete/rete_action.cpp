#include "rete_action.h"

#include "rete_agent.h"

namespace Rete {

  Rete_Action::Rete_Action(const std::string &name_,
                           const Action &action_,
                           const Action &retraction_)
    : name(name_),
    action(action_),
    retraction(retraction_)
  {
    assert(!name.empty());
  }

  Rete_Action::~Rete_Action() {
    if(!excised) {
      for(auto &wme_token : input_tokens)
        retraction(*this, *wme_token);
    }
  }

  void Rete_Action::destroy(Rete_Agent &agent, const Rete_Node_Ptr &
#ifndef NDEBUG
                                                                    output
#endif
                                                                          ) {
    assert(!output);

    if(!destruction_suppressed && !excised) {
      excised = true;
      agent.excise_rule(name, false);

      //std::cerr << "Destroying: ";
      //output_name(std::cerr);
      //std::cerr << std::endl;

      for(auto &wme_token : input_tokens)
        retraction(*this, *wme_token);

      input->destroy(agent, shared());
    }
  }

  void Rete_Action::insert_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &
#ifndef NDEBUG
                                                                                              from
#endif
                                                                                                  ) {
    assert(from == input);

    input_tokens.push_back(wme_token);

//#ifndef NDEBUG
//    std::cerr << "Firing action: ";
//    output_name(std::cerr);
//    std::cerr << std::endl;
//#endif

    agent.get_agenda().insert_action(debuggable_pointer_cast<Rete_Action>(shared()), wme_token);
  }

  bool Rete_Action::remove_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &
#ifndef NDEBUG
                                                                                              from
#endif
                                                                                                  ) {
    assert(from == input);

    auto found = find(input_tokens, wme_token);
    if(found != input_tokens.end())
    // TODO: change from the 'if' to the 'assert', ensuring that we're not wasting time on non-existent removals
    //assert(found != input_tokens.end());
    {
      agent.get_agenda().insert_retraction(debuggable_pointer_cast<Rete_Action>(shared()), wme_token);

      input_tokens.erase(found);
    }

    return input_tokens.empty();
  }

  bool Rete_Action::matches_wme_token(const WME_Token_Ptr_C &wme_token) const {
    return find(input_tokens, wme_token) != input_tokens.end();
  }

  void Rete_Action::pass_tokens(Rete_Agent &, Rete_Node * const &) {
    abort();
  }

  void Rete_Action::unpass_tokens(Rete_Agent &, Rete_Node * const &) {
    abort();
  }

  bool Rete_Action::operator==(const Rete_Node &/*rhs*/) const {
//       if(auto rete_action = dynamic_cast<const Rete_Action *>(&rhs))
//         return action == rete_action->action && retraction == rete_action->retraction && input == rete_action->input;
    return false;
  }

  void Rete_Action::print_details(std::ostream &os) const {
    os << "  " << intptr_t(this) << " [label=\"Act\"];" << std::endl;
    os << "  " << intptr_t(input) << " -> " << intptr_t(this) << " [color=red];" << std::endl;
  }

  void Rete_Action::output_name(std::ostream &os, const int64_t &depth) const {
    os << "a(";
    if(input && depth)
      input->output_name(os, depth - 1);
    os << ')';
  }

  bool Rete_Action::is_active() const {
    return !input_tokens.empty();
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

  void bind_to_action(Rete_Agent &agent, const Rete_Action_Ptr &action, const Rete_Node_Ptr &out, const Variable_Indices &variables) {
    assert(action && !action->input);
    action->input = out.get();
    action->variables = variables;
    out->insert_output_enabled(action);
    out->pass_tokens(agent, action.get());
  }

}
