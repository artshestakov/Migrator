#pragma once
//-----------------------------------------------------------------------------
#include "IDatabase.h"
//-----------------------------------------------------------------------------
class MRDatabase
{
public:
    MRDatabase(IDatabase::DatabaseType database_type);
    ~MRDatabase();

    const std::string& GetErrorString() const;

    bool InitLibrary();
    const std::string& GetLibraryPath() const;

    std::string GetVersion() const;
    bool ExistsDB(bool& is_exists);
    bool Create();
    bool Connect();
    bool Disconnect();

    bool Execute(const TMetaExecute* meta_execute);

    bool TableExists(const TMetaTable* meta_table, bool& is_exists);
    bool TableCreate(const TMetaTable* meta_table);
    bool TableUpdate(const TMetaTable* meta_table);

    bool FieldExists(const TMetaField* meta_field, bool& is_exists);
    bool FieldCreate(const TMetaField* meta_field);

    bool IndexExists(const TMetaIndex* meta_index, bool& is_exists);
    bool IndexCreate(const TMetaIndex* meta_index);
    bool IndexUpdate(const TMetaIndex* meta_index);

    bool ForeignExists(const TMetaForeign* meta_foreign, bool& is_exists);
    bool ForeignCreate(const TMetaForeign* meta_foreign);
    bool ForeignUpdate(const TMetaForeign* meta_foreign);

    bool ViewExists(const TMetaView* meta_view, bool& is_exists);
    bool ViewCreate(const TMetaView* meta_view);
    bool ViewUpdate(const TMetaView* meta_view);

    void GetOldObjects(const std::vector<IDatabase::ShowOldType>& v, IDatabase::ShowOldStruct& s, std::optional<std::string>& error_string);

private:
    IDatabase* GetInterface() const; //Для поддержки дружественности с классом MRQuery

private:
    IDatabase* m_DB; //Указатель на интерфейс БД
};
//-----------------------------------------------------------------------------
