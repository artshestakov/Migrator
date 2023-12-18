#include "MRMigrator.h"
#include "MRMetaData.h"
#include "MRDatabase.h"
#include "MRConfig.h"
#include "MRUtils.h"
#include "MRQuery.h"
#include "MRLogger.h"
#include "MRString.h"
#include "MRFileSystem.h"
//-----------------------------------------------------------------------------
int MRMigrator::Ping()
{
    std::string module_name = MRConfig::Instance().GetString("Main", "Module");
    IDatabase::DatabaseType db_type = MRUtils::ModuleNameToDatabaseType(module_name);
    if (db_type == IDatabase::DatabaseType::Unknown)
    {
        return EXIT_FAILURE;
    }

    MRDatabase d(db_type);
    if (!d.InitLibrary())
    {
        MR_LOG.Log("Cannot init library \"%s\": %s", d.GetLibraryPath().c_str(), d.GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    bool is_exists = false;
    if (!d.ExistsDB(is_exists))
    {
        return EXIT_FAILURE;
    }

    if (!is_exists)
    {
        MR_LOG.Log("Database is not exists");
        return EXIT_FAILURE;
    }

    auto tick = MRUtils::GetTick();
    if (!d.Connect())
    {
        MR_LOG.Log("Cannot connect to database: %d", d.GetErrorString().c_str());
        return EXIT_FAILURE;
    }
    MR_LOG.Log("Ping to the current database (%s) is %lld msec",
        module_name.c_str(), MRUtils::GetTickDiff(tick));

    d.Disconnect();
    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
int MRMigrator::Validate(const ISVectorString& paths)
{
    if (!MRMetaData::Instance().Init(paths))
    {
        MR_LOG.Log("Meta data is not init: %s", MRMetaData::Instance().GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    MR_LOG.Log("Meta data validate success");
    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
int MRMigrator::Update(const ISVectorString& paths)
{
    if (!MRMetaData::Instance().Init(paths))
    {
        MR_LOG.Log("Meta data is not init: %s", MRMetaData::Instance().GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    IDatabase::DatabaseType db_type = MRUtils::ModuleNameToDatabaseType(MRConfig::Instance().GetString("Main", "Module"));
    if (db_type == IDatabase::DatabaseType::Unknown)
    {
        return EXIT_FAILURE;
    }

    MRDatabase db(db_type);
    if (!InitLibrary(db))
    {
        return EXIT_FAILURE;
    }

    bool is_exists = false;
    if (!db.ExistsDB(is_exists))
    {
        MR_LOG.Log(db.GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    if (!is_exists && !db.Create())
    {
        MR_LOG.Log(db.GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    if (!db.Connect())
    {
        MR_LOG.Log(db.GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    //Выполняем запросы
    for (const TMetaExecute* meta_execute : MRMetaData::Instance().GetExecutes())
    {
        if (!db.Execute(meta_execute))
        {
            MR_LOG.Log(db.GetErrorString().c_str());
            return EXIT_FAILURE;
        }

        MR_LOG.Log("Execute \"%s\" runned successfully", meta_execute->Name.c_str());
    }

    //Обновляем таблицы
    for (const TMetaTable* meta_table : MRMetaData::Instance().GetTables())
    {
        if (!db.TableExists(meta_table, is_exists))
        {
            MR_LOG.Log(db.GetErrorString().c_str());
            return EXIT_FAILURE;
        }

        if (bool result = is_exists ? db.TableUpdate(meta_table) : db.TableCreate(meta_table); !result)
        {
            MR_LOG.Log("Cant't create table \"%s\"\n%s",
                meta_table->Name.c_str(), db.GetErrorString().c_str());
            return EXIT_FAILURE;
        }
        MR_LOG.Log("Table \"%s\" created/updated successfully", meta_table->Name.c_str());
    }

    //Обновляем индексы
    for (const TMetaIndex* meta_index : MRMetaData::Instance().GetIndexes())
    {
        if (!db.IndexExists(meta_index, is_exists))
        {
            MR_LOG.Log(db.GetErrorString().c_str());
            return EXIT_FAILURE;
        }

        if (bool result = is_exists ? db.IndexUpdate(meta_index) : db.IndexCreate(meta_index); !result)
        {
            MR_LOG.Log(db.GetErrorString().c_str());
            return EXIT_FAILURE;
        }
        MR_LOG.Log("Index \"%s\" created/updated successfully", meta_index->Name.c_str());
    }

    //Обновляем представления
    for (const TMetaView* meta_view : MRMetaData::Instance().GetViews())
    {
        if (!db.ViewExists(meta_view, is_exists))
        {
            MR_LOG.Log(db.GetErrorString().c_str());
            return EXIT_FAILURE;
        }

        if (bool result = is_exists ? db.ViewUpdate(meta_view) : db.ViewCreate(meta_view); !result)
        {
            MR_LOG.Log(db.GetErrorString().c_str());
            return EXIT_FAILURE;
        }
        MR_LOG.Log("View \"%s\" created/updated successfully", meta_view->Name.c_str());
    }

    //Обновляем внешние ключи
    for (const TMetaForeign* meta_foreign : MRMetaData::Instance().GetForeigns())
    {
        if (!db.ForeignExists(meta_foreign, is_exists))
        {
            MR_LOG.Log(db.GetErrorString().c_str());
            return EXIT_FAILURE;
        }

        if (bool result = is_exists ? db.ForeignUpdate(meta_foreign) : db.ForeignCreate(meta_foreign); !result)
        {
            MR_LOG.Log(db.GetErrorString().c_str());
            return EXIT_FAILURE;
        }
        MR_LOG.Log("Foreign key \"%s\" created/updated successfully", meta_foreign->Name.c_str());
    }

    (void)db.Disconnect();
    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
int MRMigrator::CheckModule()
{
    std::string module_name = MRConfig::Instance().GetString("Main", "Module");
    IDatabase::DatabaseType db_type = MRUtils::ModuleNameToDatabaseType(module_name);
    if (db_type == IDatabase::DatabaseType::Unknown)
    {
        return EXIT_FAILURE;
    }

    MRDatabase d(db_type);

    auto t = MRUtils::GetTick();
    if (!d.InitLibrary())
    {
        MR_LOG.Log("Cannot init library \"%s\": %s", d.GetLibraryPath().c_str(), d.GetErrorString().c_str());
        return EXIT_FAILURE;
    }
    MR_LOG.Log("Load library %s success for %lld msec",
        d.GetLibraryPath().c_str(), MRUtils::GetTickDiff(t));

    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
int MRMigrator::ShowConfig()
{
    const std::string& config_path = MRConfig::Instance().GetFilePath();
    std::ifstream file(config_path);
    if (!file.is_open())
    {
        MR_LOG.Log("Cannot open config file %s: %s",
            config_path.c_str(), MRString::GetLastErrorS().c_str());
        return EXIT_FAILURE;
    }

    MR_LOG.LogWH("%s\n", config_path.c_str());

    std::string line;
    while (std::getline(file, line))
    {
        MR_LOG.LogWH(line.c_str());
    }
    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
int MRMigrator::ShowOld(const ISVectorString& paths, const argparse::ArgumentParser& show_old_command)
{
    if (!MRMetaData::Instance().Init(paths))
    {
        MR_LOG.Log("Meta data is not init: %s", MRMetaData::Instance().GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    IDatabase::DatabaseType db_type = MRUtils::ModuleNameToDatabaseType(MRConfig::Instance().GetString("Main", "Module"));
    if (db_type == IDatabase::DatabaseType::Unknown)
    {
        return EXIT_FAILURE;
    }

    MRDatabase db(db_type);
    if (!InitLibrary(db))
    {
        return EXIT_FAILURE;
    }

    bool is_exists = false;
    if (!db.ExistsDB(is_exists))
    {
        MR_LOG.Log(db.GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    if (!is_exists)
    {
        MR_LOG.Log("Database is not exists");
        return EXIT_FAILURE;
    }

    if (!db.Connect())
    {
        MR_LOG.Log(db.GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    std::vector<IDatabase::ShowOldType> v;
    if (show_old_command.is_used("--tables"))
        v.emplace_back(IDatabase::ShowOldType::Tables);
    if (show_old_command.is_used("--fields"))
        v.emplace_back(IDatabase::ShowOldType::Fields);
    if (show_old_command.is_used("--views"))
        v.emplace_back(IDatabase::ShowOldType::Views);
    if (show_old_command.is_used("--indexes"))
        v.emplace_back(IDatabase::ShowOldType::Indexes);
    if (show_old_command.is_used("--foreigns"))
        v.emplace_back(IDatabase::ShowOldType::Foreigns);

    IDatabase::ShowOldStruct s;
    std::optional<std::string> error_string;
    db.GetOldObjects(v, s, error_string);
    (void)db.Disconnect();

    if (error_string.has_value())
    {
        MR_LOG.Log(error_string.value().c_str());
        return EXIT_FAILURE;
    }

    MRUtils::PrintOldObjects(s);
    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
int MRMigrator::InitConfig(const std::string& file_path)
{
    if (MRFileSystem::FileExist(file_path))
    {
        MR_LOG.Log("File \"%s\" already exists", file_path.c_str());
        return EXIT_FAILURE;
    }

    if (!MRConfig::Instance().ReInitialize(file_path))
    {
        MR_LOG.Log("Cannot init config file: %s", MRConfig::Instance().GetErrorString().c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
bool MRMigrator::InitLibrary(MRDatabase& db)
{
    if (!db.InitLibrary())
    {
        MR_LOG.Log("Cannot init library \"%s\": %s", db.GetLibraryPath().c_str(), db.GetErrorString().c_str());
        return false;
    }

    MR_LOG.Log("Init library \"%s\" success, version %s",
        db.GetLibraryPath().c_str(), db.GetVersion().c_str());
    return true;
}
//-----------------------------------------------------------------------------
