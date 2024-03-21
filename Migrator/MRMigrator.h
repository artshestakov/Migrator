#pragma once
//-----------------------------------------------------------------------------
#include <..\..\LibraryExt\ISCore\argparse.hpp>
#include "MRDatabase.h"
//-----------------------------------------------------------------------------
class MRMigrator
{
public:
    static int Ping();
    static int Validate(const ISVectorString& paths);
    static int Update(const ISVectorString &paths);
    static int CheckModule();
    static int ShowConfig();
    static int ShowOld(const ISVectorString& paths, const argparse::ArgumentParser &show_old_command);
    static int InitConfig(const std::string& file_path);

private:
    static bool InitLibrary(MRDatabase &db);
};
//-----------------------------------------------------------------------------
