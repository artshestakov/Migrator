#include "MRUtils.h"
#include "MRConfig.h"
#include "MRLogger.h"
#include "MRTemplate.h"
#include "MRConstants.h"
#include "MRString.h"
//-----------------------------------------------------------------------------
IDatabase::DatabaseType MRUtils::ModuleNameToDatabaseType(std::string module_name)
{
    MRString::StringToLower(module_name);
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
    ISConfigSection* SectionMain = MRConfig::Instance().AddSection("Main");
    SectionMain->AddString("Module", std::string(), true, "#Module name. Use one of sections name below like a [SQLite] or [PostgreSQL]");

    ISConfigSection* SectionSQLite = MRConfig::Instance().AddSection("SQLite");
    SectionSQLite->AddString("PathDatabase", std::string(), true, "#Path to sqlite database file");
    SectionSQLite->AddString("PathLibrary", std::string(), true, "#Path to dynamic library (*.dll, *.so or *.a)");

    ISConfigSection* SectionPostgreSQL = MRConfig::Instance().AddSection("PostgreSQL");
    SectionPostgreSQL->AddString("Host", std::string(), true);
    SectionPostgreSQL->AddInt("Port", 0, 1, UINT16_MAX, true);
    SectionPostgreSQL->AddString("DatabaseName", std::string(), true);
    SectionPostgreSQL->AddString("User", std::string(), true);
    SectionPostgreSQL->AddString("Password", std::string(), true);
    SectionPostgreSQL->AddString("PathLibrary", std::string(), true, "#Path to dynamic library (*.dll, *.so or *.a)");
    SectionPostgreSQL->AddString("SystemDBName", "postgres", true, "#This is a service database name. It is created when you first install PostgreSQL.");

    if (!MRConfig::Instance().Initialize("Migrator"))
    {
        MR_LOG.Log("Cannot init config file: %s", MRConfig::Instance().GetErrorString().c_str());
        return false;
    }

    if (!MRConfig::Instance().IsValid())
    {
        MR_LOG.Log("Config file is not valid: %s", MRConfig::Instance().GetErrorString().c_str());
        return false;
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
            *error_string = MRString::GetLastErrorS();
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
std::string MRUtils::UuidGenerate(bool is_need_dash)
{
    std::string StringUID(MRConstants::UUID_STANDART_SIZE, '\0');
#ifdef WIN32
    GUID UID = { 0 };
    HRESULT Result = CoCreateGuid(&UID); //Генерируем идентификатор
    if (Result == S_OK) //Генерация прошла успешно
    {
        unsigned char* Char = { 0 };
        if (UuidToString(&UID, &Char) == RPC_S_OK) //Преобразовываем в строку
        {
            //Заполняем строку
            for (size_t i = 0; i < MRConstants::UUID_STANDART_SIZE; ++i)
            {
                StringUID[i] = Char[i];
            }
        }
    }
#else
    //Генерируем идентификатор
    uuid_t UUID = { 0 };
    uuid_generate(UUID);

    //Переводим его в строку
    char Char[MRConstants::UUID_STANDART_SIZE] = { 0 };
    uuid_unparse(UUID, Char);
    StringUID = Char;
#endif

    if (!is_need_dash)
    {
        MRString::StringRemoveAllChar(StringUID, '-');
    }

    return StringUID;
}
//-----------------------------------------------------------------------------
ISTimePoint MRUtils::GetTick()
{
    return std::chrono::steady_clock::now();
}
//-----------------------------------------------------------------------------
uint64_t MRUtils::GetTickDiff(const ISTimePoint& T)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(MRUtils::GetTick() - T).count();
}
//-----------------------------------------------------------------------------
std::string MRUtils::GetApplicationPath()
{
    std::string Path;
    char Buffer[MRConstants::MAX_PATH] = { 0 };
#ifdef WIN32
    if (GetModuleFileName(GetModuleHandle(NULL), Buffer, sizeof(Buffer)) > 0)
    {
        Path = Buffer;
    }
#else
    char Temp[20] = { 0 };
    sprintf(Temp, "/proc/%d/exe", CURRENT_PID());
    size_t Size = readlink(Temp, Buffer, 1024);
    Path = std::string(Buffer, Size);
#endif
    return Path;
}
//-----------------------------------------------------------------------------
std::string MRUtils::GetApplicationDir()
{
    std::string Path = MRUtils::GetApplicationPath();
    if (!Path.empty())
    {
        size_t Pos = Path.rfind(MRConstants::PATH_SEPARATOR);
        if (Pos != MRConstants::NPOS)
        {
            Path.erase(Pos);
        }
    }
    return Path;
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
