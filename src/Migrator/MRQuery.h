#pragma once
//-----------------------------------------------------------------------------
#include "IQuery.h"
#include "MRDatabase.h"
//-----------------------------------------------------------------------------
class MRQuery
{
public:
    MRQuery(IDatabase* db, const std::string& sql = std::string());
    ~MRQuery();

    const std::string& GetErrorString() const;

    void BindInt(int i);
    void BindString(const std::string& s);

    bool IsNull(int i);

    void ClearStatement();
    bool Execute(const std::string& sql = std::string());
    bool First();
    bool Next();
    int GetColumnCount() const;

    const char* ReadColumn(int index);
    int ReadColumn_Int(int index);
    bool ReadColumn_Bool(int index);

private:
    IQuery* m_Query;
};
//-----------------------------------------------------------------------------
