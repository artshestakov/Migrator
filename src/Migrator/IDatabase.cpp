#include "IDatabase.h"
#include "MRDefDatabase.h"
//-----------------------------------------------------------------------------
IDatabase::IDatabase(DatabaseType database_type)
    : m_DatabaseType(database_type)
{

}
//-----------------------------------------------------------------------------
IDatabase::~IDatabase()
{

}
//-----------------------------------------------------------------------------
const std::string& IDatabase::GetErrorString() const
{
    return m_ErrorString;
}
//-----------------------------------------------------------------------------
IDatabase::DatabaseType IDatabase::GetDatabaseType() const
{
    return m_DatabaseType;
}
//-----------------------------------------------------------------------------
bool IDatabase::InitLibrary()
{
    if (!MRDefSingleton::Get().Init(m_DatabaseType))
    {
        m_ErrorString = MRDefSingleton::Get().GetErrorString();
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
const std::string& IDatabase::GetLibraryPath() const
{
    return MRDefSingleton::Get().GetLibraryPath();
}
//-----------------------------------------------------------------------------
