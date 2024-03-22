#pragma once
//-----------------------------------------------------------------------------
#include "IDatabase.h"
//#include <argparse.h>
#include "..\ISCore\argparse.hpp"
//-----------------------------------------------------------------------------
class MRUtils
{
public:
    static IDatabase::DatabaseType ModuleNameToDatabaseType(std::string module_name);
    static bool InitConfig();
    static std::optional<ISVectorString> ExtractPaths(const std::string& subcommand, argparse::ArgumentParser& args);
    static void PrintOldObjects(const IDatabase::ShowOldStruct& s);

private:
    static void PrintOldObjects(const ISVectorString& v, const std::string& title);
};
//-----------------------------------------------------------------------------
