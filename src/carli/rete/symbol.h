#ifndef RETE_SYMBOL_H
#define RETE_SYMBOL_H

#include "../memory_pool.h"
#include <iostream>
#include <memory>
#include <string>

namespace Rete {

  class Symbol;
  class Symbol_Constant;
  class Symbol_Constant_Float;
  class Symbol_Constant_Int;
  class Symbol_Constant_String;
  class Symbol_Identifier;
  class Symbol_Variable;

  typedef std::shared_ptr<const Symbol> Symbol_Ptr_C;
  typedef std::shared_ptr<const Symbol_Constant> Symbol_Constant_Ptr_C;
  typedef std::shared_ptr<const Symbol_Constant_Float> Symbol_Constant_Float_Ptr_C;
  typedef std::shared_ptr<const Symbol_Constant_Int> Symbol_Constant_Int_Ptr_C;
  typedef std::shared_ptr<const Symbol_Constant_String> Symbol_Constant_String_Ptr_C;
  typedef std::shared_ptr<const Symbol_Identifier> Symbol_Identifier_Ptr_C;
  typedef std::shared_ptr<const Symbol_Variable> Symbol_Variable_Ptr_C;
  typedef std::shared_ptr<Symbol> Symbol_Ptr;
  typedef std::shared_ptr<Symbol_Constant> Symbol_Constant_Ptr;
  typedef std::shared_ptr<Symbol_Constant_Float> Symbol_Constant_Float_Ptr;
  typedef std::shared_ptr<Symbol_Constant_Int> Symbol_Constant_Int_Ptr;
  typedef std::shared_ptr<Symbol_Constant_String> Symbol_Constant_String_Ptr;
  typedef std::shared_ptr<Symbol_Identifier> Symbol_Identifier_Ptr;
  typedef std::shared_ptr<Symbol_Variable> Symbol_Variable_Ptr;

  class Symbol : public Zeni::Pool_Allocator<Symbol_Constant_String> {
    Symbol(const Symbol &);
    Symbol & operator=(const Symbol &);

  public:
    Symbol() {}
    virtual ~Symbol() {}

    virtual bool operator==(const Symbol &rhs) const = 0;
    virtual bool operator!=(const Symbol &rhs) const = 0;
    virtual bool operator<(const Symbol &rhs) const = 0;
    virtual bool operator<=(const Symbol &rhs) const = 0;
    virtual bool operator>(const Symbol &rhs) const = 0;
    virtual bool operator>=(const Symbol &rhs) const = 0;

    virtual size_t hash() const = 0;
    virtual std::ostream & print(std::ostream &os) const = 0;
  };

  inline std::ostream & operator<<(std::ostream &os, const Symbol &symbol) {
    return symbol.print(os);
  }

  class Symbol_Constant : public Symbol {
    Symbol_Constant(const Symbol_Constant &);
    Symbol_Constant & operator=(const Symbol_Constant &);

  public:
    Symbol_Constant() {}
  };

  class Symbol_Constant_Int : public Symbol_Constant {
    Symbol_Constant_Int(const Symbol_Constant_Int &);
    Symbol_Constant_Int & operator=(const Symbol_Constant_Int &);

  public:
    Symbol_Constant_Int(const int64_t &value_) : value(value_) {}

    inline bool operator==(const Symbol &rhs) const;
    inline bool operator!=(const Symbol &rhs) const;
    inline bool operator>(const Symbol &rhs) const;
    inline bool operator>=(const Symbol &rhs) const;
    inline bool operator<(const Symbol &rhs) const;
    inline bool operator<=(const Symbol &rhs) const;

    size_t hash() const {
      return std::hash<int64_t>()(value);
    }

    virtual std::ostream & print(std::ostream &os) const {
      return os << value;
    }

    int64_t value;
  };
  class Symbol_Constant_Float : public Symbol_Constant {
    Symbol_Constant_Float(const Symbol_Constant_Float &);
    Symbol_Constant_Float & operator=(const Symbol_Constant_Float &);

  public:
    Symbol_Constant_Float(const double &value_) : value(value_) {}

    inline bool operator==(const Symbol &rhs) const;
    inline bool operator!=(const Symbol &rhs) const;
    inline bool operator>(const Symbol &rhs) const;
    inline bool operator>=(const Symbol &rhs) const;
    inline bool operator<(const Symbol &rhs) const;
    inline bool operator<=(const Symbol &rhs) const;

    size_t hash() const {
      return std::hash<double>()(value);
    }

    virtual std::ostream & print(std::ostream &os) const {
      return os << value;
    }

    double value;
  };

  bool Symbol_Constant_Float::operator==(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value == rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value == rhs_ptr->value;
    return false;
  }

  bool Symbol_Constant_Float::operator!=(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value != rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value != rhs_ptr->value;
    return true;
  }

  bool Symbol_Constant_Float::operator>(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value > rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value > rhs_ptr->value;
    return false;
  }

  bool Symbol_Constant_Float::operator>=(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value >= rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value >= rhs_ptr->value;
    return false;
  }

