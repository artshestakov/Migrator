#pragma once
//-----------------------------------------------------------------------------
#include "IDatabase.h"
#include "MRDefDatabase.h"
//-----------------------------------------------------------------------------
class IDatabasePostgreSQL : public IDatabase
{
    friend class IQueryPostgreSQL;

public:
    IDatabasePostgreSQL(DatabaseType database_type);
    ~IDatabasePostgreSQL();

    std::string GetVersion() const override;
    bool ExistsDB(bool& is_exists) override;
    bool Create() override;
    bool Connect() override;
    bool Disconnect() override;

    bool Execute(const TMetaExecute* meta_execute) override;

    bool TableExists(const TMetaTable* meta_table, bool& is_exists) override;
    bool TableCreate(const TMetaTable* meta_table) override;
    bool TableUpdate(const TMetaTable* meta_table) override;

    bool FieldExists(const TMetaField* meta_field, bool& is_exists) override;
    bool FieldCreate(const TMetaField* meta_field) override;

    bool IndexExists(const TMetaIndex* meta_index, bool& is_exists) override;
    bool IndexCreate(const TMetaIndex* meta_index) override;
    bool IndexUpdate(const TMetaIndex* meta_index) override;

    bool ForeignExists(const TMetaForeign* meta_foreign, bool& is_exists) override;
    bool ForeignCreate(const TMetaForeign* meta_foreign) override;
    bool ForeignUpdate(const TMetaForeign* meta_foreign) override;

    bool ViewExists(const TMetaView* meta_view, bool& is_exists) override;
    bool ViewCreate(const TMetaView* meta_view) override;
    bool ViewUpdate(const TMetaView* meta_view) override;

    void GetOldObjects(const std::vector<ShowOldType>& v, IDatabase::ShowOldStruct& s, std::optional<std::string> &error_string) override;

private:
    MRDefPostgreSQL::PGconn* GetPointer() const;

private:
    std::string m_Host;
    int m_Port;
    std::string m_DatabaseName;
    std::string m_User;
    std::string m_Password;
    std::string m_SystemDBName;

    bool m_UseSystemDB;

    MRDefPostgreSQL::PGconn* m_DB;
};
//-----------------------------------------------------------------------------
