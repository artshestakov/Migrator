#include "MRDatabase.h"
#include "IDatabaseSQLite.h"
#include "IDatabasePostgreSQL.h"
#include "MRQuery.h"
#include "MRMetaData.h"
//-----------------------------------------------------------------------------
MRDatabase::MRDatabase(IDatabase::DatabaseType database_type)
    : m_DB(nullptr)
{
    switch (database_type)
    {
    case IDatabase::DatabaseType::SQLite:
        m_DB = new IDatabaseSQLite(database_type);
        break;

    case IDatabase::DatabaseType::PostgreSQL:
        m_DB = new IDatabasePostgreSQL(database_type);
        break;

    default:
    { }
    }
}
//-----------------------------------------------------------------------------
MRDatabase::~MRDatabase()
{
    if (m_DB)
    {
        delete m_DB;
    }
}
//-----------------------------------------------------------------------------
const std::string& MRDatabase::GetErrorString() const
{
    return m_DB->GetErrorString();
}
//-----------------------------------------------------------------------------
bool MRDatabase::InitLibrary()
{
    return m_DB->InitLibrary();
}
//-----------------------------------------------------------------------------
const std::string& MRDatabase::GetLibraryPath() const
{
    return m_DB->GetLibraryPath();
}
//-----------------------------------------------------------------------------
std::string MRDatabase::GetVersion() const
{
    return m_DB->GetVersion();
}
//-----------------------------------------------------------------------------
bool MRDatabase::ExistsDB(bool& is_exists)
{
    return m_DB->ExistsDB(is_exists);
}
//-----------------------------------------------------------------------------
bool MRDatabase::Create()
{
    return m_DB->Create();
}
//-----------------------------------------------------------------------------
bool MRDatabase::Connect()
{
    return m_DB->Connect();
}
//-----------------------------------------------------------------------------
bool MRDatabase::Disconnect()
{
    return m_DB->Disconnect();
}
//-----------------------------------------------------------------------------
bool MRDatabase::Execute(const TMetaExecute* meta_execute)
{
    return m_DB->Execute(meta_execute);
}
//-----------------------------------------------------------------------------
bool MRDatabase::TableExists(const TMetaTable* meta_table, bool& is_exists)
{
    return m_DB->TableExists(meta_table, is_exists);
}
//-----------------------------------------------------------------------------
bool MRDatabase::TableCreate(const TMetaTable* meta_table)
{
    return m_DB->TableCreate(meta_table);
}
//-----------------------------------------------------------------------------
bool MRDatabase::TableUpdate(const TMetaTable* meta_table)
{
    return m_DB->TableUpdate(meta_table);
}
//-----------------------------------------------------------------------------
bool MRDatabase::FieldExists(const TMetaField* meta_field, bool& is_exists)
{
    return m_DB->FieldExists(meta_field, is_exists);
}
//-----------------------------------------------------------------------------
bool MRDatabase::FieldCreate(const TMetaField* meta_field)
{
    return m_DB->FieldCreate(meta_field);
}
//-----------------------------------------------------------------------------
bool MRDatabase::IndexExists(const TMetaIndex* meta_index, bool& is_exists)
{
    return m_DB->IndexExists(meta_index, is_exists);
}
//-----------------------------------------------------------------------------
bool MRDatabase::IndexCreate(const TMetaIndex* meta_index)
{
    return m_DB->IndexCreate(meta_index);
}
//-----------------------------------------------------------------------------
bool MRDatabase::IndexUpdate(const TMetaIndex* meta_index)
{
    return m_DB->IndexUpdate(meta_index);
}
//-----------------------------------------------------------------------------
bool MRDatabase::ForeignExists(const TMetaForeign* meta_foreign, bool& is_exists)
{
    return m_DB->ForeignExists(meta_foreign, is_exists);
}
//-----------------------------------------------------------------------------
bool MRDatabase::ForeignCreate(const TMetaForeign* meta_foreign)
{
    return m_DB->ForeignCreate(meta_foreign);
}
//-----------------------------------------------------------------------------
bool MRDatabase::ForeignUpdate(const TMetaForeign* meta_foreign)
{
    return m_DB->ForeignUpdate(meta_foreign);
}
//-----------------------------------------------------------------------------
bool MRDatabase::ViewExists(const TMetaView* meta_view, bool& is_exists)
{
    return m_DB->ViewExists(meta_view, is_exists);
}
//-----------------------------------------------------------------------------
bool MRDatabase::ViewCreate(const TMetaView* meta_view)
{
    return m_DB->ViewCreate(meta_view);
}
//-----------------------------------------------------------------------------
bool MRDatabase::ViewUpdate(const TMetaView* meta_view)
{
    return m_DB->ViewUpdate(meta_view);
}
//-----------------------------------------------------------------------------
void MRDatabase::GetOldObjects(const std::vector<IDatabase::ShowOldType>& v, IDatabase::ShowOldStruct& s, std::optional<std::string>& error_string)
{
    m_DB->GetOldObjects(v, s, error_string);
}
//-----------------------------------------------------------------------------
IDatabase* MRDatabase::GetInterface() const
{
    return m_DB;
}
//-----------------------------------------------------------------------------
