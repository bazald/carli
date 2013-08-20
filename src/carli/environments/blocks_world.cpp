#include "blocks_world.h"

namespace Blocks_World {

  Environment::Environment() {
    init_impl();

    {
      m_goal.push_front(Stack());
      Stack &stack = *m_goal.begin();
      stack.push_front(3);
      stack.push_front(2);
      stack.push_front(1);
    }
  }

  void Environment::init_impl() {
    m_blocks.clear();

    {
      m_blocks.push_front(Stack());
      Stack &stack = *m_blocks.begin();
      stack.push_front(2);
    }
    {
      m_blocks.push_front(Stack());
      Stack &stack = *m_blocks.begin();
      stack.push_front(1);
      stack.push_front(3);
    }
  }

  Agent::reward_type Environment::transition_impl(const Action &action) {
    const Move &move = debuggable_cast<const Move &>(action);

    Stacks::iterator src = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && *stack.begin() == move.block;
    });
    Stacks::iterator dest = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && *stack.begin() == move.dest;
    });

    if(src == m_blocks.end())
      return -10.0; ///< throw std::runtime_error("Attempt to move Block from under another Block.");
    if(dest == m_blocks.end())
      dest = m_blocks.insert(m_blocks.begin(), Stack());

    src->pop_front();
    dest->push_front(move.block);

    if(src->empty())
      m_blocks.erase(src);

    return -1.0;
  }

  void Environment::print_impl(ostream &os) const {
    os << "Blocks World:" << endl;
    for(const Stack &stack : m_blocks) {
      for(const block_id &id : stack)
        os << ' ' << id;
      os << endl;
    }
  }

  Agent::Agent(const std::shared_ptr< ::Environment> &env)
   : ::Agent(env)
  {
    insert_wme(m_wme_blink);
    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
    destroy();
    for(auto &action : m_action)
      action.delete_and_zero();
  }

  void Agent::generate_rete() {
    for(const auto &action : m_action) {
      const auto &move = debuggable_cast<const Move &>(*action.get());

      auto filter_top = make_filter(Rete::WME(m_s_id, m_block_attr, get_block_id(move.block)));
      auto filter_bottom = make_filter(Rete::WME(m_s_id, m_block_attr, get_block_id(move.dest)));
      Rete::WME_Bindings state_bindings;
      state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 0), Rete::WME_Token_Index(0, 0)));
      auto join_top_and_dest = make_join(state_bindings, filter_top,filter_bottom);

      auto filter_blink = make_filter(*m_wme_blink);
      auto join_blink = make_join(Rete::WME_Bindings(), join_top_and_dest, filter_blink);

      auto rl = std::make_shared<RL>(*this, 1,
                                     RL::Range(std::make_pair(0.0, 0.0), std::make_pair(1.0, 1.0)),
                                     RL::Lines());
      rl->q_value = new Q_Value(0.0, Q_Value::Type::UNSPLIT, rl->depth);
      ++this->m_q_value_count;
//      rl->fringe_values = new RL::Fringe_Values;
      rl->action = make_action_retraction([this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
//        if(!this->specialize(action, rl))
          this->m_next_q_values[action].push_back(rl->q_value);
      }, [this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
        this->purge_q_value_next(action, rl->q_value);
      }, join_blink);
    }
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &blocks = env->get_blocks();
    const auto &goal = env->get_goal();
    std::list<Rete::WME_Ptr_C> wmes_current;

    for(auto bt = blocks.begin(), bend = blocks.end(); bt != bend; ++bt) {
      for(auto st = bt->begin(), send = bt->end(); st != send; ++st) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_block_attr, get_block_id(*st)));
        wmes_current.push_back(std::make_shared<Rete::WME>(get_block_id(*st), m_name_attr, get_block_name(*st)));
      }

      wmes_current.push_back(std::make_shared<Rete::WME>(get_block_id(*bt->begin()), m_clear_attr, m_true_value));

      for(auto st = bt->begin(), stn = ++bt->begin(), send = bt->end(); stn != send; st = stn, ++stn)
        wmes_current.push_back(std::make_shared<Rete::WME>(get_block_id(*st), m_on_top_attr, get_block_id(*stn)));

      auto base = bt->rbegin();
      const auto stack = std::find_if(goal.begin(), goal.end(), [&base](const Environment::Stack &stack)->bool{return std::find(stack.begin(), stack.end(), *base) != stack.end();});
      assert(stack != goal.end());
      for(auto base_goal = stack->rbegin(); base != bt->rend() && base_goal != stack->rend() && *base == *base_goal; ++base, ++base_goal)
        wmes_current.push_back(std::make_shared<Rete::WME>(get_block_id(*base), m_in_place_attr, m_true_value));
    }

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_block_attr, m_table_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_table_id, m_name_attr, m_table_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_table_id, m_clear_attr, m_true_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_table_id, m_in_place_attr, m_true_value));

    remove_wme(m_wme_blink);

    for(auto wt = m_wmes_prev.begin(), wend = m_wmes_prev.end(); wt != wend; ) {
      const auto found = std::find_if(wmes_current.begin(), wmes_current.end(), [wt](const Rete::WME_Ptr_C &wme_)->bool{return *wme_ == **wt;});
      if(found == wmes_current.end()) {
        remove_wme(*wt);
        m_wmes_prev.erase(wt++);
      }
      else {
        wmes_current.erase(found);
        ++wt;
      }
    }

    for(auto &wme : wmes_current) {
      const auto found = std::find_if(m_wmes_prev.begin(), m_wmes_prev.end(), [wme](const Rete::WME_Ptr_C &wme_)->bool{return *wme_ == *wme;});
      if(found == m_wmes_prev.end()) {
        m_wmes_prev.push_back(wme);
        insert_wme(wme);
      }
    }

    insert_wme(m_wme_blink);
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    m_metastate = env->get_blocks() == env->get_goal() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
  }

  const Rete::Symbol_Identifier_Ptr_C & Agent::get_block_id(const block_id &block) {
    switch(block) {
      case 0: return m_table_id;
      case 1: return m_a_id;
      case 2: return m_b_id;
      case 3: return m_c_id;
      default: abort();
    }
  }

  const Rete::Symbol_Constant_String_Ptr_C & Agent::get_block_name(const block_id &block) {
    switch(block) {
      case 0: return m_table_value;
      case 1: return m_a_value;
      case 2: return m_b_value;
      case 3: return m_c_value;
      default: abort();
    }
  }

}
