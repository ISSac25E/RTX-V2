// VarPar
//.h
/*
  v1.0.2 changes:
    - removed overloads to prevent unecessary bugs
      replaced with 'var'
*/
#ifndef VarPar_h
#define VarPar_h

#include "Arduino.h"

class Par_bool
{
public:
  Par_bool(bool _var = false)
  {
    var = _var;
    _prevar = var;
  }
  // Par_bool &operator=(bool var)
  // {
  //     var = var;
  //     return *this;
  // }
  // operator bool()
  // {
  //     return var;
  // }
  bool change()
  {
    if (var != _prevar)
    {
      _prevar = var;
      return true;
    }
    return false;
  }

  bool var;

private:
  bool _prevar;
};

class Par_uint8_t
{
public:
  Par_uint8_t(uint8_t _var = 0)
  {
    var = _var;
    _prevar = var;
  }
  // Par_uint8_t &operator=(uint8_t var)
  // {
  //   var = var;
  //   return *this;
  // }
  // operator uint8_t()
  // {
  //   return var;
  // }
  bool change()
  {
    if (var != _prevar)
    {
      _prevar = var;
      return true;
    }
    return false;
  }

  uint8_t var;

private:
  uint8_t _prevar;
};

class Par_uint16_t
{
public:
  Par_uint16_t(uint16_t _var = 0)
  {
    var = _var;
    _prevar = var;
  }
  // Par_uint16_t &operator=(uint16_t var)
  // {
  //   var = var;
  //   return *this;
  // }
  // operator uint16_t()
  // {
  //   return var;
  // }
  bool change()
  {
    if (var != _prevar)
    {
      _prevar = var;
      return true;
    }
    return false;
  }

  uint16_t var;

private:
  uint16_t _prevar;
};

class Par_uint32_t
{
public:
  Par_uint32_t(uint32_t _var = 0)
  {
    var = _var;
    _prevar = var;
  }
  // Par_uint32_t &operator=(uint32_t var)
  // {
  //   var = var;
  //   return *this;
  // }
  // operator uint32_t()
  // {
  //   return var;
  // }
  bool change()
  {
    if (var != _prevar)
    {
      _prevar = var;
      return true;
    }
    return false;
  }

  uint32_t var;

private:
  uint32_t _prevar;
};

class Par_int8_t
{
public:
  Par_int8_t(int8_t _var = 0)
  {
    var = _var;
    _prevar = var;
  }
  // Par_int8_t &operator=(int8_t var)
  // {
  //   var = var;
  //   return *this;
  // }
  // operator int8_t()
  // {
  //   return var;
  // }
  bool change()
  {
    if (var != _prevar)
    {
      _prevar = var;
      return true;
    }
    return false;
  }

  int8_t var;

private:
  int8_t _prevar;
};

class Par_int16_t
{
public:
  Par_int16_t(int16_t _var = 0)
  {
    var = _var;
    _prevar = var;
  }
  // Par_int16_t &operator=(int16_t var)
  // {
  //   var = var;
  //   return *this;
  // }
  // operator int16_t()
  // {
  //   return var;
  // }
  bool change()
  {
    if (var != _prevar)
    {
      _prevar = var;
      return true;
    }
    return false;
  }

  int16_t var;

private:
  int16_t _prevar;
};

class Par_int32_t
{
public:
  Par_int32_t(int32_t _var = 0)
  {
    var = _var;
    _prevar = var;
  }
  // Par_int32_t &operator=(int32_t var)
  // {
  //   var = var;
  //   return *this;
  // }
  // operator int32_t()
  // {
  //   return var;
  // }
  bool change()
  {
    if (var != _prevar)
    {
      _prevar = var;
      return true;
    }
    return false;
  }

  int32_t var;

private:
  int32_t _prevar;
};

//.cpp
// #include "VarPar.h"
// #include "Arduino.h"

#endif
