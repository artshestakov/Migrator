#pragma once
//-----------------------------------------------------------------------------
#include "IDatabase.h"
//-----------------------------------------------------------------------------
//! Тип данных для работы с параметрами запросов
using ISVariant = std::variant<
    std::nullptr_t,
    int,
    std::string>;
//-----------------------------------------------------------------------------
class IQuery
{
public:
    IQuery(IDatabase* db, const std::string& sql = std::string());
    virtual ~IQuery();

    const std::string& GetErrorString() const;

    virtual void BindInt(int i) = 0;
    virtual void BindString(const std::string& s) = 0;

    virtual bool IsNull(int i) = 0;

    virtual void ClearStatement() = 0;
    virtual bool Execute(const std::string& sql = std::string()) = 0;
    virtual bool First() = 0;
    virtual bool Next() = 0;
    virtual int GetColumnCount() const = 0;

    virtual const char* ReadColumn(int index) = 0;
    virtual int ReadColumn_Int(int index) = 0;
    virtual bool ReadColumn_Bool(int index) = 0;

protected:
    IDatabase* m_IDB;
    std::string m_SQL;
    std::string m_ErrorString;
};
//-----------------------------------------------------------------------------
