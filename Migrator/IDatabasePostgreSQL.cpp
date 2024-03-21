#include "IDatabasePostgreSQL.h"
#include "ISConfig.h"
#include "MRQuery.h"
#include "MRSQLProcessor.h"
#include "MRMetaData.h"
#include "MRTemplate.h"
#include "MRConstants.h"
//-----------------------------------------------------------------------------
IDatabasePostgreSQL::IDatabasePostgreSQL(DatabaseType database_type)
    : IDatabase(database_type),
    m_Host(ISConfig::Instance().GetString("PostgreSQL", "Host")),
    m_Port(ISConfig::Instance().GetInt("PostgreSQL", "Port")),
    m_DatabaseName(ISConfig::Instance().GetString("PostgreSQL", "DatabaseName")),
    m_User(ISConfig::Instance().GetString("PostgreSQL", "User")),
    m_Password(ISConfig::Instance().GetString("PostgreSQL", "Password")),
    m_SystemDBName(ISConfig::Instance().GetString("PostgreSQL", "SystemDBName")),
    m_UseSystemDB(false),
    m_DB(nullptr)
{

}
//-----------------------------------------------------------------------------
IDatabasePostgreSQL::~IDatabasePostgreSQL()
{

}
//-----------------------------------------------------------------------------
std::string IDatabasePostgreSQL::GetVersion() const
{
    int v = MRDefSingleton::Get().PostgreSQL->PQLibVersion();
    return ISAlgorithm::StringF("%d.%d.%d", v / 10000, (v / 100) %100, v % 100);
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::ExistsDB(bool& is_exists)
{
    m_UseSystemDB = true;
    bool r = Connect();
    m_UseSystemDB = false;

    if (!r)
    {
        return false;
    }

    MRQuery qSelect(this, "SELECT COUNT(*) FROM pg_database WHERE datname = $1");
    qSelect.BindString(m_DatabaseName);
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }
    qSelect.First();

    is_exists = qSelect.ReadColumn_Int(0) != 0;
    Disconnect();
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::Create()
{
    m_UseSystemDB = true;
    bool r = Connect();
    m_UseSystemDB = false;

    if (!r)
    {
        return false;
    }

    MRQuery qCreate(this, ISAlgorithm::StringF("CREATE DATABASE %s WITH OWNER = %s ENCODING = 'UTF8'", m_DatabaseName.c_str(), m_User.c_str()));
    if (!qCreate.Execute())
    {
        m_ErrorString = qCreate.GetErrorString();
        return false;
    }

    Disconnect();
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::Connect()
{
    //Формируем строку подключения
    std::string connection_string = ISAlgorithm::StringF("host=%s port=%d dbname=%s user=%s password=%s connect_timeout=3 application_name=Migrator",
        m_Host.c_str(), m_Port, (m_UseSystemDB ? m_SystemDBName : m_DatabaseName).c_str(), m_User.c_str(), m_Password.c_str());

    auto ping = MRDefSingleton::Get().PostgreSQL->PQping(connection_string.c_str());
    if (ping != MRDefPostgreSQL::PGPing::PQPING_OK)
    {
        switch (ping)
        {
        case MRDefPostgreSQL::PGPing::PQPING_REJECT:
            m_ErrorString = "The server reject connection";
            break;

        case MRDefPostgreSQL::PGPing::PQPING_NO_RESPONSE:
            m_ErrorString = "The server no responce";
            break;

        case MRDefPostgreSQL::PGPing::PQPING_NO_ATTEMPT:
            m_ErrorString = "No contact was made with the server: invalid connection parameters or out of memory";
            break;

        default:
            m_ErrorString = "Unknown error";
        }
        return false;
    }

    m_DB = MRDefSingleton::Get().PostgreSQL->PQconnectdb(connection_string.c_str());
    if (!m_DB)
    {
        m_ErrorString = "PQconnectdb returned null";
        return false;
    }

    if (MRDefSingleton::Get().PostgreSQL->PQstatus(m_DB) != 0)
    {
        m_ErrorString = MRDefSingleton::Get().PostgreSQL->PQerrorMessage(m_DB);
        MRDefSingleton::Get().PostgreSQL->PQfinish(m_DB);
        return false;
    }

    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::Disconnect()
{
    MRDefSingleton::Get().PostgreSQL->PQfinish(m_DB);
    return true;
}
//-----------------------------------------------------------------------------
MRDefPostgreSQL::PGconn* IDatabasePostgreSQL::GetPointer() const
{
    return m_DB;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::Execute(const TMetaExecute* meta_execute)
{
    MRQuery q(this, meta_execute->SQL);
    if (!q.Execute())
    {
        m_ErrorString = q.GetErrorString();
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::TableExists(const TMetaTable* meta_table, bool& is_exists)
{
    MRQuery qSelect(this, "SELECT COUNT(*) FROM pg_tables WHERE schemaname = current_schema() AND tablename = $1");
    qSelect.BindString(meta_table->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }
    qSelect.First();

    is_exists = qSelect.ReadColumn_Int(0) != 0;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::TableCreate(const TMetaTable* meta_table)
{
    MRQuery q(this, MRSQLProcessor::PostgreSQL::TableCreate(meta_table));
    if (!q.Execute())
    {
        m_ErrorString = q.GetErrorString();
        return false;
    }

    //Таблица успешно создалась - комментируем
    (void)q.Execute(MRSQLProcessor::PostgreSQL::TableComment(meta_table));

    //Комментируем поля
    for (const TMetaField* meta_field : meta_table->Fields)
    {
        (void)q.Execute(MRSQLProcessor::PostgreSQL::FieldComment(meta_field));
    }

    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::TableUpdate(const TMetaTable* meta_table)
{
    //Прокомментируем таблицу
    MRQuery qComment(this);
    (void)qComment.Execute(MRSQLProcessor::PostgreSQL::TableComment(meta_table));

    bool is_exists = false;

    //Проверяем, не добавились ли новые поля
    for (const TMetaField* meta_field : meta_table->Fields)
    {
        if (!FieldExists(meta_field, is_exists))
        {
            return false;
        }

        if (!is_exists)
        {
            FieldCreate(meta_field);
        }

        //Прокомментируем поле
        (void)qComment.Execute(MRSQLProcessor::PostgreSQL::FieldComment(meta_field));
    }

    //Проверяем, не нужно ли удалять поля
    MRQuery qSelect(this, "SELECT column_name "
        "FROM information_schema.columns "
        "WHERE table_schema = current_schema() "
        "AND table_name = $1 "
        "ORDER BY ordinal_position");
    qSelect.BindString(meta_table->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }

    while (qSelect.Next())
    {
        //Если такого поля нет в мета-данных - удаляем его из БД
        const char* field_name = qSelect.ReadColumn(0);
        if (!meta_table->GetField(field_name))
        {
            MRQuery qDrop(this, ISAlgorithm::StringF("ALTER TABLE %s DROP COLUMN %s",
                meta_table->Name.c_str(), field_name));
            if (!qDrop.Execute())
            {
                m_ErrorString = qDrop.GetErrorString();
                return false;
            }
        }
    }

    //Начинаем обновлять поля
    MRQuery qSelectFields(this, 
        "SELECT c.column_name, "
        "CASE "
        "    WHEN COALESCE(c.character_maximum_length, 0) > 0 THEN cs.field_type || '(' || c.character_maximum_length || ')' "
        "    WHEN COALESCE(c.numeric_precision, 0) > 0 THEN cs.field_type || '(' || c.numeric_precision || ')' "
        "    ELSE cs.field_type "
        "END AS field_type, "
        "column_default,  "
        "CASE WHEN lower(is_nullable) = 'yes' THEN false ELSE true END AS is_nullable "
        "FROM information_schema.columns c, "
        "( "
        "    SELECT column_name, "
        "    CASE "
        "        WHEN data_type = 'character varying' THEN 'varchar' "
        "        WHEN data_type = 'character' THEN 'char' "
        "        ELSE data_type "
        "    END AS field_type "
        "    FROM information_schema.columns "
        "    WHERE table_schema = current_schema() "
        "    AND table_name = $1 "
        ") AS cs " //Тут фактически выполняем CASE
        "WHERE cs.column_name = c.column_name " //Чтобы не было дублирования поле
        "AND table_schema = current_schema() "
        "AND table_name = $1 "
        "ORDER BY ordinal_position");
    qSelectFields.BindString(meta_table->Name);
    if (!qSelectFields.Execute())
    {
        m_ErrorString = qSelectFields.GetErrorString();
        return false;
    }

    while (qSelectFields.Next())
    {
        std::string field_name = qSelectFields.ReadColumn(0);
        std::string field_type = qSelectFields.ReadColumn(1);
        std::string field_default = qSelectFields.ReadColumn(2);
        bool field_not_null = qSelectFields.ReadColumn_Bool(3);

        const TMetaField* meta_field = meta_table->GetField(field_name);

        if (field_type != meta_field->Type)
        {
            std::string using_type = meta_field->Type;
            if (size_t pos = using_type.find("("); pos != std::string::npos)
            {
                using_type = ISAlgorithm::StringLeft(using_type, pos);
            }

            MRQuery qAlterType(this, ISAlgorithm::StringF("ALTER TABLE \"%s\" ALTER COLUMN \"%s\" TYPE %s USING(%s::%s)",
                meta_table->Name.c_str(), meta_field->Name.c_str(), meta_field->Type.c_str(), meta_field->Name.c_str(), using_type.c_str()));
            if (!qAlterType.Execute())
            {
                m_ErrorString = qAlterType.GetErrorString();
                return false;
            }
        }

        //К сожалению придётся вызывать обновление значение по умолчанию всегда из-за особенностей PostgreSQL
        MRQuery qAlterDefault(this, MRSQLProcessor::PostgreSQL::FieldDefault(meta_field));
        if (!qAlterDefault.Execute())
        {
            m_ErrorString = qAlterDefault.GetErrorString();
            return false;
        }

        if (field_not_null != meta_field->NotNull)
        {
            MRQuery qAlterNotNull(this, ISAlgorithm::StringF("ALTER TABLE \"%s\" ALTER COLUMN \"%s\" %s NOT NULL",
                meta_table->Name.c_str(), meta_field->Name.c_str(), (meta_field->NotNull ? "SET" : "DROP")));
            if (!qAlterNotNull.Execute())
            {
                m_ErrorString = qAlterNotNull.GetErrorString();
                return false;
            }
        }
    }

    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::FieldExists(const TMetaField* meta_field, bool& is_exists)
{
    MRQuery qSelect(this, "SELECT COUNT(*) "
        "FROM information_schema.columns "
        "WHERE table_schema = current_schema()"
        "AND table_name = $1 "
        "AND column_name = $2");
    qSelect.BindString(meta_field->MetaTable->Name);
    qSelect.BindString(meta_field->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }
    (void)qSelect.First();

    is_exists = qSelect.ReadColumn_Int(0) > 0;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::FieldCreate(const TMetaField* meta_field)
{
    MRQuery qAlter(this, MRSQLProcessor::PostgreSQL::FieldCreate(meta_field));
    if (!qAlter.Execute())
    {
        m_ErrorString = qAlter.GetErrorString();
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::IndexExists(const TMetaIndex* meta_index, bool& is_exists)
{
    MRQuery qSelect(this, "SELECT COUNT(*) "
        "FROM pg_indexes "
        "WHERE schemaname = current_schema() "
        "AND indexname = $1");
    qSelect.BindString(meta_index->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }
    (void)qSelect.First();

    is_exists = qSelect.ReadColumn_Int(0) > 0;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::IndexCreate(const TMetaIndex* meta_index)
{
    MRQuery qCreate(this, MRSQLProcessor::PostgreSQL::IndexCreate(meta_index));
    if (!qCreate.Execute())
    {
        m_ErrorString = qCreate.GetErrorString();
        return false;
    }

    //Комментируем индекс
    (void)qCreate.Execute(MRSQLProcessor::PostgreSQL::IndexComment(meta_index));

    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::IndexUpdate(const TMetaIndex* meta_index)
{
    MRQuery qSelectUnqiue(this, "SELECT i.indisunique "
        "FROM pg_index i "
        "JOIN pg_class c ON c.oid = i.indexrelid "
        "JOIN pg_namespace n ON n.oid = c.relnamespace "
        "WHERE c.relname = $1");
    qSelectUnqiue.BindString(meta_index->Name);
    if (!qSelectUnqiue.Execute())
    {
        m_ErrorString = qSelectUnqiue.GetErrorString();
        return false;
    }
    (void)qSelectUnqiue.First();

    bool need_update = meta_index->Unique != qSelectUnqiue.ReadColumn_Bool(0);

    //Если уникальность не менялась, то проверим не поменялся ли состав полей
    if (!need_update)
    {
        MRQuery qSelectFields(this, "SELECT a.attname AS column_name "
            "FROM pg_class c1, pg_class c2, pg_index i, pg_attribute a "
            "WHERE c1.oid = i.indrelid "
            "AND c2.oid = i.indexrelid "
            "AND a.attrelid = c1.oid "
            "AND a.attnum = ANY(i.indkey) "
            "AND c1.relkind = 'r' "
            "AND c2.relname = $1 "
            "ORDER BY a.attname");
        qSelectFields.BindString(meta_index->Name);
        if (!qSelectFields.Execute())
        {
            m_ErrorString = qSelectFields.GetErrorString();
            return false;
        }

        ISVectorString index_fields;
        while (qSelectFields.Next())
        {
            index_fields.emplace_back(qSelectFields.ReadColumn(0));
        }
        need_update = meta_index->Fields != index_fields;
    }

    //Обновление все-таки необходимо - удаляем индекс и создаём по-новой
    if (need_update)
    {
        MRQuery qDrop(this, "DROP INDEX " + meta_index->Name);
        if (!qDrop.Execute())
        {
            m_ErrorString = qDrop.GetErrorString();
            return false;
        }
        return IndexCreate(meta_index);
    }

    //Принудительно комментируем индекс
    MRQuery qComment(this, MRSQLProcessor::PostgreSQL::IndexComment(meta_index));
    (void)qComment.Execute();
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::ForeignExists(const TMetaForeign* meta_foreign, bool& is_exists)
{
    MRQuery qSelect(this, "SELECT COUNT(*) "
        "FROM pg_constraint "
        "WHERE conrelid::regclass::VARCHAR = $1 "
        "AND conname = $2");
    qSelect.BindString(meta_foreign->TableName);
    qSelect.BindString(meta_foreign->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }
    (void)qSelect.First();

    is_exists = qSelect.ReadColumn_Int(0) > 0;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::ForeignCreate(const TMetaForeign* meta_foreign)
{
    MRQuery qCreate(this, MRSQLProcessor::PostgreSQL::ForeignCreate(meta_foreign));
    if (!qCreate.Execute())
    {
        m_ErrorString = qCreate.GetErrorString();
        return false;
    }

    //Комментируем внешний ключ
    (void)qCreate.Execute(MRSQLProcessor::PostgreSQL::ForeignComment(meta_foreign));

    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::ForeignUpdate(const TMetaForeign* meta_foreign)
{
    MRQuery qDrop(this, ISAlgorithm::StringF("ALTER TABLE %s DROP CONSTRAINT %s RESTRICT",
        meta_foreign->TableName.c_str(), meta_foreign->Name.c_str()));
    if (!qDrop.Execute())
    {
        m_ErrorString = qDrop.GetErrorString();
        return false;
    }
    return ForeignCreate(meta_foreign);
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::ViewExists(const TMetaView* meta_view, bool& is_exists)
{
    MRQuery qSelect(this, "SELECT COUNT(*) "
        "FROM pg_catalog.pg_views "
        "WHERE schemaname = current_schema() "
        "AND viewname = $1");
    qSelect.BindString(meta_view->Name);
    if (!qSelect.Execute())
    {
        m_ErrorString = qSelect.GetErrorString();
        return false;
    }
    (void)qSelect.First();

    is_exists = qSelect.ReadColumn_Int(0) > 0;
    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::ViewCreate(const TMetaView* meta_view)
{
    MRQuery qCreate(this, MRSQLProcessor::PostgreSQL::ViewCreate(meta_view));
    if (!qCreate.Execute())
    {
        m_ErrorString = qCreate.GetErrorString();
        return false;
    }

    if (!meta_view->Comment.empty())
    {
        MRQuery qComment(this, MRSQLProcessor::PostgreSQL::ViewComment(meta_view));
        (void)qComment.Execute();
    }

    return true;
}
//-----------------------------------------------------------------------------
bool IDatabasePostgreSQL::ViewUpdate(const TMetaView* meta_view)
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
void IDatabasePostgreSQL::GetOldObjects(const std::vector<ShowOldType>& v, IDatabase::ShowOldStruct& s, std::optional<std::string>& error_string)
{
    //Вытаскиваем таблицы
    if (ISAlgorithm::VectorContains(v, IDatabase::ShowOldType::Tables))
    {
        MRQuery q(this, "SELECT tablename FROM pg_tables WHERE schemaname = current_schema() ORDER BY tablename");
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
    if (ISAlgorithm::VectorContains(v, IDatabase::ShowOldType::Fields))
    {
        MRQuery q(this, "SELECT table_name, column_name FROM information_schema.columns WHERE table_schema = current_schema() ORDER BY table_name, ordinal_position");
        if (q.Execute())
        {
            while (q.Next())
            {
                std::string table_name = q.ReadColumn(0);
                std::string field_name = q.ReadColumn(1);

                if (const TMetaTable* meta_table = MRMetaData::Instance().GetMetaTable(table_name); meta_table)
                {
                    if (!meta_table->GetField(field_name))
                    {
                        s.Fields.emplace_back(table_name + "." + field_name);
                    }
                }
            }
        }
        else
        {
            error_string = q.GetErrorString();
            return;
        }
    }

    //Вытаскиваем представления
    if (ISAlgorithm::VectorContains(v, IDatabase::ShowOldType::Views))
    {
        MRQuery q(this, "SELECT viewname FROM pg_catalog.pg_views WHERE schemaname = current_schema() ORDER BY viewname");
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
    if (ISAlgorithm::VectorContains(v, IDatabase::ShowOldType::Indexes))
    {
        MRQuery q(this, "SELECT indexname "
            "FROM pg_indexes "
            "WHERE schemaname = current_schema() "
            "AND indexname NOT IN (SELECT constraint_name FROM information_schema.table_constraints WHERE constraint_type = 'PRIMARY KEY') "
            "ORDER BY indexname");
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

    //Вытаскиваем внешние ключи
    if (ISAlgorithm::VectorContains(v, IDatabase::ShowOldType::Foreigns))
    {
        MRQuery q(this, "SELECT conname FROM pg_constraint WHERE contype = 'f' AND connamespace = current_schema()::regnamespace ORDER BY conname");
        if (q.Execute())
        {
            while (q.Next())
            {
                std::string foreign_name = q.ReadColumn(0);
                if (!MRMetaData::Instance().GetMetaForeign(foreign_name))
                {
                    s.Foreigns.emplace_back(foreign_name);
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
