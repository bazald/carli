#ifndef RETE_ACTION_H
#define RETE_ACTION_H

#include "agenda.h"
#include "rete_node.h"

namespace Rete {

  class Rete_Action : public Rete_Node {
    Rete_Action(const Rete_Action &);
    Rete_Action & operator=(const Rete_Action &);

    friend void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out);

  public:
    typedef std::function<void (const Rete_Action &rete_action, const WME_Token &wme_token)> Action;

    Rete_Action(
#ifdef USE_AGENDA
                Agenda &actions_,
                Agenda &retractions_,
#else
                Agenda &,
                Agenda &,
#endif
                const Action &action_ = [](const Rete_Action &, const WME_Token &){},
                const Action &retraction_ = [](const Rete_Action &, const WME_Token &){},
                const bool &attach_immediately = true);

    ~Rete_Action();

    Rete_Node_Ptr_C parent() const {return input.lock();}
    Rete_Node_Ptr parent() {return input.lock();}

    void destroy(Filters &filters, const Rete_Node_Ptr &output = Rete_Node_Ptr());

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from);
    void remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from);

    virtual void pass_tokens(const Rete_Node_Ptr &);

    bool operator==(const Rete_Node &/*rhs*/) const;

    static Rete_Action_Ptr find_existing(const Action &/*action_*/, const Action &/*retraction_*/, const Rete_Node_Ptr &/*out*/);

    bool attached() const {return m_attached;}
    void attach();
    void detach();

    void set_action(const Action &action_) {
      action = action_;
    }

    void set_retraction(const Action &retraction_) {
      retraction = retraction_;
    }

  private:
    std::weak_ptr<Rete_Node> input;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input_tokens;
    Action action;
    Action retraction;
#ifdef USE_AGENDA
    Agenda &actions;
    Agenda &retractions;
#endif
    bool m_attached;
  };

  void bind_to_action(const Rete_Action_Ptr &action, const Rete_Node_Ptr &out);

}

#endif
