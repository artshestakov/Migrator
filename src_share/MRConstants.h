#pragma once
//-----------------------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------------------
namespace MRConstants
{
#ifdef WIN32
    inline constexpr char PATH_SEPARATOR = '\\';
#else
    inline constexpr char PATH_SEPARATOR = '/';
#endif

    inline constexpr char   SIGNER_REGEXP[] = "<!--[A-Za-z0-9_.]*-->"; //Регулярное выражение для поиска подписи в Миграторе
}
//-----------------------------------------------------------------------------
