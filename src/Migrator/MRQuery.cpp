#include "MRQuery.h"
#include "IQuerySQLite.h"
#include "IQueryPostgreSQL.h"
//-----------------------------------------------------------------------------
MRQuery::MRQuery(IDatabase* db, const std::string& sql)
    : m_Query(nullptr)
{
    switch (db->GetDatabaseType())
    {
    case IDatabase::DatabaseType::SQLite:
        m_Query = new IQuerySQLite(db, sql);
        break;

    case IDatabase::DatabaseType::PostgreSQL:
        m_Query = new IQueryPostgreSQL(db, sql);
        break;

    default:
    { }
    }
}
//-----------------------------------------------------------------------------
MRQuery::~MRQuery()
{
    if (m_Query)
    {
        delete m_Query;
    }
}
//-----------------------------------------------------------------------------
const std::string& MRQuery::GetErrorString() const
{
    return m_Query->GetErrorString();
}
//-----------------------------------------------------------------------------
void MRQuery::BindInt(int i)
{
    m_Query->BindInt(i);
}
//-----------------------------------------------------------------------------
void MRQuery::BindString(const std::string& s)
{
    m_Query->BindString(s);
}
//-----------------------------------------------------------------------------
bool MRQuery::IsNull(int i)
{
    return m_Query->IsNull(i);
}
//-----------------------------------------------------------------------------
void MRQuery::ClearStatement()
{
    m_Query->ClearStatement();
}
//-----------------------------------------------------------------------------
bool MRQuery::Execute(const std::string& sql)
{
    return m_Query->Execute(sql);
}
//-----------------------------------------------------------------------------
bool MRQuery::First()
{
    return m_Query->First();
}
//-----------------------------------------------------------------------------
bool MRQuery::Next()
{
    return m_Query->Next();
}
//-----------------------------------------------------------------------------
int MRQuery::GetColumnCount() const
{
    return m_Query->GetColumnCount();
}
//-----------------------------------------------------------------------------
const char* MRQuery::ReadColumn(int index)
{
    return m_Query->ReadColumn(index);
}
//-----------------------------------------------------------------------------
int MRQuery::ReadColumn_Int(int index)
{
    return m_Query->ReadColumn_Int(index);
}
//-----------------------------------------------------------------------------
bool MRQuery::ReadColumn_Bool(int index)
{
    return m_Query->ReadColumn_Bool(index);
}
//-----------------------------------------------------------------------------
