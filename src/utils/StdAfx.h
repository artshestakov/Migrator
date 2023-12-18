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
#include <execinfo.h>
#include <poll.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <bits/local_lim.h>
#include <uuid/uuid.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <regex.h>
#include <math.h>
#include <dlfcn.h>
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
