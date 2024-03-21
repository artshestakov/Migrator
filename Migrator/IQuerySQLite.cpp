#include "IQuerySQLite.h"
#include "IDatabaseSQLite.h"
//-----------------------------------------------------------------------------
IQuerySQLite::IQuerySQLite(IDatabase* db, const std::string& sql)
    : IQuery(db, sql),
    m_DB(static_cast<IDatabaseSQLite*>(m_IDB)->GetPointer()),
    m_STMT(nullptr),
    m_CanNext(false),
    m_NeedClear(false),
    m_ColumnCount(0),
    m_BindIterator(0)
{

}
//-----------------------------------------------------------------------------
IQuerySQLite::~IQuerySQLite()
{
    ClearStatement();
}
//-----------------------------------------------------------------------------
void IQuerySQLite::BindInt(int i)
{
    m_Params.emplace_back(i);
}
//-----------------------------------------------------------------------------
void IQuerySQLite::BindString(const std::string& s)
{
    m_Params.emplace_back(s);
}
//-----------------------------------------------------------------------------
bool IQuerySQLite::IsNull(int i)
{
    return MRDefSingleton::Get().SQLite->sqlite3_column_type(m_STMT, i) == SQLITE_NULL;
}
//-----------------------------------------------------------------------------
void IQuerySQLite::ClearStatement()
{
    if (m_STMT)
    {
        if (int r = MRDefSingleton::Get().SQLite->sqlite3_reset(m_STMT); r != SQLITE_OK)
        {
            throw ISException(ISAlgorithm::StringF("cannot reset statement: %s", MRDefSingleton::Get().SQLite->sqlite3_errstr(r)));
        }

        if (int r = MRDefSingleton::Get().SQLite->sqlite3_finalize(m_STMT); r != SQLITE_OK)
        {
            throw ISException(ISAlgorithm::StringF("cannot finalize statement: %s", MRDefSingleton::Get().SQLite->sqlite3_errstr(r)));
        }

        m_STMT = nullptr;
    }
}
//-----------------------------------------------------------------------------
bool IQuerySQLite::Execute(const std::string& sql)
{
    if (m_NeedClear)
    {
        ClearStatement();
    }

    if (!sql.empty() && sql != m_SQL)
    {
        m_SQL = sql;
    }

    //Пропускаем все пустые запросы
    if (m_SQL.empty())
    {
        m_ErrorString = "Sql text is empty";
        return false;
    }

    if (!Prepare())
    {
        return false;
    }

    if (!FillParams())
    {
        return false;
    }

    int r = MRDefSingleton::Get().SQLite->sqlite3_step(m_STMT);

    //Помечаем, что чистка нужна только для выборок
    //Но чистка будет вызвана только в случае, если метод Execute вызовут ещё раз
    m_NeedClear = r == SQLITE_ROW;

    //Если оператор вернул нам какие-то данные, тогда мы можем гулять по выборке
    if (r == SQLITE_ROW)
    {
        m_CanNext = r == SQLITE_ROW; //Но при этом убедимся, что он действительно что-то вернул
        return true;
    }
    else if (r == SQLITE_DONE) //Запрос на выполнение чего-либо, т.е. НЕ выборка
    {
        ClearStatement(); //И только в таком случае вызываем очистку
        return true;
    }

    m_ErrorString = ISAlgorithm::StringF("Cannot execute sql query\n%s\nError: %s", m_SQL.c_str(), MRDefSingleton::Get().SQLite->sqlite3_errstr(r));
    return false;
}
//-----------------------------------------------------------------------------
bool IQuerySQLite::First()
{
    std::runtime_error("Is not realized");
    return true;
}
//-----------------------------------------------------------------------------
bool IQuerySQLite::Next()
{
    //Если нам разрешено "гулять" по выборке, то первый вызов пропускаем, т.к. в
    //методе Execute мы по умолчанию встали на первую строку
    //Повторный вывод этой функции переместить указатель на очередную (следующую) строку
    if (m_CanNext)
    {
        m_CanNext = false;
        return true;
    }

    return MRDefSingleton::Get().SQLite->sqlite3_step(m_STMT) == SQLITE_ROW;
}
//-----------------------------------------------------------------------------
int IQuerySQLite::GetColumnCount() const
{
    return m_ColumnCount;
}
//-----------------------------------------------------------------------------
const char* IQuerySQLite::ReadColumn(int index)
{
    return (const char*)MRDefSingleton::Get().SQLite->sqlite3_column_text(m_STMT, index);
}
//-----------------------------------------------------------------------------
int IQuerySQLite::ReadColumn_Int(int index)
{
    return MRDefSingleton::Get().SQLite->sqlite3_column_int(m_STMT, index);
}
//-----------------------------------------------------------------------------
bool IQuerySQLite::ReadColumn_Bool(int index)
{
    (void)index;
    throw ISException("Is not realized");
    return true;
}
//-----------------------------------------------------------------------------
bool IQuerySQLite::Prepare()
{
    if (int r = MRDefSingleton::Get().SQLite->sqlite3_prepare_v2(m_DB, m_SQL.c_str(), (int)m_SQL.size(), &m_STMT, nullptr); r != SQLITE_OK)
    {
        m_ErrorString = ISAlgorithm::StringF("Cannot prepare sql query\n%s\nError: %s", m_SQL.c_str(), MRDefSingleton::Get().SQLite->sqlite3_errstr(r));
        return false;
    }
    m_ColumnCount = MRDefSingleton::Get().SQLite->sqlite3_column_count(m_STMT);
    return true;
}
//-----------------------------------------------------------------------------
bool IQuerySQLite::FillParams()
{
    if (m_Params.empty())
    {
        return true;
    }

    int r = 0;
    for (size_t i = 0; i < m_Params.size(); ++i)
    {
        const ISVariant& param = m_Params[i];
        if (std::holds_alternative<std::nullptr_t>(param))
        {
            m_ErrorString = ISAlgorithm::StringF("Parameter %zu is null", i);
            return false;
        }

        if (std::holds_alternative<int>(param))
        {
            r = MRDefSingleton::Get().SQLite->sqlite3_bind_int(m_STMT, ++m_BindIterator, std::get<int>(param));
        }
        else if (std::holds_alternative<std::string>(param))
        {
            std::string s = std::get<std::string>(param);
            r = MRDefSingleton::Get().SQLite->sqlite3_bind_text(m_STMT, ++m_BindIterator, s.c_str(), (int)s.size(), (void*)-1); //Последний параметр SQLITE_TRANSIENT
        }

        if (r != SQLITE_OK)
        {
            m_ErrorString = ISAlgorithm::StringF("Cannot bind parameter %zu: %s", i, MRDefSingleton::Get().SQLite->sqlite3_errstr(r));
            return false;
        }
    }

    m_BindIterator = 0;
    m_Params.clear();
    return true;
}
//-----------------------------------------------------------------------------
