#include "MRUtils.h"
#include "ISConfig.h"
#include "MRTemplate.h"
#include "MRConstants.h"
#include "ISLogger.h"
//-----------------------------------------------------------------------------
IDatabase::DatabaseType MRUtils::ModuleNameToDatabaseType(std::string module_name)
{
    ISAlgorithm::StringToLower(module_name);
    if (module_name == "sqlite")
    {
        return IDatabase::DatabaseType::SQLite;
    }
    else if (module_name == "postgresql")
    {
        return IDatabase::DatabaseType::PostgreSQL;
    }

    ISLOGGER_E(__CLASS__, "Invalid module name '%s' in config file", module_name.c_str());
    return IDatabase::DatabaseType::Unknown;
}
//-----------------------------------------------------------------------------
bool MRUtils::InitConfig()
{
    ISConfigSection* SectionMain = ISConfig::Instance().AddSection("Main");
    SectionMain->AddString("Module", std::string(), true, "#Module name. Use one of sections name below like a [SQLite] or [PostgreSQL]");

    ISConfigSection* SectionSQLite = ISConfig::Instance().AddSection("SQLite");
    SectionSQLite->AddString("PathDatabase", std::string(), true, "#Path to sqlite database file");
    SectionSQLite->AddString("PathLibrary", std::string(), true, "#Path to dynamic library (*.dll, *.so or *.a)");

    ISConfigSection* SectionPostgreSQL = ISConfig::Instance().AddSection("PostgreSQL");
    SectionPostgreSQL->AddString("Host", std::string(), true);
    SectionPostgreSQL->AddInt("Port", 0, 1, UINT16_MAX, true);
    SectionPostgreSQL->AddString("DatabaseName", std::string(), true);
    SectionPostgreSQL->AddString("User", std::string(), true);
    SectionPostgreSQL->AddString("Password", std::string(), true);
    SectionPostgreSQL->AddString("PathLibrary", std::string(), true, "#Path to dynamic library (*.dll, *.so or *.a)");
    SectionPostgreSQL->AddString("SystemDBName", "postgres", true, "#This is a service database name. It is created when you first install PostgreSQL.");

    if (!ISConfig::Instance().Initialize("Migrator"))
    {
        ISLOGGER_E(__CLASS__, "Cannot init config file: %s", ISConfig::Instance().GetErrorString().c_str());
        return false;
    }

    //Проверяем валидность только когда конфигурационный файл уже был создан
    if (!ISConfig::Instance().GetIsFirstInit())
    {
        if (!ISConfig::Instance().IsValid())
        {
            ISLOGGER_E(__CLASS__, "Config file is not valid: %s", ISConfig::Instance().GetErrorString().c_str());
            return false;
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
std::optional<ISVectorString> MRUtils::ExtractPaths(const std::string& subcommand, argparse::ArgumentParser& args)
{
    try
    {
        return args.at<argparse::ArgumentParser>(subcommand).get<ISVectorString>("paths");
    }
    catch (const std::logic_error& e)
    {
        ISLOGGER_E(__CLASS__, "Cannot read paths argument. %s", e.what());
    }
    return std::nullopt;
}
//-----------------------------------------------------------------------------
void MRUtils::PrintOldObjects(const IDatabase::ShowOldStruct& s)
{
    ISLOGGER_I(__CLASS__, "");
    MRUtils::PrintOldObjects(s.Tables, "Tables");
    MRUtils::PrintOldObjects(s.Fields, "Fields");
    MRUtils::PrintOldObjects(s.Views, "Views");
    MRUtils::PrintOldObjects(s.Indexes, "Indexes");
    MRUtils::PrintOldObjects(s.Foreigns, "Foreigns");
}
//-----------------------------------------------------------------------------
void MRUtils::PrintOldObjects(const ISVectorString& v, const std::string& title)
{
    ISLOGGER_I(__CLASS__, "");

    ISLOGGER_I(__CLASS__, "%s:", title.c_str());
    if (v.empty())
    {
        ISLOGGER_I(__CLASS__, "  No old ones");
    }
    else
    {
        std::for_each(v.begin(), v.end(), [](const std::string &object_name)
        {
            ISLOGGER_I(__CLASS__, "  %s", object_name.c_str());
        });
    }

    ISLOGGER_I(__CLASS__, "");
}
//-----------------------------------------------------------------------------
