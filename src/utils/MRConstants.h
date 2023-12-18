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

    inline constexpr size_t NPOS = std::string::npos;
    inline constexpr char   SIGNER_REGEXP[] = "<!--[A-Za-z0-9_.]*-->"; //Регулярное выражение для поиска подписи в Миграторе
    inline constexpr size_t LOG_HEADER_SIZE = 128; //Макисмальный размер заголовка
    inline constexpr size_t UUID_STANDART_SIZE = 36;
}
//-----------------------------------------------------------------------------
