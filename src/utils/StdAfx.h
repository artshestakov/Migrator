#pragma once
//-----------------------------------------------------------------------------
#ifdef WIN32
#pragma warning(disable: 4702) //Недостижимый код
#endif
//PLATFORM---------------------------------------------------------------------
#ifdef WIN32
#include <Shlwapi.h>
#else
#include <unistd.h>
#include <string.h>
#include <uuid/uuid.h>
#include <openssl/sha.h>
#include <dlfcn.h>
#include <stdarg.h>
#endif
//C++--------------------------------------------------------------------------
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <exception>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <variant>
#include <optional>
//-----------------------------------------------------------------------------
