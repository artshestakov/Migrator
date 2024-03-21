#include "IQueryPostgreSQL.h"
#include "IDatabasePostgreSQL.h"
//-----------------------------------------------------------------------------
IQueryPostgreSQL::IQueryPostgreSQL(IDatabase* db, const std::string& sql)
    : IQuery(db, sql),
    m_DB(static_cast<IDatabasePostgreSQL*>(m_IDB)->GetPointer()),
    m_Result(nullptr),
    m_IsSelect(false),
    m_CurrentRow(-1),
    m_Rows(0),
    m_Columns(0),
    m_ParameterCount(0)
{

}
//-----------------------------------------------------------------------------
IQueryPostgreSQL::~IQueryPostgreSQL()
{
    ClearStatement();
}
//-----------------------------------------------------------------------------
void IQueryPostgreSQL::BindInt(int i)
{
    std::string str = std::to_string(i);
    char* Temp = (char*)malloc(str.size() + 1);
    if (Temp)
    {
        strcpy(Temp, str.c_str());
        m_ParameterValues.emplace_back(Temp);
        m_ParameterTypes.emplace_back(20);
        m_ParameterFormats.emplace_back(0);
        m_ParameterLengths.emplace_back(0);
        ++m_ParameterCount;
    }
}
//-----------------------------------------------------------------------------
void IQueryPostgreSQL::BindString(const std::string& s)
{
    if (s.empty())
    {
        m_ParameterValues.emplace_back(nullptr);
    }
    else
    {
        char* Value = (char*)malloc(s.size() + 1);
        if (Value)
        {
            strcpy(Value, s.c_str());
            m_ParameterValues.emplace_back(Value);
        }
    }
    m_ParameterTypes.emplace_back(1043);
    m_ParameterFormats.emplace_back(0);
    m_ParameterLengths.emplace_back(0);
    ++m_ParameterCount;
}
//-----------------------------------------------------------------------------
bool IQueryPostgreSQL::IsNull(int i)
{
    return MRDefSingleton::Get().PostgreSQL->PQgetisnull(m_Result, m_CurrentRow, i);
}
//-----------------------------------------------------------------------------
void IQueryPostgreSQL::ClearStatement()
{
    if (m_Result)
    {
        MRDefSingleton::Get().PostgreSQL->PQclear(m_Result);
        m_Result = nullptr;
    }
}
//-----------------------------------------------------------------------------
bool IQueryPostgreSQL::Execute(const std::string& sql)
{
    ClearStatement();

    if (!sql.empty() && sql != m_SQL)
    {
        m_SQL = sql;
    }

    if (m_ParameterCount > 0)
    {
        m_Result = MRDefSingleton::Get().PostgreSQL->PQexecParams(m_DB, m_SQL.c_str(), (int)m_ParameterCount,
            m_ParameterTypes.data(), m_ParameterValues.data(), m_ParameterLengths.data(), m_ParameterFormats.data(), 0);
    }
    else
    {
        m_Result = MRDefSingleton::Get().PostgreSQL->PQexec(m_DB, m_SQL.c_str());
    }

    bool r = false;
    switch (MRDefSingleton::Get().PostgreSQL->PQresultStatus(m_Result)) //Проверяем результат
    {
    case MRDefPostgreSQL::ExecStatusType::PGRES_TUPLES_OK:
        m_Rows = MRDefSingleton::Get().PostgreSQL->PQntuples(m_Result);
        m_Columns = MRDefSingleton::Get().PostgreSQL->PQnfields(m_Result);
        m_CurrentRow = -1;
        m_IsSelect = true;
        r = true;
        break;

    case MRDefPostgreSQL::ExecStatusType::PGRES_SINGLE_TUPLE:
        m_IsSelect = true;
        r = true;
        break;

    case MRDefPostgreSQL::ExecStatusType::PGRES_COMMAND_OK:
        r = true;
        break;

    case MRDefPostgreSQL::ExecStatusType::PGRES_FATAL_ERROR: //Ошибка - проверим, не упало ли соединение с БД
        if (MRDefSingleton::Get().PostgreSQL->PQstatus(m_DB) != 0)
        {
            //if (Reconnect())
            {
                return Execute();
            }
        }
        break;

    default: //Неизвестно что произошло
        m_ErrorString = "Unknown result status";
    }

    if (m_ParameterCount > 0)
    {
        std::for_each(m_ParameterValues.begin(), m_ParameterValues.end(), [](char* param) { free(param); });
        m_ParameterValues.clear();
        m_ParameterTypes.clear();
        m_ParameterFormats.clear();
        m_ParameterLengths.clear();
        m_ParameterCount = 0;
    }

    if (!r)
    {
        m_ErrorString = MRDefSingleton::Get().PostgreSQL->PQerrorMessage(m_DB);
    }
    return r;
}
//-----------------------------------------------------------------------------
bool IQueryPostgreSQL::First()
{
    if (m_CurrentRow + 1 < m_Rows)
    {
        ++m_CurrentRow;
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
bool IQueryPostgreSQL::Next()
{
    ++m_CurrentRow;
    if (m_CurrentRow == m_Rows)
    {
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
int IQueryPostgreSQL::GetColumnCount() const
{
    return m_Columns;
}
//-----------------------------------------------------------------------------
const char* IQueryPostgreSQL::ReadColumn(int index)
{
    return MRDefSingleton::Get().PostgreSQL->PQgetvalue(m_Result, m_CurrentRow, index);
}
//-----------------------------------------------------------------------------
int IQueryPostgreSQL::ReadColumn_Int(int index)
{
    return std::atoi(ReadColumn(index));
}
//-----------------------------------------------------------------------------
bool IQueryPostgreSQL::ReadColumn_Bool(int index)
{
    return (strcmp(ReadColumn(index), "t") == 0) ? true : false;
}
//-----------------------------------------------------------------------------
bool IQueryPostgreSQL::Prepare()
{
    return true;
}
//-----------------------------------------------------------------------------
bool IQueryPostgreSQL::FillParams()
{
    return true;
}
//-----------------------------------------------------------------------------
