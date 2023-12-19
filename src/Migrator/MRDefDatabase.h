#pragma once
//-----------------------------------------------------------------------------
#include "MRLibrary.h"
#include "MRConfig.h"
#include "IDatabase.h"
//-----------------------------------------------------------------------------
class MRDefBase
{
public:
    MRDefBase(const std::string& library_path)
    {
        m_Library.SetPath(library_path);
        m_Library.SetUseException(true);
        if (!m_Library.Load())
        {
            throw MRException(m_Library.GetErrorString());
        }
    }
    ~MRDefBase() = default;

protected:
    template <typename T>
    T Register(const std::string& function_name)
    {
        return m_Library.GetFunction<T>(function_name);
    }

private:
    MRLibrary m_Library;
};
//-----------------------------------------------------------------------------
class MRDefSQLite : public MRDefBase
{
public:
    struct sqlite3;
    struct sqlite3_stmt;

public:
    MRDefSQLite(const std::string& library_path)
        : MRDefBase(library_path)
    {
        sqlite3_libversion = Register<_def_sqlite3_libversion>("sqlite3_libversion");
        sqlite3_open = Register<_def_sqlite3_open>("sqlite3_open");
        sqlite3_close_v2 = Register<_def_sqlite3_close_v2>("sqlite3_close_v2");
        sqlite3_reset = Register<_def_sqlite3_reset>("sqlite3_reset");
        sqlite3_finalize = Register<_def_sqlite3_finalize>("sqlite3_finalize");
        sqlite3_step = Register<_def_sqlite3_step>("sqlite3_step");
        sqlite3_prepare_v2 = Register<_def_sqlite3_prepare_v2>("sqlite3_prepare_v2");
        sqlite3_errstr = Register<_def_sqlite3_errstr>("sqlite3_errstr");
        sqlite3_column_type = Register<_def_sqlite3_column_type>("sqlite3_column_type");
        sqlite3_column_text = Register<_def_sqlite3_column_text>("sqlite3_column_text");
        sqlite3_column_int = Register<_def_sqlite3_column_int>("sqlite3_column_int");
        sqlite3_column_count = Register<_def_sqlite3_column_count>("sqlite3_column_count");
        sqlite3_bind_int = Register<_def_sqlite3_bind_int>("sqlite3_bind_int");
        sqlite3_bind_text = Register<_def_sqlite3_bind_text>("sqlite3_bind_text");
    };
    ~MRDefSQLite() = default;

    typedef const char* (*_def_sqlite3_libversion)(void);
    _def_sqlite3_libversion sqlite3_libversion;

    typedef int(*_def_sqlite3_open)(const char*, sqlite3**);
    _def_sqlite3_open sqlite3_open;

    typedef int(*_def_sqlite3_close_v2)(sqlite3*);
    _def_sqlite3_close_v2 sqlite3_close_v2;

    typedef int(*_def_sqlite3_reset)(sqlite3_stmt*);
    _def_sqlite3_reset sqlite3_reset;

    typedef int(*_def_sqlite3_finalize)(sqlite3_stmt*);
    _def_sqlite3_finalize sqlite3_finalize;

    typedef int(*_def_sqlite3_step)(sqlite3_stmt*);
    _def_sqlite3_step sqlite3_step;

    typedef int(*_def_sqlite3_prepare_v2)(void*, const char*, int, sqlite3_stmt**, const char**);
    _def_sqlite3_prepare_v2 sqlite3_prepare_v2;

    typedef const char* (*_def_sqlite3_errstr)(int);
    _def_sqlite3_errstr sqlite3_errstr;

    typedef int(*_def_sqlite3_column_type)(sqlite3_stmt*, int);
    _def_sqlite3_column_type sqlite3_column_type;

    typedef const unsigned char* (*_def_sqlite3_column_text)(sqlite3_stmt*, int);
    _def_sqlite3_column_text sqlite3_column_text;

    typedef int(*_def_sqlite3_column_int)(sqlite3_stmt*, int);
    _def_sqlite3_column_int sqlite3_column_int;

    typedef int(*_def_sqlite3_column_count)(sqlite3_stmt*);
    _def_sqlite3_column_count sqlite3_column_count;

    typedef int(*_def_sqlite3_bind_int)(sqlite3_stmt*, int, int);
    _def_sqlite3_bind_int sqlite3_bind_int;

    typedef int(*_def_sqlite3_bind_text)(sqlite3_stmt*, int, const char*, int, void*);
    _def_sqlite3_bind_text sqlite3_bind_text;

#define SQLITE_INTEGER  1
#define SQLITE_FLOAT    2
#define SQLITE_BLOB     4
#define SQLITE_NULL     5
#ifdef SQLITE_TEXT
# undef SQLITE_TEXT
#else
# define SQLITE_TEXT     3
#endif
#define SQLITE3_TEXT     3

#define SQLITE_OK           0   /* Successful result */
#define SQLITE_ROW          100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE         101  /* sqlite3_step() has finished executing */
};
//-----------------------------------------------------------------------------
class MRDefPostgreSQL : public MRDefBase
{
public:
    MRDefPostgreSQL(const std::string& library_path)
        : MRDefBase(library_path)
    {
        PQLibVersion = Register<_def_pqlibversion>("PQlibVersion");
        PQping = Register<_def_pqping>("PQping");
        PQconnectdb = Register<_def_pqconnectdb>("PQconnectdb");
        PQstatus = Register<_def_pqstatus>("PQstatus");
        PQerrorMessage = Register<_def_pqerrormessage>("PQerrorMessage");
        PQfinish = Register<_def_pqfinish>("PQfinish");
        PQclear = Register<_def_pqclear>("PQclear");
        PQexec = Register<_def_pqexec>("PQexec");
        PQresultStatus = Register<_def_pqresultstatus>("PQresultStatus");
        PQnfields = Register<_def_pqnfields>("PQnfields");
        PQntuples = Register<_def_pqntuples>("PQntuples");
        PQgetvalue = Register<_def_pqgetvalue>("PQgetvalue");
        PQgetisnull = Register<_def_pqgetisnull>("PQgetisnull");
        PQexecParams = Register<_def_pqexecparams>("PQexecParams");
    };
    ~MRDefPostgreSQL() = default;

