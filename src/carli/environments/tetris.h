#ifndef TETRIS_H
#define TETRIS_H

#include "../agent.h"
#include "../environment.h"

#include <array>
#include <stdexcept>

namespace Tetris {
  enum Tetromino_Type {TET_LINE = 0, TET_SQUARE = 1, TET_T = 2, TET_S = 3, TET_Z = 4, TET_L = 5, TET_J = 6};
  enum {TET_TYPES = 7};
}

inline std::ostream & operator<<(std::ostream &os, const Tetris::Tetromino_Type &type);

namespace Tetris {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  class Feature;
  class Type;
  class Orientation;
  class Size;
  class Position;
  class Gaps;

  class Feature : public ::Feature {
  public:
    Feature() {}

    virtual Feature * clone() const = 0;

    int compare_axis(const ::Feature &rhs) const {
      return compare_axis(debuggable_cast<const Feature &>(rhs));
    }

    virtual int compare_axis(const Feature &rhs) const = 0;
    virtual int compare_axis(const Type &rhs) const = 0;
    virtual int compare_axis(const Orientation &rhs) const = 0;
    virtual int compare_axis(const Size &rhs) const = 0;
    virtual int compare_axis(const Position &rhs) const = 0;
    virtual int compare_axis(const Gaps &rhs) const = 0;

    virtual Rete::WME_Token_Index wme_token_index() const = 0;
  };

  class Type : public Feature_Enumerated<Feature> {
  public:
    enum Axis : size_t {CURRENT = 1, NEXT = 2};

    Type(const Axis &axis_, const Tetromino_Type &type_)
     : Feature_Enumerated<Feature>(type_), axis(axis_)
    {
    }

    Type * clone() const {
      return new Type(axis, Tetromino_Type(value));
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Type &rhs) const {
      return axis - rhs.axis;
    }
    int compare_axis(const Orientation &) const {
      return -1;
    }
    int compare_axis(const Size &) const {
      return -1;
    }
    int compare_axis(const Position &) const {
      return -1;
    }
    int compare_axis(const Gaps &) const {
      return -1;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(axis, 2);
    }

    void print(ostream &os) const {
      os << "type(" << (axis == CURRENT ? "current" : "next") << ':' << value << ')';
    }

    Axis axis;
  };

  class Orientation : public Feature_Enumerated<Feature> {
  public:
    enum Axis : size_t {ORIENTATION = 3};

    Orientation(const size_t &orientation_)
     : Feature_Enumerated(orientation_)
    {
    }

    Orientation * clone() const {
      return new Orientation(value);
    }

    int compare_axis(const Tetris::Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Type &) const {
      return 1;
    }
    int compare_axis(const Orientation &) const {
      return 0;
    }
    int compare_axis(const Size &) const {
      return -1;
    }
    int compare_axis(const Position &) const {
      return -1;
    }
    int compare_axis(const Gaps &) const {
      return -1;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(ORIENTATION, 2);
    }

    void print(ostream &os) const {
      os << "orientation(" << value << ')';
    }
  };

  class Size : public Feature_Ranged<Feature> {
  public:
    enum Axis : size_t {WIDTH = 4, HEIGHT = 5};

    Size(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_, true)
    {
    }

    Size * clone() const {
      return new Size(Axis(axis.first), bound_lower, bound_upper, depth, upper);
    }

    int compare_axis(const Tetris::Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Type &) const {
      return 1;
    }
    int compare_axis(const Orientation &) const {
      return 1;
    }
    int compare_axis(const Size &rhs) const {
      return Feature_Ranged_Data::compare_axis(rhs);
    }
    int compare_axis(const Position &) const {
      return -1;
    }
    int compare_axis(const Gaps &) const {
      return -1;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return axis;
    }

    void print(ostream &os) const {
      switch(axis.first) {
        case WIDTH: os << "width"; break;
        case HEIGHT: os << "height"; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }
  };

  class Position : public Feature_Ranged<Feature> {
  public:
    enum Axis : size_t {X = 6, Y = 7};

    Position(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_, true)
    {
    }

    Position * clone() const {
      return new Position(Axis(axis.first), bound_lower, bound_upper, depth, upper);
    }

    int compare_axis(const Tetris::Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Type &) const {
      return 1;
    }
    int compare_axis(const Orientation &) const {
      return 1;
    }
    int compare_axis(const Size &) const {
      return 1;
    }
    int compare_axis(const Position &rhs) const {
      return Feature_Ranged_Data::compare_axis(rhs);
    }
    int compare_axis(const Gaps &) const {
      return -1;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return axis;
    }

