#include "MRUtils.h"
#include "ISConfig.h"
#include "MRLogger.h"
#include "MRTemplate.h"
#include "MRConstants.h"
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

    MR_LOG.Log("Invalid module name '%s' in config file", module_name.c_str());
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
        MR_LOG.Log("Cannot init config file: %s", ISConfig::Instance().GetErrorString().c_str());
        return false;
    }

    //Проверяем валидность только когда конфигурационный файл уже был создан
    if (!ISConfig::Instance().GetIsFirstInit())
    {
        if (!ISConfig::Instance().IsValid())
        {
            MR_LOG.Log("Config file is not valid: %s", ISConfig::Instance().GetErrorString().c_str());
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
        MR_LOG.Log("Cannot read paths argument. %s", e.what());
    }
    return std::nullopt;
}
//-----------------------------------------------------------------------------
void MRUtils::PrintOldObjects(const IDatabase::ShowOldStruct& s)
{
    MR_LOG.LogWH("");
    MRUtils::PrintOldObjects(s.Tables, "Tables");
    MRUtils::PrintOldObjects(s.Fields, "Fields");
    MRUtils::PrintOldObjects(s.Views, "Views");
    MRUtils::PrintOldObjects(s.Indexes, "Indexes");
    MRUtils::PrintOldObjects(s.Foreigns, "Foreigns");
}
//-----------------------------------------------------------------------------
bool MRUtils::InstallEncoding(unsigned int code_page, std::string* error_string)
{
#if defined(WIN32) && defined(DEBUG)
    if (SetConsoleOutputCP(code_page) == FALSE) //Не удалось установить кодовую страницу
    {
        if (error_string)
        {
            *error_string = ISAlgorithm::GetLastErrorS();
        }
        return false;
    }
#else //Реализации под Linux не существует
    (void)code_page;
    (void)error_string;
#endif
    return true;
}
//-----------------------------------------------------------------------------
void MRUtils::PrintOldObjects(const ISVectorString& v, const std::string& title)
{
    MR_LOG.LogWH("");

    MR_LOG.LogWH("%s:", title.c_str());
    if (v.empty())
    {
        MR_LOG.LogWH("  No old ones");
    }
    else
    {
        std::for_each(v.begin(), v.end(), [](const std::string &object_name)
            {
                MR_LOG.LogWH("  %s", object_name.c_str());
            });
    }

    MR_LOG.LogWH("");
}
//-----------------------------------------------------------------------------
