#pragma once
//-----------------------------------------------------------------------------
#include "IQuery.h"
#include "MRDefDatabase.h"
//-----------------------------------------------------------------------------
class IQuerySQLite : public IQuery
{
public:
    IQuerySQLite(IDatabase* db, const std::string& sql = std::string());
    ~IQuerySQLite();

    void BindInt(int i) override;
    void BindString(const std::string& s) override;

    bool IsNull(int i) override;

    void ClearStatement() override;
    bool Execute(const std::string& sql = std::string()) override;
    bool First() override;
    bool Next() override;
    int GetColumnCount() const override;

    const char* ReadColumn(int index) override;
    int ReadColumn_Int(int index) override;
    bool ReadColumn_Bool(int index) override;

private:
    bool Prepare();
    bool FillParams();

private:
    MRDefSQLite::sqlite3* m_DB;
    MRDefSQLite::sqlite3_stmt* m_STMT;
    bool m_CanNext;
    bool m_NeedClear;
    int m_ColumnCount;
    int m_BindIterator;
    std::vector<ISVariant> m_Params;
};
//-----------------------------------------------------------------------------