    void print(ostream &os) const {
      switch(axis.first) {
        case X: os << 'x'; break;
        case Y: os << 'y'; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }
  };

  class Gaps : public Feature_Ranged<Feature> {
  public:
    enum Axis : size_t {BENEATH = 8, CREATED = 9};

    Gaps(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_, true)
    {
    }

    Gaps * clone() const {
      return new Gaps(Axis(axis.first), bound_lower, bound_upper, depth, upper);
    }

    int compare_axis(const Tetris::Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Type &) const {
      return 1;
    }
    int compare_axis(const Orientation &) const {
      return 1;
    }
    int compare_axis(const Size &) const {
      return 1;
    }
    int compare_axis(const Position &) const {
      return 1;
    }
    int compare_axis(const Gaps &rhs) const {
      return Feature_Ranged_Data::compare_axis(rhs);
    }

    Rete::WME_Token_Index wme_token_index() const {
      return axis;
    }

    void print(ostream &os) const {
      switch(axis.first) {
        case BENEATH: os << "gaps-beneath"; break;
        case CREATED: os << "gaps-created"; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }
  };

  class Place : public Action {
  public:
    Place()
     : orientation(0)
    {
    }

    Place(const std::pair<size_t, size_t> &position_, const int &orientation_)
     : position(position_),
     orientation(orientation_)
    {
    }

