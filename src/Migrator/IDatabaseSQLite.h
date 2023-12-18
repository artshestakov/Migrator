#pragma once
//-----------------------------------------------------------------------------
#include "IDatabase.h"
#include "MRDefDatabase.h"
//-----------------------------------------------------------------------------
class IDatabaseSQLite : public IDatabase
{
    friend class IQuerySQLite;

public:
    IDatabaseSQLite(DatabaseType database_type);
    ~IDatabaseSQLite();

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

    void GetOldObjects(const std::vector<ShowOldType>& v, IDatabase::ShowOldStruct& s, std::optional<std::string>& error_string) override;

private:
    MRDefSQLite::sqlite3* GetPointer() const;

private:
    std::string m_Path;
    MRDefSQLite::sqlite3* m_DB;
};
//-----------------------------------------------------------------------------
