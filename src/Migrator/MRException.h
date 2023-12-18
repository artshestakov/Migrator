#pragma once
//-----------------------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------------------
#ifdef WIN32
class MRException : public std::exception
#else
class MRException : public std::runtime_error
#endif
{
public:
    MRException(const char* const Message) :
#ifdef WIN32
        std::exception(Message)
#else
        std::runtime_error(Message)
#endif
    {

    }

    MRException(const std::string& Message) : MRException(Message.c_str())
    {

    }

    virtual ~MRException()
    {

    }

    virtual const char* what() const noexcept override
    {
#ifdef WIN32
        return std::exception::what();
#else
        return std::runtime_error::what();
#endif
    }
};
//-----------------------------------------------------------------------------