    Place(const Rete::WME_Token &token)
     : position(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(Position::X, 2)]).value,
                debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(Position::Y, 2)]).value),
     orientation(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(Orientation::ORIENTATION, 2)]).value)
    {
    }

    Place * clone() const {
      return new Place(position, orientation);
    }

    int compare(const Action &rhs) const {
      return compare(debuggable_cast<const Place &>(rhs));
    }

    int compare(const Place &rhs) const {
      return position.first != rhs.position.first ? position.first - rhs.position.first :
             position.second != rhs.position.second ? position.second - rhs.position.second :
             orientation - rhs.orientation;
    }

    void print_impl(ostream &os) const {
      os << "place(" << position.first << ',' << position.second << ':' << orientation << ')';
    }

    std::pair<size_t, size_t> position;
    int orientation;
  };

  class Environment : public ::Environment {
  public:
    enum Outcome {OUTCOME_NULL, OUTCOME_ACHIEVED, OUTCOME_ENABLED, OUTCOME_PROHIBITED};

    struct Placement {
      Placement(const size_t &orientation_,
                const std::pair<size_t, size_t> &size_,
                const std::pair<size_t, size_t> &position_,
                const size_t &gaps_beneath_,
                const size_t &gaps_created_,
                const Outcome &outcome_1,
                const Outcome &outcome_2,
                const Outcome &outcome_3,
                const Outcome &outcome_4)
       : orientation(orientation_),
       size(size_),
       position(position_),
       gaps_beneath(gaps_beneath_),
       gaps_created(gaps_created_),
       outcome({{OUTCOME_NULL, outcome_1, outcome_2, outcome_3, outcome_4}})
      {
      }

      size_t orientation;
      std::pair<size_t, size_t> size;
      std::pair<size_t, size_t> position;
      size_t gaps_beneath;
      size_t gaps_created;
      std::array<Outcome, 5> outcome;
    };
    typedef std::vector<Placement> Placements;

//    const std::array<double, 5> score_line = {{0.0, 10.0, 20.0, 30.0, 40.0}}; /// No risk-reward tradeoff
    const std::array<double, 5> score_line = {{0.0, 10.0, 20.0, 40.0, 80.0}}; /// Risk-reward tradeoff
    const double score_failure = 0.0;

    Environment();

    Environment operator=(const Environment &rhs);

    Tetromino_Type get_current() const {return m_current;}
    Tetromino_Type get_next() const {return m_next;}

    const Placements & get_placements() const {return m_placements;}

  private:
    enum Result {PLACE_ILLEGAL, PLACE_GROUNDED, PLACE_UNGROUNDED};

    struct Tetromino : public std::array<std::array<bool, 4>, 4> {
      uint8_t width;
      uint8_t height;
    };

    void init_impl();

    reward_type transition_impl(const Action &action);
    void place_Tetromino(const Tetromino &tet, const std::pair<size_t, size_t> &position);

    void print_impl(ostream &os) const;

    static Tetromino generate_Tetromino(const Tetromino_Type &type, const int &orientation = 0);
    static uint8_t orientations_Tetromino(const Tetromino_Type &type);
    uint8_t clear_lines(const std::pair<size_t, size_t> &position);

    size_t gaps_beneath(const Tetromino &tet, const std::pair<size_t, size_t> &position) const;
    size_t gaps_created(const Tetromino &tet, const std::pair<size_t, size_t> &position) const;
    Outcome outcome(const uint8_t &lines_cleared, const Tetromino &tet, const std::pair<size_t, size_t> &position) const;

    Zeni::Random m_random_init;
    Zeni::Random m_random_selection;

    std::array<std::pair<std::array<bool, 10>, size_t>, 20> m_grid;
    Tetromino_Type m_current;
    Tetromino_Type m_next;

    void generate_placements();
    Placements m_placements;

    bool m_lookahead = false;
  };

  class Agent : public ::Agent {
  public:
    Agent(const std::shared_ptr< ::Environment> &env);
    ~Agent();

  private:
    template<typename SUBFEATURE, typename AXIS>
    void generate_rete_continuous(const Node_Unsplit_Ptr &node_unsplit,
                                  const std::function<action_ptrsc(const Rete::WME_Token &token)> &get_action,
                                  const AXIS &axis,
                                  const double &lower_bound,
                                  const double &upper_bound);
    void generate_rete();

    void generate_features();

    void update();

    const Rete::Symbol_Variable_Ptr_C m_first_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First);
    const Rete::Symbol_Variable_Ptr_C m_third_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::Third);

    const Rete::Symbol_Identifier_Ptr_C m_input_id = std::make_shared<Rete::Symbol_Identifier>("I1");
    const Rete::Symbol_Constant_String_Ptr_C m_input_attr = std::make_shared<Rete::Symbol_Constant_String>("input");
    const Rete::Symbol_Constant_String_Ptr_C m_action_attr = std::make_shared<Rete::Symbol_Constant_String>("action");
    const Rete::Symbol_Constant_String_Ptr_C m_type_current_attr = std::make_shared<Rete::Symbol_Constant_String>("type-current");
    const Rete::Symbol_Constant_String_Ptr_C m_type_next_attr = std::make_shared<Rete::Symbol_Constant_String>("type-next");
    const Rete::Symbol_Constant_String_Ptr_C m_orientation_attr = std::make_shared<Rete::Symbol_Constant_String>("orientation");
    const Rete::Symbol_Constant_String_Ptr_C m_width_attr = std::make_shared<Rete::Symbol_Constant_String>("width");
    const Rete::Symbol_Constant_String_Ptr_C m_height_attr = std::make_shared<Rete::Symbol_Constant_String>("height");
    const Rete::Symbol_Constant_String_Ptr_C m_x_attr = std::make_shared<Rete::Symbol_Constant_String>("x");
    const Rete::Symbol_Constant_String_Ptr_C m_y_attr = std::make_shared<Rete::Symbol_Constant_String>("y");
    const Rete::Symbol_Constant_String_Ptr_C m_gaps_beneath_attr = std::make_shared<Rete::Symbol_Constant_String>("gaps-beneath");
    const Rete::Symbol_Constant_String_Ptr_C m_gaps_created_attr = std::make_shared<Rete::Symbol_Constant_String>("gaps-created");
    const Rete::Symbol_Constant_String_Ptr_C m_true_value = std::make_shared<Rete::Symbol_Constant_String>("true");

    std::array<Rete::Symbol_Identifier_Ptr_C, 7> m_type_ids = {{std::make_shared<Rete::Symbol_Identifier>("LINE"),
                                                                std::make_shared<Rete::Symbol_Identifier>("SQUARE"),
                                                                std::make_shared<Rete::Symbol_Identifier>("T"),
                                                                std::make_shared<Rete::Symbol_Identifier>("S"),
                                                                std::make_shared<Rete::Symbol_Identifier>("Z"),
                                                                std::make_shared<Rete::Symbol_Identifier>("L"),
                                                                std::make_shared<Rete::Symbol_Identifier>("J")}};

    std::list<Rete::WME_Ptr_C> m_wmes_prev;
  };

}

std::ostream & operator<<(std::ostream &os, const Tetris::Tetromino_Type &type) {
  switch(type) {
    case Tetris::TET_LINE:   os << "----"; break;
    case Tetris::TET_SQUARE: os << "[]"; break;
    case Tetris::TET_T:      os << "T"; break;
    case Tetris::TET_S:      os << "S"; break;
    case Tetris::TET_Z:      os << "Z"; break;
    case Tetris::TET_L:      os << "L"; break;
    case Tetris::TET_J:      os << "J"; break;
    default: abort();
  }

  return os;
}

#endif
