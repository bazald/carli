#ifndef RETE_SYMBOL_H
#define RETE_SYMBOL_H

#include "../utility/memory_pool.h"

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

  class Symbol :
#if __WORDSIZE == 64
                 public Zeni::Pool_Allocator<Symbol_Constant_String>
#else
                 public Zeni::Pool_Allocator<Symbol_Constant_Float>
#endif
  {
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

    virtual bool operator==(const Symbol_Constant_Float &) const {return false;}
    virtual bool operator!=(const Symbol_Constant_Float &) const {return true;}
    virtual bool operator<(const Symbol_Constant_Float &) const {return false;}
    virtual bool operator<=(const Symbol_Constant_Float &) const {return false;}
    virtual bool operator>(const Symbol_Constant_Float &) const {return false;}
    virtual bool operator>=(const Symbol_Constant_Float &) const {return false;}

    virtual bool operator==(const Symbol_Constant_Int &) const {return false;}
    virtual bool operator!=(const Symbol_Constant_Int &) const {return true;}
    virtual bool operator<(const Symbol_Constant_Int &) const {return false;}
    virtual bool operator<=(const Symbol_Constant_Int &) const {return false;}
    virtual bool operator>(const Symbol_Constant_Int &) const {return false;}
    virtual bool operator>=(const Symbol_Constant_Int &) const {return false;}

    virtual bool operator==(const Symbol_Constant_String &) const {return false;}
    virtual bool operator!=(const Symbol_Constant_String &) const {return true;}
    virtual bool operator<(const Symbol_Constant_String &) const {return false;}
    virtual bool operator<=(const Symbol_Constant_String &) const {return false;}
    virtual bool operator>(const Symbol_Constant_String &) const {return false;}
    virtual bool operator>=(const Symbol_Constant_String &) const {return false;}

    virtual bool operator==(const Symbol_Identifier &) const {return false;}
    virtual bool operator!=(const Symbol_Identifier &) const {return true;}
    virtual bool operator<(const Symbol_Identifier &) const {return false;}
    virtual bool operator<=(const Symbol_Identifier &) const {return false;}
    virtual bool operator>(const Symbol_Identifier &) const {return false;}
    virtual bool operator>=(const Symbol_Identifier &) const {return false;}

    virtual bool operator==(const Symbol_Variable &) const {return false;}
    virtual bool operator!=(const Symbol_Variable &) const {return true;}
    virtual bool operator<(const Symbol_Variable &) const {return false;}
    virtual bool operator<=(const Symbol_Variable &) const {return false;}
    virtual bool operator>(const Symbol_Variable &) const {return false;}
    virtual bool operator>=(const Symbol_Variable &) const {return false;}

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

  class Symbol_Constant_Float : public Symbol_Constant {
    Symbol_Constant_Float(const Symbol_Constant_Float &);
    Symbol_Constant_Float & operator=(const Symbol_Constant_Float &);

  public:
    Symbol_Constant_Float(const double &value_) : value(value_) {}

    bool operator==(const Symbol &rhs) const {return rhs == *this;}
    bool operator!=(const Symbol &rhs) const {return rhs != *this;}
    bool operator>(const Symbol &rhs) const {return rhs < *this;}
    bool operator>=(const Symbol &rhs) const {return rhs <= *this;}
    bool operator<(const Symbol &rhs) const {return rhs > *this;}
    bool operator<=(const Symbol &rhs) const {return rhs >= *this;}

    bool operator==(const Symbol_Constant_Float &rhs) const {return value == rhs.value;}
    bool operator!=(const Symbol_Constant_Float &rhs) const {return value != rhs.value;}
    bool operator<(const Symbol_Constant_Float &rhs) const {return value < rhs.value;}
    bool operator<=(const Symbol_Constant_Float &rhs) const {return value <= rhs.value;}
    bool operator>(const Symbol_Constant_Float &rhs) const {return value > rhs.value;}
    bool operator>=(const Symbol_Constant_Float &rhs) const {return value >= rhs.value;}

    inline bool operator==(const Symbol_Constant_Int &rhs) const;
    inline bool operator!=(const Symbol_Constant_Int &rhs) const;
    inline bool operator<(const Symbol_Constant_Int &rhs) const;
    inline bool operator<=(const Symbol_Constant_Int &rhs) const;
    inline bool operator>(const Symbol_Constant_Int &rhs) const;
    inline bool operator>=(const Symbol_Constant_Int &rhs) const;

    size_t hash() const {
      return std::hash<double>()(value);
    }

    virtual std::ostream & print(std::ostream &os) const {
      return os << value;
    }

    double value;
  };

  class Symbol_Constant_Int : public Symbol_Constant {
    Symbol_Constant_Int(const Symbol_Constant_Int &);
    Symbol_Constant_Int & operator=(const Symbol_Constant_Int &);

  public:
    Symbol_Constant_Int(const int64_t &value_) : value(value_) {}

    bool operator==(const Symbol &rhs) const {return rhs == *this;}
    bool operator!=(const Symbol &rhs) const {return rhs != *this;}
    bool operator>(const Symbol &rhs) const {return rhs < *this;}
    bool operator>=(const Symbol &rhs) const {return rhs <= *this;}
    bool operator<(const Symbol &rhs) const {return rhs > *this;}
    bool operator<=(const Symbol &rhs) const {return rhs >= *this;}

    bool operator==(const Symbol_Constant_Float &rhs) const {return value == rhs.value;}
    bool operator!=(const Symbol_Constant_Float &rhs) const {return value != rhs.value;}
    bool operator<(const Symbol_Constant_Float &rhs) const {return value < rhs.value;}
    bool operator<=(const Symbol_Constant_Float &rhs) const {return value <= rhs.value;}
    bool operator>(const Symbol_Constant_Float &rhs) const {return value > rhs.value;}
    bool operator>=(const Symbol_Constant_Float &rhs) const {return value >= rhs.value;}

    bool operator==(const Symbol_Constant_Int &rhs) const {return value == rhs.value;}
    bool operator!=(const Symbol_Constant_Int &rhs) const {return value != rhs.value;}
    bool operator<(const Symbol_Constant_Int &rhs) const {return value < rhs.value;}
    bool operator<=(const Symbol_Constant_Int &rhs) const {return value <= rhs.value;}
    bool operator>(const Symbol_Constant_Int &rhs) const {return value > rhs.value;}
    bool operator>=(const Symbol_Constant_Int &rhs) const {return value >= rhs.value;}

    size_t hash() const {
      return std::hash<int64_t>()(value);
    }

    virtual std::ostream & print(std::ostream &os) const {
      return os << value;
    }

    int64_t value;
  };

  bool Symbol_Constant_Float::operator==(const Symbol_Constant_Int &rhs) const {return value == rhs.value;}
  bool Symbol_Constant_Float::operator!=(const Symbol_Constant_Int &rhs) const {return value != rhs.value;}
  bool Symbol_Constant_Float::operator<(const Symbol_Constant_Int &rhs) const {return value < rhs.value;}
  bool Symbol_Constant_Float::operator<=(const Symbol_Constant_Int &rhs) const {return value <= rhs.value;}
  bool Symbol_Constant_Float::operator>(const Symbol_Constant_Int &rhs) const {return value > rhs.value;}
  bool Symbol_Constant_Float::operator>=(const Symbol_Constant_Int &rhs) const {return value >= rhs.value;}

  class Symbol_Constant_String : public Symbol_Constant {
    Symbol_Constant_String(const Symbol_Constant_String &);
    Symbol_Constant_String & operator=(const Symbol_Constant_String &);

  public:
    Symbol_Constant_String(const std::string &value_) : value(value_) {}

    bool operator==(const Symbol &rhs) const {return rhs == *this;}
    bool operator!=(const Symbol &rhs) const {return rhs != *this;}
    bool operator>(const Symbol &rhs) const {return rhs < *this;}
    bool operator>=(const Symbol &rhs) const {return rhs <= *this;}
    bool operator<(const Symbol &rhs) const {return rhs > *this;}
    bool operator<=(const Symbol &rhs) const {return rhs >= *this;}

    bool operator==(const Symbol_Constant_String &rhs) const {return value == rhs.value;}
    bool operator!=(const Symbol_Constant_String &rhs) const {return value != rhs.value;}
    bool operator<(const Symbol_Constant_String &rhs) const {return value < rhs.value;}
    bool operator<=(const Symbol_Constant_String &rhs) const {return value <= rhs.value;}
    bool operator>(const Symbol_Constant_String &rhs) const {return value > rhs.value;}
    bool operator>=(const Symbol_Constant_String &rhs) const {return value >= rhs.value;}

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

    bool operator==(const Symbol &rhs) const {return rhs == *this;}
    bool operator!=(const Symbol &rhs) const {return rhs != *this;}
    bool operator>(const Symbol &rhs) const {return rhs < *this;}
    bool operator>=(const Symbol &rhs) const {return rhs <= *this;}
    bool operator<(const Symbol &rhs) const {return rhs > *this;}
    bool operator<=(const Symbol &rhs) const {return rhs >= *this;}

    bool operator==(const Symbol_Identifier &rhs) const {return value == rhs.value;}
    bool operator!=(const Symbol_Identifier &rhs) const {return value != rhs.value;}
    bool operator<(const Symbol_Identifier &rhs) const {return value < rhs.value;}
    bool operator<=(const Symbol_Identifier &rhs) const {return value <= rhs.value;}
    bool operator>(const Symbol_Identifier &rhs) const {return value > rhs.value;}
    bool operator>=(const Symbol_Identifier &rhs) const {return value >= rhs.value;}

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

    bool operator==(const Symbol &rhs) const {return rhs == *this;}
    bool operator!=(const Symbol &rhs) const {return rhs != *this;}
    bool operator>(const Symbol &rhs) const {return rhs < *this;}
    bool operator>=(const Symbol &rhs) const {return rhs <= *this;}
    bool operator<(const Symbol &rhs) const {return rhs > *this;}
    bool operator<=(const Symbol &rhs) const {return rhs >= *this;}

    bool operator==(const Symbol_Variable &rhs) const {return value == rhs.value;}
    bool operator!=(const Symbol_Variable &rhs) const {return value != rhs.value;}
    bool operator<(const Symbol_Variable &rhs) const {return value < rhs.value;}
    bool operator<=(const Symbol_Variable &rhs) const {return value <= rhs.value;}
    bool operator>(const Symbol_Variable &rhs) const {return value > rhs.value;}
    bool operator>=(const Symbol_Variable &rhs) const {return value >= rhs.value;}

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

  inline void __symbol_size_check() {
    typedef typename Symbol::pool_allocator_type pool_allocator_type;
    static_assert(sizeof(pool_allocator_type) >= sizeof(Symbol_Constant_Float), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Symbol_Constant_Int), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Symbol_Constant_String), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Symbol_Identifier), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Symbol_Variable), "Pool size suboptimal.");
  }

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
