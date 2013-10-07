#ifndef TETRIS_H
#define TETRIS_H

#include "../agent.h"
#include "../environment.h"

#include <array>
#include <stdexcept>

namespace Tetris {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  class Width;
  class Height;

  class Feature;
  class Feature : public Feature_Present {
  public:
    Feature(const bool &present_)
     : Feature_Present(present_)
    {
    }

    virtual Feature * clone() const = 0;

    int compare_axis(const ::Feature &rhs) const {
      return compare_axis(debuggable_cast<const Feature &>(rhs));
    }

    virtual int compare_axis(const Feature &rhs) const = 0;
    virtual int compare_axis(const Width &rhs) const = 0;
    virtual int compare_axis(const Height &rhs) const = 0;

    virtual Rete::WME_Token_Index wme_token_index() const = 0;
  };

  class Width : public Feature {
  public:
    Width(const size_t &width_, const bool &present_)
     : Feature(present_),
     width(width_)
    {
    }

    Width * clone() const {
      return new Width(width, this->present);
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Width &rhs) const {
      return width - rhs.width;
    }
    int compare_axis(const Height &) const {
      return -1;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(3, 2);
    }

    void print(ostream &os) const {
      if(!present)
        os << '!';
      os << "width(" << width << ')';
    }

    size_t width;
  };

  class Height : public Feature {
  public:
    Height(const size_t &height_, const bool &present_)
     : Feature(present_),
     height(height_)
    {
    }

    Height * clone() const {
      return new Height(height, this->present);
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Width &) const {
      return 1;
    }
    int compare_axis(const Height &rhs) const {
      return height - rhs.height;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(4, 2);
    }

    void print(ostream &os) const {
      if(!present)
        os << '!';
      os << "height(" << height << ')';
    }

    size_t height;
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
    Environment();

  private:
    enum Tetromino_Type {TET_LINE, TET_SQUARE, TET_T, TET_S, TET_Z, TET_L, TET__l};
    enum Placement {PLACE_ILLEGAL, PLACE_GROUNDED, PLACE_UNGROUNDED};
    typedef std::array<std::array<bool, 4>, 4> Tetromino;

    void init_impl();

    reward_type transition_impl(const Action &action);

    void print_impl(ostream &os) const;

    Tetromino generate_Tetronmino(const Tetromino_Type &type, const int &orientation = 0);
    double clear_lines();

    Placement get_placement(const Tetromino &tet, const std::pair<size_t, size_t> &position);
    size_t width_Tetronmino(const Tetromino &tet);
    size_t height_Tetronmino(const Tetromino &tet);

    Zeni::Random m_random_init;
    Zeni::Random m_random_selection;

    std::array<std::array<bool, 10>, 20> m_grid;
    Tetromino_Type m_current;
    Tetromino_Type m_next;

//    void generate_placements();
//    std::list<std::pair<std::pair<size_t, size_t>, int> > m_placements;
  };

}

#endif
