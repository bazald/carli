#include "tetris.h"

namespace Tetris {

  Environment::Environment() {
    init_impl();
  }

  void Environment::init_impl() {
    m_random_selection = Zeni::Random(2147483647 * m_random_init.rand());

    memset(&m_grid, 0, sizeof(m_grid));

    m_next = Tetromino_Type(m_random_selection.rand_lt(7));
    m_current = Tetromino_Type(m_random_selection.rand_lt(7));

//    generate_placements();
  }

  Environment::reward_type Environment::transition_impl(const Action &action) {
    const Place &place = debuggable_cast<const Place &>(action);

    const auto tet = generate_Tetronmino(m_current, place.orientation);
    for(size_t j = 0; j != 4; ++j) {
      for(size_t i = 0; i != 4; ++i) {
        if(tet[j][i])
          m_grid[j + place.position.second][i + place.position.first] = true;
      }
    }
    m_current = m_next;
    m_next = Tetromino_Type(m_random_selection.rand_lt(7));

    const double score = clear_lines();
//    generate_placements();
    return score;
  }

  void Environment::print_impl(ostream &os) const {
  }

  Environment::Tetromino Environment::generate_Tetronmino(const Tetromino_Type &type, const int &orientation) {
    Environment::Tetromino tet;
    memset(&tet, 0, sizeof(tet));

    switch(type) {
    case TET_LINE:
      if(orientation & 1)
        tet[0][0] = tet[0][1] = tet[0][2] = tet[0][3] = true;
      else
        tet[0][0] = tet[1][0] = tet[2][0] = tet[3][0] = true;
      break;
    case TET_SQUARE:
      tet[0][0] = tet[0][1] = tet[1][0] = tet[1][1] = true;
      break;
    case TET_T:
      if(orientation & 2) {
        if(orientation & 1)
          tet[0][1] = tet[1][1] = tet[1][2] = tet[1][0] = true;
        else
          tet[1][0] = tet[1][1] = tet[2][1] = tet[0][1] = true;
      }
      else {
        if(orientation & 1)
          tet[0][0] = tet[0][1] = tet[0][2] = tet[1][1] = true;
        else
          tet[0][0] = tet[1][0] = tet[2][0] = tet[1][1] = true;
      }
      break;
    case TET_S:
      if(orientation & 1)
        tet[0][0] = tet[1][0] = tet[1][1] = tet[2][1] = true;
      else
        tet[1][0] = tet[1][1] = tet[0][1] = tet[0][2] = true;
      break;
    case TET_Z:
      if(orientation & 1)
        tet[0][0] = tet[0][1] = tet[1][1] = tet[1][2] = true;
      else
        tet[2][0] = tet[1][0] = tet[1][1] = tet[0][1] = true;
      break;
    case TET_L:
      if(orientation & 2) {
        if(orientation & 1)
          tet[0][2] = tet[1][2] = tet[1][1] = tet[1][0] = true;
        else
          tet[0][0] = tet[0][1] = tet[1][1] = tet[2][1] = true;
      }
      else {
        if(orientation & 1)
          tet[0][0] = tet[0][1] = tet[0][2] = tet[1][0] = true;
        else
          tet[0][0] = tet[1][0] = tet[2][0] = tet[2][1] = true;
      }
      break;
    case TET__l:
      if(orientation & 2) {
        if(orientation & 1)
          tet[0][0] = tet[1][2] = tet[1][1] = tet[1][0] = true;
        else
          tet[2][0] = tet[0][1] = tet[1][1] = tet[2][1] = true;
      }
      else {
        if(orientation & 1)
          tet[0][0] = tet[0][1] = tet[0][2] = tet[1][2] = true;
        else
          tet[0][0] = tet[1][0] = tet[2][0] = tet[0][1] = true;
      }
      break;
    default:
      abort();
    }

    return tet;
  }

  double Environment::clear_lines() {
    double score = 0.0;

    for(size_t j = 19; j < 20; ) {
      bool cleared = true;
      for(int i = 0; i != 10; ++i) {
        if(!m_grid[j][i]) {
          cleared = false;
          break;
        }
      }

      if(!cleared) {
        --j;
        continue;
      }

      memmove(&m_grid[1], &m_grid[0], sizeof(m_grid[0]) * j);
      memset(&m_grid[0], 0, sizeof(m_grid[0]));
    }

    return score;
  }

  Environment::Placement Environment::get_placement(const Environment::Tetromino &tet, const std::pair<size_t, size_t> &position) {
    const size_t width = width_Tetronmino(tet);
    const size_t height = height_Tetronmino(tet);

    if(position.first + width > 10 || position.second + height > 20)
      return PLACE_ILLEGAL;

    bool grounded = position.second + height == 20;

    for(int j = 0; j != 4; ++j) {
      for(int i = 0; i != 4; ++i) {
        if(tet[j][i]) {
          if(m_grid[position.second + j][position.first + i])
            return PLACE_ILLEGAL;
          else if(!grounded && m_grid[position.second + j + 1][position.first + i])
            grounded = true;
        }
      }
    }

    return grounded ? PLACE_GROUNDED : PLACE_UNGROUNDED;
  }

  size_t Environment::width_Tetronmino(const Environment::Tetromino &tet) {
    for(size_t i = 0; i != 4; ++i) {
      for(size_t j = 0; ; ++j) {
        if(j == 4)
          return i;
        else {
          if(tet[j][i])
            break;
        }
      }
    }

    return 4;
  }

  size_t Environment::height_Tetronmino(const Environment::Tetromino &tet) {
    for(size_t j = 0; j != 4; ++j) {
      for(size_t i = 0; ; ++i) {
        if(i == 4)
          return j;
        else {
          if(tet[j][i])
            break;
        }
      }
    }

    return 4;
  }

//  void Environment::generate_placements() {
//    m_placements.clear();
//
//    for(int orientation = 0; orientation != 4; ++orientation) {
//      const auto tet = generate_Tetronmino(m_current, orientation);
//      const size_t width = width_Tetronmino(tet);
//
//      for(size_t i = 0, iend = 11 - width; i != iend; ++i) {
//        for(size_t j = 0; ; ++j) {
//          if(j == 20 || get_placement(tet, Point2i(i, j)) == PLACE_ILLEGAL) {
//            if(j)
//              m_placements.emplace_back(Point2i(i, j - 1), orientation);
//            break;
//          }
//        }
//      }
//    }
//
//    m_index = 0;
//  }

}
