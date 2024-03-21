#pragma once
//-----------------------------------------------------------------------------
#include "ISAlgorithm.h"
//-----------------------------------------------------------------------------
struct TMetaTable;
//-----------------------------------------------------------------------------
struct TMetaBase
{
    std::string Name;
    std::string Comment;
};
//-----------------------------------------------------------------------------
struct TMetaField : public TMetaBase
{
    TMetaField(const TMetaTable* meta_table) : TMetaBase(),
        MetaTable(meta_table),
        NotNull(false)
    { }

    const TMetaTable* MetaTable;

    std::string Type;
    std::string DefaultValue;
    bool NotNull;
};
//-----------------------------------------------------------------------------
struct TMetaIndex : public TMetaBase
{
    TMetaIndex(const std::string& table_name) : TMetaBase(),
        TableName(table_name),
        Unique(false)
    { }

    std::string TableName;
    ISVectorString Fields;
    bool Unique;
};
//-----------------------------------------------------------------------------
struct TMetaForeign : public TMetaBase
{
    TMetaForeign(const std::string& table_name) : TMetaBase(),
        TableName(table_name)
    { }

    std::string TableName;
    std::string FieldName;
    std::string ReferenceTable;
    std::string ReferenceField;
};
//-----------------------------------------------------------------------------
struct TMetaTable : public TMetaBase
{
    TMetaTable() : TMetaBase()
    {

    }

    ~TMetaTable()
    {
        ISAlgorithm::VectorDeletePtrs(Fields);
        ISAlgorithm::VectorDeletePtrs(Indexes);
    }

    const TMetaField* GetField(const std::string& field_name) const
    {
        for (const TMetaField* meta_field : Fields)
        {
            if (meta_field->Name == field_name)
            {
                return meta_field;
            }
        }
        return nullptr;
    }

    ISVectorString PrimaryFields;
    std::vector<const TMetaField*> Fields;
    std::vector<const TMetaIndex*> Indexes;
    std::vector<const TMetaForeign*> Foreigns;
};
//-----------------------------------------------------------------------------
struct TMetaView : public TMetaBase
{
    TMetaView() : TMetaBase()
    {

    }

    std::string SQL;
};
//-----------------------------------------------------------------------------
struct TMetaExecute : public TMetaBase
{
    TMetaExecute() : TMetaBase()
    {

    }

    std::string Comment;
    std::string SQL;
};
//-----------------------------------------------------------------------------