    enum class PGPing
    {
        PQPING_OK,
        PQPING_REJECT,
        PQPING_NO_RESPONSE,
        PQPING_NO_ATTEMPT
    };

    enum class ExecStatusType
    {
        PGRES_EMPTY_QUERY = 0,
        PGRES_COMMAND_OK,
        PGRES_TUPLES_OK,
        PGRES_COPY_OUT,
        PGRES_COPY_IN,
        PGRES_BAD_RESPONSE,
        PGRES_NONFATAL_ERROR,
        PGRES_FATAL_ERROR,
        PGRES_COPY_BOTH,
        PGRES_SINGLE_TUPLE
    };

    struct PGconn;
    struct PGresult;

    typedef int(*_def_pqlibversion)(void);
    _def_pqlibversion PQLibVersion;

    typedef PGPing(*_def_pqping)(const char*);
    _def_pqping PQping;

    typedef PGconn*(*_def_pqconnectdb)(const char*);
    _def_pqconnectdb PQconnectdb;

    typedef int(*_def_pqstatus)(const PGconn*);
    _def_pqstatus PQstatus;

    typedef char*(*_def_pqerrormessage)(const PGconn*);
    _def_pqerrormessage PQerrorMessage;

    typedef void(*_def_pqfinish)(PGconn*);
    _def_pqfinish PQfinish;

    typedef void(*_def_pqclear)(PGresult*);
    _def_pqclear PQclear;

    typedef PGresult* (*_def_pqexec)(PGconn*, const char*);
    _def_pqexec PQexec;

    typedef ExecStatusType(*_def_pqresultstatus)(const PGresult*);
    _def_pqresultstatus PQresultStatus;

    typedef int(*_def_pqntuples)(const PGresult*);
    _def_pqntuples PQntuples;

    typedef int(*_def_pqnfields)(const PGresult*);
    _def_pqnfields PQnfields;

    typedef char*(*_def_pqgetvalue)(const PGresult*, int, int);
    _def_pqgetvalue PQgetvalue;

    typedef int(*_def_pqgetisnull)(const PGresult*, int, int);
    _def_pqgetisnull PQgetisnull;

    typedef PGresult*(*_def_pqexecparams)(PGconn*, const char*, int, const unsigned int*, const char* const*, const int*, const int*, int);
    _def_pqexecparams PQexecParams;
};
//-----------------------------------------------------------------------------
class MRDefSingleton
{
public:
    static MRDefSingleton& Get()
    {
        static MRDefSingleton singleton;
        return singleton;
    }

    bool Init(IDatabase::DatabaseType database_type)
    {
        m_DatabaseType = database_type;
        bool res = false;

        switch (m_DatabaseType)
        {
        case IDatabase::DatabaseType::SQLite:
            m_LibraryPath = MRConfig::Instance().GetString("SQLite", "PathLibrary");
            res = Init<MRDefSQLite>();
            break;

        case IDatabase::DatabaseType::PostgreSQL:
            m_LibraryPath = MRConfig::Instance().GetString("PostgreSQL", "PathLibrary");
            res = Init<MRDefPostgreSQL>();
            break;

        default:
        { }
        }

        return res;
    }

    const std::string& GetErrorString() const
    {
        return m_ErrorString;
    }

    const std::string& GetLibraryPath() const
    {
        return m_LibraryPath;
    }

    MRDefSQLite* SQLite;
    MRDefPostgreSQL* PostgreSQL;

private:
    MRDefSingleton()
        : SQLite(nullptr),
        PostgreSQL(nullptr),
        m_Initialized(false),
        m_DatabaseType(IDatabase::DatabaseType::Unknown)
    {

    }

    ~MRDefSingleton()
    {
        if (SQLite)
        {
            delete SQLite;
            SQLite = nullptr;
        }

        if (PostgreSQL)
        {
            delete PostgreSQL;
            PostgreSQL = nullptr;
        }
    }

    template <typename T> struct type_false : std::false_type {};
    template <typename T>
    bool Init()
    {
        if (m_Initialized)
        {
            return true;
        }

        try
        {
            if constexpr (std::is_same_v<MRDefSQLite, T>)
            {
                SQLite = new MRDefSQLite(m_LibraryPath);
            }
            else if constexpr (std::is_same_v<MRDefPostgreSQL, T>)
            {
                PostgreSQL = new MRDefPostgreSQL(m_LibraryPath);
            }
            else
            {
                static_assert(type_false<T>::value, "Not support template type");
            }

            m_Initialized = true;
            return true;
        }
        catch (const MRException& e)
        {
            m_ErrorString = e.what();
        }
        return false;
    }

private:
    MRDefSingleton(const MRDefSingleton&) = delete;
    MRDefSingleton(MRDefSingleton&&) = delete;
    MRDefSingleton& operator=(const MRDefSingleton&) = delete;
    MRDefSingleton& operator=(MRDefSingleton&&) = delete;

private:
    bool m_Initialized;
    IDatabase::DatabaseType m_DatabaseType;
    std::string m_ErrorString;
    std::string m_LibraryPath;
};
//-----------------------------------------------------------------------------
