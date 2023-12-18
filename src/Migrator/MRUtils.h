#pragma once
//-----------------------------------------------------------------------------
#include "IDatabase.h"
#include "MRDateTime.h"
#include <argparse.h>
//-----------------------------------------------------------------------------
#ifdef WIN32
#define CRITICAL_SECTION_INIT(CRITICAL_SECTION) InitializeCriticalSection(CRITICAL_SECTION)
#define CRITICAL_SECTION_LOCK(CRITICAL_SECTION) EnterCriticalSection(CRITICAL_SECTION)
#define CRITICAL_SECTION_UNLOCK(CRITICAL_SECTION) LeaveCriticalSection(CRITICAL_SECTION)
#define CRITICAL_SECTION_DESTROY(CRITICAL_SECTION) DeleteCriticalSection(CRITICAL_SECTION)
#define CURRENT_PID GetCurrentProcessId
#else
#define CRITICAL_SECTION_INIT(CRITICAL_SECTION) pthread_mutex_init(CRITICAL_SECTION, NULL)
#define CRITICAL_SECTION_LOCK(CRITICAL_SECTION) pthread_mutex_lock(CRITICAL_SECTION)
#define CRITICAL_SECTION_UNLOCK(CRITICAL_SECTION) pthread_mutex_unlock(CRITICAL_SECTION)
#define CRITICAL_SECTION_DESTROY(CRITICAL_SECTION) pthread_mutex_destroy(CRITICAL_SECTION)
#define CURRENT_PID getpid
#endif
//-----------------------------------------------------------------------------
class MRUtils
{
public:
    static IDatabase::DatabaseType ModuleNameToDatabaseType(std::string module_name);
    static bool InitConfig();
    static std::optional<ISVectorString> ExtractPaths(const std::string& subcommand, argparse::ArgumentParser& args);
    static void PrintOldObjects(const IDatabase::ShowOldStruct& s);
    static bool InstallEncoding(unsigned int code_page, std::string* error_string);

    //! Генерация стандартного уникального идентификатора в формате b75ed238-411a-4f06-85ea-a2ecca37cfa8
    //! \param is_need_dash указывает, нужны ли тире в идентификаторе
    //! \return возвращает стандартный уникальный идентификатор
    static std::string UuidGenerate(bool is_need_dash = true);

    //! Получить временную метку
    //! \return возвращает временную метку
    static ISTimePoint GetTick();

    //! Получить разницу между T и текущим временем
    //! \param T временная метка
    //! \return возвращает разницу между двумя временными метками: текущая и T
    static uint64_t GetTickDiff(const ISTimePoint& T);

    //! Получить путь к исполняемому файлу
    //! \return возвращает путь к испролняемому файлу
    static std::string GetApplicationPath();

    //! Получить путь к папке приложения
    //! \return возвращает путь к папке приложения
    static std::string GetApplicationDir();
private:
    static void PrintOldObjects(const ISVectorString& v, const std::string& title);
};
//-----------------------------------------------------------------------------
