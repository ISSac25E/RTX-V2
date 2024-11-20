//VarPar
//.h
#ifndef VarPar_h
#define VarPar_h

#include "Arduino.h"

class Par_bool
{
public:
    Par_bool(bool Var = false)
    {
        _Var = Var;
        _PreVar = Var;
    }
    Par_bool &operator=(bool Var)
    {
        _Var = Var;
        return *this;
    }
    operator bool()
    {
        return _Var;
    }
    bool change()
    {
        if (_Var != _PreVar)
        {
            _PreVar = _Var;
            return true;
        }
        return false;
    }

private:
    bool _Var;
    bool _PreVar;
};

class Par_uint8_t
{
public:
    Par_uint8_t(uint8_t Var = 0)
    {
        _Var = Var;
        _PreVar = Var;
    }
    Par_uint8_t &operator=(uint8_t Var)
    {
        _Var = Var;
        return *this;
    }
    operator uint8_t()
    {
        return _Var;
    }
    bool change()
    {
        if (_Var != _PreVar)
        {
            _PreVar = _Var;
            return true;
        }
        return false;
    }

private:
    uint8_t _Var;
    uint8_t _PreVar;
};

class Par_uint16_t
{
public:
    Par_uint16_t(uint16_t Var = 0)
    {
        _Var = Var;
        _PreVar = Var;
    }
    Par_uint16_t &operator=(uint16_t Var)
    {
        _Var = Var;
        return *this;
    }
    operator uint16_t()
    {
        return _Var;
    }
    bool change()
    {
        if (_Var != _PreVar)
        {
            _PreVar = _Var;
            return true;
        }
        return false;
    }

private:
    uint16_t _Var;
    uint16_t _PreVar;
};

class Par_uint32_t
{
public:
    Par_uint32_t(uint32_t Var = 0)
    {
        _Var = Var;
        _PreVar = Var;
    }
    Par_uint32_t &operator=(uint32_t Var)
    {
        _Var = Var;
        return *this;
    }
    operator uint32_t()
    {
        return _Var;
    }
    bool change()
    {
        if (_Var != _PreVar)
        {
            _PreVar = _Var;
            return true;
        }
        return false;
    }

private:
    uint32_t _Var;
    uint32_t _PreVar;
};

class Par_int8_t
{
public:
    Par_int8_t(int8_t Var = 0)
    {
        _Var = Var;
        _PreVar = Var;
    }
    Par_int8_t &operator=(int8_t Var)
    {
        _Var = Var;
        return *this;
    }
    operator int8_t()
    {
        return _Var;
    }
    bool change()
    {
        if (_Var != _PreVar)
        {
            _PreVar = _Var;
            return true;
        }
        return false;
    }

private:
    int8_t _Var;
    int8_t _PreVar;
};

class Par_int16_t
{
public:
    Par_int16_t(int16_t Var = 0)
    {
        _Var = Var;
        _PreVar = Var;
    }
    Par_int16_t &operator=(int16_t Var)
    {
        _Var = Var;
        return *this;
    }
    operator int16_t()
    {
        return _Var;
    }
    bool change()
    {
        if (_Var != _PreVar)
        {
            _PreVar = _Var;
            return true;
        }
        return false;
    }

private:
    int16_t _Var;
    int16_t _PreVar;
};

class Par_int32_t
{
public:
    Par_int32_t(int32_t Var = 0)
    {
        _Var = Var;
        _PreVar = Var;
    }
    Par_int32_t &operator=(int32_t Var)
    {
        _Var = Var;
        return *this;
    }
    operator int32_t()
    {
        return _Var;
    }
    bool change()
    {
        if (_Var != _PreVar)
        {
            _PreVar = _Var;
            return true;
        }
        return false;
    }

private:
    int32_t _Var;
    int32_t _PreVar;
};

//.cpp
//#include "VarPar.h"
//#include "Arduino.h"

#endif
