//VarPar
//.h
#ifndef VarPar_h
#define VarPar_h

#include "Arduino.h"

class Par_bool
{
public:
    Par_bool(bool var = false)
    {
        _var = var;
        _prevar = var;
    }
    Par_bool &operator=(bool var)
    {
        _var = var;
        return *this;
    }
    operator bool()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    bool _var;
    bool _prevar;
};

class Par_uint8_t
{
public:
    Par_uint8_t(uint8_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_uint8_t &operator=(uint8_t var)
    {
        _var = var;
        return *this;
    }
    operator uint8_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    uint8_t _var;
    uint8_t _prevar;
};

class Par_uint16_t
{
public:
    Par_uint16_t(uint16_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_uint16_t &operator=(uint16_t var)
    {
        _var = var;
        return *this;
    }
    operator uint16_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    uint16_t _var;
    uint16_t _prevar;
};

class Par_uint32_t
{
public:
    Par_uint32_t(uint32_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_uint32_t &operator=(uint32_t var)
    {
        _var = var;
        return *this;
    }
    operator uint32_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    uint32_t _var;
    uint32_t _prevar;
};

class Par_int8_t
{
public:
    Par_int8_t(int8_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_int8_t &operator=(int8_t var)
    {
        _var = var;
        return *this;
    }
    operator int8_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    int8_t _var;
    int8_t _prevar;
};

class Par_int16_t
{
public:
    Par_int16_t(int16_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_int16_t &operator=(int16_t var)
    {
        _var = var;
        return *this;
    }
    operator int16_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    int16_t _var;
    int16_t _prevar;
};

class Par_int32_t
{
public:
    Par_int32_t(int32_t var = 0)
    {
        _var = var;
        _prevar = var;
    }
    Par_int32_t &operator=(int32_t var)
    {
        _var = var;
        return *this;
    }
    operator int32_t()
    {
        return _var;
    }
    bool change()
    {
        if (_var != _prevar)
        {
            _prevar = _var;
            return true;
        }
        return false;
    }

private:
    int32_t _var;
    int32_t _prevar;
};

//.cpp
//#include "VarPar.h"
//#include "Arduino.h"

#endif
