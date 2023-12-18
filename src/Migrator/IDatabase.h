#pragma once
//-----------------------------------------------------------------------------
#include "TMetaClass.h"
//-----------------------------------------------------------------------------
class IDatabase
{
public:
    enum class DatabaseType
    {
        Unknown,
        SQLite,
        PostgreSQL
    };

    enum class ShowOldType
    {
        Tables,
        Fields,
        Views,
        Indexes,
        Foreigns
    };

    struct ShowOldStruct
    {
        ISVectorString Tables;
        ISVectorString Fields;
        ISVectorString Views;
        ISVectorString Indexes;
        ISVectorString Foreigns;
    };

public:
    IDatabase(DatabaseType database_type);
    virtual ~IDatabase();

    const std::string& GetErrorString() const;
    IDatabase::DatabaseType GetDatabaseType() const;

    bool InitLibrary();
    const std::string& GetLibraryPath() const;

    virtual std::string GetVersion() const = 0;
    virtual bool ExistsDB(bool& is_exists) = 0;
    virtual bool Create() = 0;
    virtual bool Connect() = 0;
    virtual bool Disconnect() = 0;

    virtual bool Execute(const TMetaExecute* meta_execute) = 0;

    virtual bool TableExists(const TMetaTable* meta_table, bool& is_exists) = 0;
    virtual bool TableCreate(const TMetaTable* meta_table) = 0;
    virtual bool TableUpdate(const TMetaTable* meta_table) = 0;

    virtual bool FieldExists(const TMetaField* meta_field, bool& is_exists) = 0;
    virtual bool FieldCreate(const TMetaField* meta_field) = 0;

    virtual bool IndexExists(const TMetaIndex* meta_index, bool& is_exists) = 0;
    virtual bool IndexCreate(const TMetaIndex* meta_index) = 0;
    virtual bool IndexUpdate(const TMetaIndex* meta_index) = 0;

    virtual bool ForeignExists(const TMetaForeign* meta_foreign, bool& is_exists) = 0;
    virtual bool ForeignCreate(const TMetaForeign* meta_foreign) = 0;
    virtual bool ForeignUpdate(const TMetaForeign* meta_foreign) = 0;

    virtual bool ViewExists(const TMetaView* meta_view, bool& is_exists) = 0;
    virtual bool ViewCreate(const TMetaView* meta_view) = 0;
    virtual bool ViewUpdate(const TMetaView* meta_view) = 0;

    virtual void GetOldObjects(const std::vector<ShowOldType> &v, ShowOldStruct &s, std::optional<std::string>& error_string) = 0;

protected:
    std::string m_ErrorString;

private:
    DatabaseType m_DatabaseType;
};
//-----------------------------------------------------------------------------
