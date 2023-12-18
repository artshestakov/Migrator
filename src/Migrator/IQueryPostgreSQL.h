#pragma once
//-----------------------------------------------------------------------------
#include "IQuery.h"
#include "MRDefDatabase.h"
//-----------------------------------------------------------------------------
class IQueryPostgreSQL : public IQuery
{
public:
    IQueryPostgreSQL(IDatabase* db, const std::string& sql = std::string());
    ~IQueryPostgreSQL();

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
    MRDefPostgreSQL::PGconn* m_DB;
    MRDefPostgreSQL::PGresult* m_Result;

    bool m_IsSelect;
    int m_CurrentRow;
    int m_Rows;
    int m_Columns;

    std::vector<char*> m_ParameterValues; //Параметры запроса
    std::vector<unsigned int> m_ParameterTypes; //Типы параметров запроса
    std::vector<int> m_ParameterFormats; //Форматы параметров
    std::vector<int> m_ParameterLengths;
    size_t m_ParameterCount; //Количество параметров
};
//-----------------------------------------------------------------------------
