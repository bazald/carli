#ifndef TETRIS_H
#define TETRIS_H

#include "../agent.h"
#include "../environment.h"

#include <array>
#include <stdexcept>

namespace Tetris {
  enum Tetromino_Type {TET_LINE, TET_SQUARE, TET_T, TET_S, TET_Z, TET_L, TET_J};
}

inline std::ostream & operator<<(std::ostream &os, const Tetris::Tetromino_Type &type);

namespace Tetris {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  class Feature;
  class Type;
  class Size;
  class Position;
  class Orientation;

  class Feature : public ::Feature {
  public:
    Feature() {}

    virtual Feature * clone() const = 0;

    int compare_axis(const ::Feature &rhs) const {
      return compare_axis(debuggable_cast<const Feature &>(rhs));
    }

    virtual int compare_axis(const Feature &rhs) const = 0;
    virtual int compare_axis(const Type &rhs) const = 0;
    virtual int compare_axis(const Size &rhs) const = 0;
    virtual int compare_axis(const Position &rhs) const = 0;
    virtual int compare_axis(const Orientation &rhs) const = 0;

    virtual Rete::WME_Token_Index wme_token_index() const = 0;
  };

  class Type : public Feature {
  public:
    Type(const Tetromino_Type &type_)
     : type(type_)
    {
    }

    Type * clone() const {
      return new Type(type);
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Type &) const {
      return 0;
    }
    int compare_axis(const Size &) const {
      return -1;
    }
    int compare_axis(const Position &) const {
      return -1;
    }
    int compare_axis(const Orientation &) const {
      return -1;
    }

    int compare_value(const ::Feature &rhs) const {
      return type - debuggable_cast<const Type &>(rhs).type;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(0, 2);
    }

    void print(ostream &os) const {
      os << "type(" << type << ')';
    }

    Tetromino_Type type;
  };

  class Size : public Feature_Ranged<Feature> {
  public:
    enum Axis : size_t {WIDTH = 0, HEIGHT = 1};

    Size(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_)
    {
    }

    Size * clone() const {
      return new Size(Axis(axis.first), bound_lower, bound_upper, depth, upper);
    }

    int compare_axis(const Tetris::Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Type &) const {
      return -1;
    }
    int compare_axis(const Size &rhs) const {
      return Feature_Ranged<Feature>::compare_axis(rhs);
    }
    int compare_axis(const Position &) const {
      return -1;
    }
    int compare_axis(const Orientation &) const {
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
    enum Axis : size_t {X = 1, Y = 2};

    Position(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_)
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
    int compare_axis(const Size &) const {
      return 1;
    }
    int compare_axis(const Position &rhs) const {
      return Feature_Ranged<Feature>::compare_axis(rhs);
    }
    int compare_axis(const Orientation &) const {
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
    int compare_axis(const Size &) const {
      return 1;
    }
    int compare_axis(const Position &) const {
      return 1;
    }
    int compare_axis(const Orientation &) const {
      return 0;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(ORIENTATION, 2);
    }

    void print(ostream &os) const {
      os << "orientation(" << value << ')';
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
     : position(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(1, 2)]).value,
                debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(2, 2)]).value),
     orientation(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(3, 2)]).value)
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
    typedef std::list<std::pair<std::pair<size_t, size_t>, int> > Placements;

    const std::array<double, 5> score_line = {{1.0, 10.0, 20.0, 40.0, 80.0}};
    const double score_failure = 0.0;

    Environment();

    const Placements & get_placements() const {return m_placements;}

  private:
    enum Placement {PLACE_ILLEGAL, PLACE_GROUNDED, PLACE_UNGROUNDED};
    typedef std::array<std::array<bool, 4>, 4> Tetromino;

    void init_impl();

    reward_type transition_impl(const Action &action);

    void print_impl(ostream &os) const;

    Tetromino generate_Tetronmino(const Tetromino_Type &type, const int &orientation = 0);
    uint8_t orientations_Tetronmino(const Tetromino_Type &type);
    double clear_lines();

    Placement test_placement(const Tetromino &tet, const std::pair<size_t, size_t> &position);
    uint8_t width_Tetronmino(const Tetromino &tet);
    uint8_t height_Tetronmino(const Tetromino &tet);

    Zeni::Random m_random_init;
    Zeni::Random m_random_selection;

    std::array<std::array<bool, 10>, 20> m_grid;
    Tetromino_Type m_current;
    Tetromino_Type m_next;

    void generate_placements();
    Placements m_placements;
  };

  class Agent : public ::Agent {
  public:
    Agent(const std::shared_ptr< ::Environment> &env);
    ~Agent();

  private:
    void generate_rete();

    void generate_features();

    void update();

    const Rete::Symbol_Variable_Ptr_C m_first_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First);
    const Rete::Symbol_Variable_Ptr_C m_third_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::Third);

    const Rete::Symbol_Identifier_Ptr_C m_input_id = std::make_shared<Rete::Symbol_Identifier>("I1");
    const Rete::Symbol_Constant_String_Ptr_C m_input_attr = std::make_shared<Rete::Symbol_Constant_String>("input");
    const Rete::Symbol_Constant_String_Ptr_C m_action_attr = std::make_shared<Rete::Symbol_Constant_String>("action");
    const Rete::Symbol_Constant_String_Ptr_C m_current_attr = std::make_shared<Rete::Symbol_Constant_String>("current");
    const Rete::Symbol_Constant_String_Ptr_C m_next_attr = std::make_shared<Rete::Symbol_Constant_String>("next");
    const Rete::Symbol_Constant_String_Ptr_C m_x_attr = std::make_shared<Rete::Symbol_Constant_String>("x");
    const Rete::Symbol_Constant_String_Ptr_C m_y_attr = std::make_shared<Rete::Symbol_Constant_String>("y");
    const Rete::Symbol_Constant_String_Ptr_C m_orientation_attr = std::make_shared<Rete::Symbol_Constant_String>("orientation");
    const Rete::Symbol_Constant_String_Ptr_C m_width_attr = std::make_shared<Rete::Symbol_Constant_String>("width");
    const Rete::Symbol_Constant_String_Ptr_C m_height_attr = std::make_shared<Rete::Symbol_Constant_String>("height");
    const Rete::Symbol_Constant_String_Ptr_C m_type_attr = std::make_shared<Rete::Symbol_Constant_String>("type");
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
