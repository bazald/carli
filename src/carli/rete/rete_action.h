#ifndef RETE_ACTION_H
#define RETE_ACTION_H

#include "agenda.h"
#include "rete_node.h"

namespace Rete {

  class RETE_LINKAGE Rete_Action : public Rete_Node {
    Rete_Action(const Rete_Action &);
    Rete_Action & operator=(const Rete_Action &);

    friend RETE_LINKAGE void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out);
    friend class Rete_Action_to_Agenda;

  public:
    typedef std::function<void (const Rete_Action &rete_action, const WME_Token &wme_token)> Action;

    Rete_Action(Agenda &agenda_,
                const std::string &name_,
                const Action &action_ = [](const Rete_Action &, const WME_Token &){},
                const Action &retraction_ = [](const Rete_Action &, const WME_Token &){});

    ~Rete_Action();

    void destroy(Filters &filters, const Rete_Node_Ptr &output = Rete_Node_Ptr()) override;
    bool is_excised() const {return excised;}

    Rete_Node_Ptr_C parent_left() const override {return input->shared();}
    Rete_Node_Ptr_C parent_right() const override {return input->shared();}
    Rete_Node_Ptr parent_left() override {return input->shared();}
    Rete_Node_Ptr parent_right() override {return input->shared();}

    int64_t height() const override {return input->height() + 1;}

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;
    bool remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;

    void pass_tokens(Rete_Node * const &) override;
    void unpass_tokens(Rete_Node * const &) override;

    bool operator==(const Rete_Node &/*rhs*/) const override;

    void print_details(std::ostream &os) const override; ///< Formatted for dot: http://www.graphviz.org/content/dot-language

    void output_name(std::ostream &os, const int64_t &depth) const override;

    bool is_active() const override;

    static Rete_Action_Ptr find_existing(const Action &/*action_*/, const Action &/*retraction_*/, const Rete_Node_Ptr &/*out*/);

    const std::string & get_name() const {return name;}

    void set_action(const Action &action_) {
      action = action_;
    }

    void set_retraction(const Action &retraction_) {
      retraction = retraction_;
    }

  private:
    Rete_Node * input = nullptr;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input_tokens;
    std::string name;
    Action action;
    Action retraction;
    Agenda &agenda;
    bool excised = false;
  };

  RETE_LINKAGE void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out);

  class RETE_LINKAGE Rete_Action_to_Agenda {
    friend class Agenda;

    static const Rete_Action::Action & action(const Rete_Action &rete_action) {
      return rete_action.action;
    }

    static const Rete_Action::Action & retraction(const Rete_Action &rete_action) {
      return rete_action.retraction;
    }
  };

}

#endif
