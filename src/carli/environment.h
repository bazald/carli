#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <utility>

template <typename TYPE>
struct Feature : public Zeni::Pool_Allocator<TYPE> {
  typedef typename Zeni::Linked_List<Feature> List;
  typedef typename List::iterator iterator;

  Feature()
    : features(this)
  {
  }

  virtual ~Feature() {}

  virtual void print(std::ostream &os) const = 0;

  List features;
};

template <typename TYPE>
std::ostream & operator << (std::ostream &os, const Feature<TYPE> &feature) {
  feature.print(os);
  return os;
}

template <typename TYPE>
struct Action : public Zeni::Pool_Allocator<TYPE> {
  typedef typename Zeni::Linked_List<Action> List;
  typedef typename List::iterator iterator;

  Action()
    : candidates(this)
  {
  }

  virtual ~Action() {}

  virtual void print(std::ostream &os) const = 0;

  List candidates;
};

template <typename TYPE>
std::ostream & operator << (std::ostream &os, const Action<TYPE> &action) {
  action.print(os);
  return os;
}

template <typename FEATURE, typename ACTION>
class Environment {
public:
  typedef FEATURE feature_type;
  typedef ACTION action_type;
  typedef double reward_type;
  typedef size_t step_count_type;
  enum metastate_type {NON_TERMINAL, SUCCESS, FAILURE};

  Environment()
   : m_metastate(NON_TERMINAL),
   m_features(nullptr),
   m_candidates(nullptr),
   m_total_reward(reward_type()),
   m_step_count(step_count_type())
  {
  }

  Environment(const Environment &rhs)
   : m_metastate(rhs.m_metastate),
   m_features(nullptr),
   m_candidates(nullptr),
   m_total_reward(rhs.m_total_reward),
   m_step_count(rhs.m_step_count)
  {
    if(rhs.m_candidates)
      generate_lists();
  }

  Environment(Environment &&rhs)
   : m_metastate(rhs.m_metastate),
   m_features(rhs.m_features),
   m_candidates(rhs.m_candidates),
   m_total_reward(rhs.m_total_reward),
   m_step_count(rhs.m_step_count)
  {
    rhs.m_candidates = 0;
  }

  Environment & operator= (const Environment &rhs) {
    Environment temp(rhs);

    std::swap(temp.m_metastate, m_metastate);
    std::swap(temp.m_features, m_features);
    std::swap(temp.m_candidates, m_candidates);
    std::swap(temp.m_total_reward, m_total_reward);
    std::swap(temp.m_step_count, m_step_count);

    return *this;
  }

  Environment & operator= (Environment &&rhs) {
    Environment temp(std::move(rhs));

    std::swap(temp.m_metastate, m_metastate);
    std::swap(temp.m_features, m_features);
    std::swap(temp.m_candidates, m_candidates);
    std::swap(temp.m_total_reward, m_total_reward);
    std::swap(temp.m_step_count, m_step_count);

    return *this;
  }

  metastate_type get_metastate() const {return m_metastate;}
  reward_type get_total_reward() const {return m_total_reward;}
  step_count_type get_step_count() const {return m_step_count;}
  const feature_type * get_features() const {return m_features;}
  const action_type * get_candidates() const {return m_candidates;}

  void init() {
    m_metastate = NON_TERMINAL;
    m_total_reward = reward_type();
    m_step_count = step_count_type();

    destroy_lists();

    init_impl();

    generate_lists();
  }

  reward_type transition(const action_type &action) {
    const reward_type reward = transition_impl(action);

    m_total_reward += reward;
    ++m_step_count;

    destroy_lists();
    generate_lists();

    return reward;
  }

  void print(std::ostream &os) const {
    print_impl(os);
  }

protected:
  metastate_type m_metastate;
  feature_type * m_features;
  action_type * m_candidates;

private:
  virtual void init_impl() = 0;
  virtual reward_type transition_impl(const action_type &action) = 0;
  virtual void print_impl(std::ostream &os) const = 0;
  virtual void generate_lists() = 0;
  virtual void destroy_lists() = 0;

  reward_type m_total_reward;
  step_count_type m_step_count;
};

template <typename FEATURE, typename ACTION>
std::ostream & operator << (std::ostream &os, const Environment<FEATURE, ACTION> &env) {
  env.print(os);
  return os;
}

#endif

