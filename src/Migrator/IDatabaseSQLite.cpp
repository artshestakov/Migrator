#include "IDatabaseSQLite.h"
#include "MRConfig.h"
#include "MRQuery.h"
#include "MRSQLProcessor.h"
#include "MRMetaData.h"
#include "MRUtils.h"
#include "MRTemplate.h"
#include "MRConstants.h"
#include "MRFileSystem.h"
#include "MRString.h"
//-----------------------------------------------------------------------------
IDatabaseSQLite::IDatabaseSQLite(DatabaseType database_type)
    : IDatabase(database_type),
    m_Path(MRConfig::Instance().GetString("SQLite", "PathDatabase")),
    m_DB(nullptr)
{

}
//-----------------------------------------------------------------------------
IDatabaseSQLite::~IDatabaseSQLite()
{

}
//-----------------------------------------------------------------------------
std::string IDatabaseSQLite::GetVersion() const
{
    return MRDefSingleton::Get().SQLite->sqlite3_libversion();
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::ExistsDB(bool& is_exists)
{
    is_exists = MRFileSystem::FileExist(m_Path);
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::Create()
{
    //Создаём директорию, если её ещё нет
    size_t pos = m_Path.rfind(MRConstants::PATH_SEPARATOR);
    std::string dir_path = m_Path.substr(0, pos);

    if (!MRFileSystem::DirExist(dir_path))
    {
        std::string error_string;
        if (!MRFileSystem::DirCreate(dir_path, &error_string))
        {
            m_ErrorString = MRString::StringF("Cannot create directory (%s): %s", dir_path.c_str(), error_string.c_str());
            return false;
        }
    }

    if (!Connect())
    {
        return false;
    }

    if (!Disconnect())
    {
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::Connect()
{
    if (!m_DB)
    {
        if (int r = MRDefSingleton::Get().SQLite->sqlite3_open(m_Path.c_str(), &m_DB); r != SQLITE_OK)
        {
            m_ErrorString = MRString::StringF("Cannot open database '%s': %s", m_Path.c_str(), MRDefSingleton::Get().SQLite->sqlite3_errstr(r));
            return false;
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::Disconnect()
{
    if (int r = MRDefSingleton::Get().SQLite->sqlite3_close_v2(m_DB); r != SQLITE_OK)
    {
        m_ErrorString = MRString::StringF("Cannot close connection to the database '%s': %s", m_Path.c_str(), MRDefSingleton::Get().SQLite->sqlite3_errstr(r));
        return false;
    }
    m_DB = nullptr;
    return true;
}
//-----------------------------------------------------------------------------
MRDefSQLite::sqlite3* IDatabaseSQLite::GetPointer() const
{
    return m_DB;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::Execute(const TMetaExecute* meta_execute)
{
    (void)meta_execute;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::TableExists(const TMetaTable* meta_table, bool& is_exists)
{
    is_exists = false;

    MRQuery qSelect(this, "SELECT COUNT(*) FROM sqlite_master WHERE lower(type) = 'table' AND lower(name) = lower(?)");
    qSelect.BindString(meta_table->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = MRString::StringF("Cannot check exist table '%s': %s",
            meta_table->Name.c_str(), qSelect.GetErrorString().c_str());
        return false;
    }

    is_exists = qSelect.ReadColumn_Int(0) == 1;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::TableCreate(const TMetaTable* meta_table)
{
    MRQuery qCreate(this, MRSQLProcessor::SQLite::TableCreate(meta_table));
    if (!qCreate.Execute())
    {
        m_ErrorString = qCreate.GetErrorString();
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::TableUpdate(const TMetaTable* meta_table)
{
    bool is_exist = false;

    //Проверяем, нужно ли создавать поля
    for (const TMetaField* meta_field : meta_table->Fields)
    {
        if (!FieldExists(meta_field, is_exist))
        {
            return false;
        }

        //Создаём, если поле не существует
        if (!is_exist && !FieldCreate(meta_field))
        {
            return false;
        }
    }

    //Проверяем, нужно ли менять атрибуты: тип данных, размер и т.д.
    MRQuery qSelect(this, MRString::StringF("PRAGMA table_xinfo('%s')", meta_table->Name.c_str()));
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }

    ISVectorString primary_fields;
    bool is_need_alter = false; //Флаг необходимости обновления одного из полей таблицы
    while (qSelect.Next())
    {
        const char* field_name = qSelect.ReadColumn(1);
        auto meta_field = meta_table->GetField(field_name);
        if (!meta_field)
        {
            //Не нашли поле в мета-данных, значит оно будет удалено далее при пересоздании таблицы
            is_need_alter = true;
            break;
        }

        std::string field_type = MRString::StringToLowerGet(qSelect.ReadColumn(2));
        bool field_not_null = qSelect.ReadColumn_Int(3);
        std::string field_default = qSelect.IsNull(4) ? std::string() : qSelect.ReadColumn(4);
        bool field_primary = qSelect.ReadColumn_Int(5) != 0;

        if (field_primary)
        {
            primary_fields.emplace_back(field_name);
        }

        if (field_type != meta_field->Type ||
            field_default != meta_field->DefaultValue ||
            field_not_null != meta_field->NotNull)
        {
            is_need_alter = true;
            break;
        }
    }
    qSelect.ClearStatement();

    std::sort(primary_fields.begin(), primary_fields.end());
    is_need_alter = primary_fields != meta_table->PrimaryFields;

    if (is_need_alter)
    {
        std::string fields_for_select;
        for (const TMetaField* meta_field : meta_table->Fields)
        {
            fields_for_select += meta_field->Name + ", ";
        }
        MRString::StringChop(fields_for_select, 2);

        ISVectorString v =
        {
            "PRAGMA foreign_keys=off",
            "BEGIN TRANSACTION",
            MRString::StringF("ALTER TABLE %s RENAME TO _temp_table", meta_table->Name.c_str()),
            MRSQLProcessor::SQLite::TableCreate(meta_table),
            MRString::StringF("INSERT INTO %s(%s) SELECT %s FROM _temp_table",
                meta_table->Name.c_str(), fields_for_select.c_str(), fields_for_select.c_str()),
            "DROP TABLE _temp_table;",
            "COMMIT",
            "PRAGMA foreign_keys=on"
        };

        for (const std::string& sql : v)
        {
            MRQuery q(this, sql);
            if (!q.Execute())
            {
                std::string prev_error = q.GetErrorString();
                if (!q.Execute("ROLLBACK"))
                {
                    m_ErrorString = MRString::StringF("Cannot rollback transaction: %s\nPrevious error: %s",
                        q.GetErrorString().c_str(), prev_error.c_str());
                }
                else
                {
                    m_ErrorString = prev_error;
                }
                return false;
            }
        }

        //Создаём необходимые индексы
        for (const TMetaIndex* meta_index : meta_table->Indexes)
        {
            if (!IndexCreate(meta_index))
            {
                return false;
            }
        }
    }

    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::FieldExists(const TMetaField* meta_field, bool& is_exists)
{
    is_exists = false;

    MRQuery qSelect(this, "SELECT name, type FROM pragma_table_info(?);");
    qSelect.BindString(meta_field->MetaTable->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }

    while (qSelect.Next())
    {
        is_exists = strcmp(qSelect.ReadColumn(0), meta_field->Name.c_str()) == 0;
        if (is_exists)
        {
            return true;
        }
    }

    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::FieldCreate(const TMetaField* meta_field)
{
    MRQuery qCreate(this, MRSQLProcessor::SQLite::FieldCreate(meta_field));
    if (!qCreate.Execute())
    {
        m_ErrorString = qCreate.GetErrorString();
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::IndexExists(const TMetaIndex* meta_index, bool& is_exists)
{
    is_exists = false;

    MRQuery qSelect(this, "SELECT COUNT(*) FROM sqlite_master WHERE lower(type) = 'index' AND lower(tbl_name) = lower(?) AND lower(name) = lower(?)");
    qSelect.BindString(meta_index->TableName);
    qSelect.BindString(meta_index->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = MRString::StringF("Cannot check exist index '%s': %s",
            meta_index->Name.c_str(), qSelect.GetErrorString().c_str());
        return false;
    }

    is_exists = qSelect.ReadColumn_Int(0) == 1;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::IndexCreate(const TMetaIndex* meta_index)
{
    MRQuery qCreate(this, MRSQLProcessor::SQLite::IndexCreate(meta_index));
    if (!qCreate.Execute())
    {
        m_ErrorString = qCreate.GetErrorString();
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::IndexUpdate(const TMetaIndex* meta_index)
{
    //Проверим, не изменилась ли уникальность
    MRQuery qSelect(this, MRString::StringF("PRAGMA index_list(%s);", meta_index->TableName.c_str()));
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }

    bool need_update = false;

    while (qSelect.Next())
    {
        if (MRString::StringToLowerGet(qSelect.ReadColumn(1)) ==
            MRString::StringToLowerGet(meta_index->Name))
        {
            need_update = (qSelect.ReadColumn_Int(2) == 1 ? true : false) != meta_index->Unique;
            break;
        }
    }
    qSelect.ClearStatement();

    //Если уникальность не менялась - проверим что там с полями
    if (!need_update)
    {
        if (!qSelect.Execute(MRString::StringF("PRAGMA index_info(%s);", meta_index->Name.c_str())))
        {
            m_ErrorString = qSelect.GetErrorString();
            return false;
        }

        ISVectorString index_fields;
        while (qSelect.Next())
        {
            index_fields.emplace_back(MRString::StringToLowerGet(qSelect.ReadColumn(2)));
        }
        std::sort(index_fields.begin(), index_fields.end());

        //Если наборы полей отличаются - нужно обновление
        need_update = index_fields != meta_index->Fields;
    }

    if (need_update)
    {
        MRQuery qDelete(this, MRString::StringF("DROP INDEX %s;", meta_index->Name.c_str()));
        if (!qDelete.Execute())
        {
            m_ErrorString = qDelete.GetErrorString();
            return false;
        }

        if (!IndexCreate(meta_index))
        {
            return false;
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::ForeignExists(const TMetaForeign* meta_foreign, bool& is_exists)
{
    (void)meta_foreign;
    (void)is_exists;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::ForeignCreate(const TMetaForeign* meta_foreign)
{
    (void)meta_foreign;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::ForeignUpdate(const TMetaForeign* meta_foreign)
{
    (void)meta_foreign;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::ViewExists(const TMetaView* meta_view, bool& is_exists)
{
    is_exists = false;

    MRQuery qSelect(this, "SELECT COUNT(*) FROM sqlite_master WHERE lower(type) = 'view' AND lower(name) = lower(?)");
    qSelect.BindString(meta_view->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = MRString::StringF("Cannot check exist view '%s': %s",
            meta_view->Name.c_str(), qSelect.GetErrorString().c_str());
        return false;
    }

    is_exists = qSelect.ReadColumn_Int(0) == 1;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::ViewCreate(const TMetaView* meta_view)
{
    MRQuery qCreate(this, MRSQLProcessor::SQLite::ViewCreate(meta_view));
    if (!qCreate.Execute())
    {
        m_ErrorString = qCreate.GetErrorString();
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabaseSQLite::ViewUpdate(const TMetaView* meta_view)
{
    MRQuery qDrop(this, "DROP VIEW " + meta_view->Name);
    if (!qDrop.Execute())
    {
        m_ErrorString = qDrop.GetErrorString();
        return false;
    }
    return ViewCreate(meta_view);
}
//-----------------------------------------------------------------------------
void IDatabaseSQLite::GetOldObjects(const std::vector<ShowOldType>& v, IDatabase::ShowOldStruct& s, std::optional<std::string>& error_string)
{
    //Вытаскиваем таблицы
    if (MRTemplate::VectorContains(v, IDatabase::ShowOldType::Tables))
    {
        MRQuery q(this, "SELECT name FROM sqlite_master WHERE lower(type) = 'table' ORDER BY name");
        if (q.Execute())
        {
            while (q.Next())
            {
                std::string table_name = q.ReadColumn(0);
                if (!MRMetaData::Instance().GetMetaTable(table_name))
                {
                    s.Tables.emplace_back(table_name);
                }
            }
        }
        else
        {
            error_string = q.GetErrorString();
            return;
        }
    }

    //Вытаскиваем поля
    if (MRTemplate::VectorContains(v, IDatabase::ShowOldType::Fields))
    {
        MRQuery qTables(this, "SELECT name FROM sqlite_master WHERE lower(type) = 'table' ORDER BY name");
        if (qTables.Execute())
        {
            while (qTables.Next())
            {
                std::string table_name = qTables.ReadColumn(0);

                MRQuery qFields(this, "SELECT name FROM pragma_table_info(?)");
                qFields.BindString(table_name);
                if (qFields.Execute())
                {
                    while (qFields.Next())
                    {
                        if (const TMetaTable* meta_table = MRMetaData::Instance().GetMetaTable(table_name); meta_table)
                        {
                            if (!meta_table->GetField(qFields.ReadColumn(0)))
                            {
                                s.Fields.emplace_back(table_name + "." + qFields.ReadColumn(0));
                            }
                        }
                    }
                }
                else
                {
                    error_string = qTables.GetErrorString();
                    return;
                }
            }
        }
        else
        {
            error_string = qTables.GetErrorString();
            return;
        }
    }

    //Вытаскиваем представления
    if (MRTemplate::VectorContains(v, IDatabase::ShowOldType::Views))
    {
        MRQuery q(this, "SELECT name FROM sqlite_master WHERE lower(type) = 'view' ORDER BY name");
        if (q.Execute())
        {
            while (q.Next())
            {
                std::string view_name = q.ReadColumn(0);
                if (!MRMetaData::Instance().GetMetaView(view_name))
                {
                    s.Views.emplace_back(view_name);
                }
            }
        }
        else
        {
            error_string = q.GetErrorString();
            return;
        }
    }

    //Вытаскиваем индексы
    if (MRTemplate::VectorContains(v, IDatabase::ShowOldType::Indexes))
    {
        MRQuery q(this, "SELECT name FROM sqlite_master WHERE lower(type) = 'index' AND lower(name) NOT LIKE '%sqlite_autoindex%' ORDER BY name");
        if (q.Execute())
        {
            while (q.Next())
            {
                std::string index_name = q.ReadColumn(0);
                if (!MRMetaData::Instance().GetMetaIndex(index_name))
                {
                    s.Indexes.emplace_back(index_name);
                }
            }
        }
        else
        {
            error_string = q.GetErrorString();
            return;
        }
    }
}
//-----------------------------------------------------------------------------