  bool Symbol_Constant_Float::operator<(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value < rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value < rhs_ptr->value;
    return false;
  }

  bool Symbol_Constant_Float::operator<=(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value <= rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value <= rhs_ptr->value;
    return false;
  }

  bool Symbol_Constant_Int::operator==(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value == rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value == rhs_ptr->value;
    return false;
  }

  bool Symbol_Constant_Int::operator!=(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value != rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value != rhs_ptr->value;
    return true;
  }

  bool Symbol_Constant_Int::operator>(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value > rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value > rhs_ptr->value;
    return false;
  }

  bool Symbol_Constant_Int::operator>=(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value >= rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value >= rhs_ptr->value;
    return false;
  }

  bool Symbol_Constant_Int::operator<(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value < rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value < rhs_ptr->value;
    return false;
  }

  bool Symbol_Constant_Int::operator<=(const Symbol &rhs) const {
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Float *>(&rhs))
      return value <= rhs_ptr->value;
    if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_Int *>(&rhs))
      return value <= rhs_ptr->value;
    return false;
  }

  class Symbol_Constant_String : public Symbol_Constant {
    Symbol_Constant_String(const Symbol_Constant_String &);
    Symbol_Constant_String & operator=(const Symbol_Constant_String &);

  public:
    Symbol_Constant_String(const std::string &value_) : value(value_) {}

    bool operator==(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_String *>(&rhs))
        return value == rhs_ptr->value;
      return false;
    }

    bool operator!=(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_String *>(&rhs))
        return value != rhs_ptr->value;
      return true;
    }

    bool operator>(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_String *>(&rhs))
        return value > rhs_ptr->value;
      return false;
    }

    bool operator>=(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_String *>(&rhs))
        return value >= rhs_ptr->value;
      return false;
    }

    bool operator<(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_String *>(&rhs))
        return value < rhs_ptr->value;
      return false;
    }

    bool operator<=(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Constant_String *>(&rhs))
        return value <= rhs_ptr->value;
      return false;
    }

    size_t hash() const {
      return std::hash<std::string>()(value);
    }

    virtual std::ostream & print(std::ostream &os) const {
      return os << value;
    }

    std::string value;
  };

  class Symbol_Identifier : public Symbol {
    Symbol_Identifier(const Symbol_Identifier &);
    Symbol_Identifier & operator=(const Symbol_Identifier &);

  public:
    Symbol_Identifier(const std::string &value_) : value(value_) {}

    bool operator==(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Identifier *>(&rhs))
        return value == rhs_ptr->value;
      return false;
    }

    bool operator!=(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Identifier *>(&rhs))
        return value != rhs_ptr->value;
      return true;
    }

    bool operator>(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Identifier *>(&rhs))
        return value > rhs_ptr->value;
      return false;
    }

    bool operator>=(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Identifier *>(&rhs))
        return value >= rhs_ptr->value;
      return false;
    }

    bool operator<(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Identifier *>(&rhs))
        return value < rhs_ptr->value;
      return false;
    }

    bool operator<=(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Identifier *>(&rhs))
        return value <= rhs_ptr->value;
      return false;
    }

    size_t hash() const {
      return std::hash<std::string>()(value);
    }

    virtual std::ostream & print(std::ostream &os) const {
      return os << value;
    }

    std::string value;
  };

  class Symbol_Variable : public Symbol {
    Symbol_Variable(const Symbol_Variable &);
    Symbol_Variable & operator=(const Symbol_Variable &);

  public:
    enum Variable {First, Second, Third};

    Symbol_Variable(const Variable &value_) : value(value_) {}

    bool operator==(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Variable *>(&rhs))
        return value == rhs_ptr->value;
      return false;
    }

    bool operator!=(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Variable *>(&rhs))
        return value != rhs_ptr->value;
      return true;
    }

    bool operator>(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Variable *>(&rhs))
        return value > rhs_ptr->value;
      return false;
    }

    bool operator>=(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Variable *>(&rhs))
        return value >= rhs_ptr->value;
      return false;
    }

    bool operator<(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Variable *>(&rhs))
        return value < rhs_ptr->value;
      return false;
    }

    bool operator<=(const Symbol &rhs) const {
      if(auto rhs_ptr = dynamic_cast<const Symbol_Variable *>(&rhs))
        return value <= rhs_ptr->value;
      return false;
    }

    size_t hash() const {
      return std::hash<size_t>()(value);
    }

    virtual std::ostream & print(std::ostream &os) const {
      os.put('<');
      os << value;
      os.put('>');
      return os;
    }

    Variable value;
  };

}

namespace std {
  template <> struct hash<Rete::Symbol> {
    size_t operator()(const Rete::Symbol &symbol) const {
      return symbol.hash();
    }
  };
  template <> struct hash<Rete::Symbol_Variable::Variable> {
    size_t operator()(const Rete::Symbol_Variable::Variable &variable) const {
      return std::hash<size_t>()(variable);
    }
  };
}

#endif
