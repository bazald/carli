#include "blocks_world.h"

#include "carli/parser/rete_parser.h"

namespace Blocks_World {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

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

  int64_t Environment::num_blocks() const {
    int64_t nb = 0;
    for(const auto &blocks : m_blocks)
      nb += blocks.size();
    return nb;
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
//    {
//      m_blocks.push_front(Stack());
//      Stack &stack = *m_blocks.begin();
//      stack.push_front(4);
//    }
//    {
//      m_blocks.push_front(Stack());
//      Stack &stack = *m_blocks.begin();
//      stack.push_front(6);
//      stack.push_front(5);
//    }
  }

  std::pair<Agent::reward_type, Agent::reward_type> Environment::transition_impl(const Carli::Action &action) {
    const Move &move = debuggable_cast<const Move &>(action);

    Stacks::iterator src = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && *stack.begin() == move.block;
    });
    Stacks::iterator dest = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && *stack.begin() == move.dest;
    });

    if(src == m_blocks.end())
      return std::make_pair(-10.0, -10.0); ///< throw std::runtime_error("Attempt to move Block from under another Block.");
    if(dest == m_blocks.end())
      dest = m_blocks.insert(m_blocks.begin(), Stack());

    src->pop_front();
    dest->push_front(move.block);

    if(src->empty())
      m_blocks.erase(src);

    return std::make_pair(-1.0, -1.0);
  }

  void Environment::print_impl(ostream &os) const {
    os << "Blocks World (Table is Left):" << endl;
    for(const Stack &stack : m_blocks) {
      for(auto it = stack.rbegin(), iend = stack.rend(); it != iend; ++it)
        os << ' ' << *it;
      os << endl;
    }
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env)
   : Carli::Agent(env, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(variables, token);}),
   m_block_ids({{Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("TABLE")),
                 Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("A")),
                 Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("B")),
                 Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("C"))//,
//                 Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("D")),
//                 Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("E")),
////                 Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("F"))
               }}),
   m_block_names({{Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(0)),
                   Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1)),
                   Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(2)),
                   Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(3))//,
//                   Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(4)),
//                   Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(5)),
//                   Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(6))
                 }})
  {
    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
    destroy();
  }

  void Agent::generate_rete() {
    std::string rules_in = dynamic_cast<const Option_String &>(Options::get_global()["rules"]).get_value();
    if(rules_in == "default")
      rules_in = "rules/blocks-world.carli";
    if(rete_parse_file(*this, rules_in))
      abort();
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &blocks = env->get_blocks();
    const auto &goal = env->get_goal();
    std::list<Rete::WME_Ptr_C> wmes_current;

    std::ostringstream oss;
//    for(size_t block = 1; block != m_block_ids.size(); ++block) {
//      for(size_t dest = 0; dest != m_block_ids.size(); ++dest) {
//        if(block == dest)
//          continue;
//        oss << "move-" << block << '-' << dest;
//        Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
//        oss.str("");
//        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
//        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_block_attr, m_block_ids[block]));
//        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_block_ids[dest]));
//      }
//    }
    for(auto stack : blocks) {
      for(auto dest_stack : blocks) {
        auto block = stack.front();
        auto dest = dest_stack.front();
        if(block == dest)
          continue;
        oss << "move-" << block << '-' << dest;
        Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
        oss.str("");
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_block_attr, m_block_ids[block]));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_block_ids[dest]));
      }
    }
    for(auto stack : blocks) {
      auto block = stack.front();
      oss << "move-" << block << '-' << 0;
      Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
      oss.str("");
      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_block_attr, m_block_ids[block]));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_block_ids[0]));
    }

    for(auto bt = blocks.begin(), bend = blocks.end(); bt != bend; ++bt) {
//      int64_t height = 0;
      for(auto st = bt->begin(), send = bt->end(); st != send; ++st) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_block_attr, m_block_ids[size_t(*st)]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*st)], m_name_attr, m_block_names[size_t(*st)]));
//        wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*st)], m_height_attr, std::make_shared<Rete::Symbol_Constant_Int>(++height)));

//        const double brightness = m_random.frand_lte();
//        wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*st)], m_brightness_attr, std::make_shared<Rete::Symbol_Constant_Float>(brightness)));
//        if(brightness > 0.5)
//          wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*st)], m_glowing_attr, m_true_value));
      }

//      wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*bt->begin())], m_clear_attr, m_true_value));

//      for(auto st = bt->begin(), stn = ++bt->begin(), send = bt->end(); stn != send; st = stn, ++stn)
//        wmes_current.push_back(std::make_shared<Rete::WME>(get_block_id(*st), m_on_top_attr, get_block_id(*stn)));

      auto base = bt->rbegin();
      const auto stack = std::find_if(goal.begin(), goal.end(), [&base](const Environment::Stack &stack)->bool{return std::find(stack.begin(), stack.end(), *base) != stack.end();});
      if(stack != goal.end()) {
        for(auto base_goal = stack->rbegin(); base != bt->rend() && base_goal != stack->rend() && *base == *base_goal; ++base, ++base_goal)
          wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*base)], m_in_place_attr, m_true_value));
      }
    }

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_block_attr, m_block_ids[0]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[0], m_name_attr, m_block_names[0]));
//    wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[0], m_clear_attr, m_true_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[0], m_in_place_attr, m_true_value));
//    wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[0], m_height_attr, std::make_shared<Rete::Symbol_Constant_Int>(0)));

    Rete::Agenda::Locker locker(agenda);
    CPU_Accumulator cpu_accumulator(*this);

    if(get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes")) {
      for(auto wt = m_wmes_prev.begin(), wend = m_wmes_prev.end(); wt != wend; ++wt)
        remove_wme(*wt);
      m_wmes_prev.clear();
    }
    else {
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
    }

    for(auto &wme : wmes_current) {
      const auto found = std::find_if(m_wmes_prev.begin(), m_wmes_prev.end(), [wme](const Rete::WME_Ptr_C &wme_)->bool{return *wme_ == *wme;});
      if(found == m_wmes_prev.end()) {
        m_wmes_prev.push_back(wme);
        insert_wme(wme);
      }
    }
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &blocks = env->get_blocks();
    const auto &goal = env->get_goal();

    for(const auto &gstack : goal) {
      if(std::find(blocks.begin(), blocks.end(), gstack) == blocks.end())
        return;
    }

    m_metastate = Metastate::SUCCESS;
  }

}
