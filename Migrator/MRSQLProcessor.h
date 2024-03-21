#pragma once
//-----------------------------------------------------------------------------
#include "TMetaClass.h"
//-----------------------------------------------------------------------------
namespace MRSQLProcessor
{
    namespace SQLite
    {
        std::string TableCreate(const TMetaTable* meta_table);
        std::string FieldCreate(const TMetaField* meta_field);
        std::string IndexCreate(const TMetaIndex* meta_index);
        std::string ViewCreate(const TMetaView* meta_view);
    }

    namespace PostgreSQL
    {
        std::string TableCreate(const TMetaTable* meta_table);
        std::string TableComment(const TMetaTable* meta_table);
        std::string FieldComment(const TMetaField* meta_field);
        std::string FieldCreate(const TMetaField* meta_field);
        std::string FieldDefault(const TMetaField* meta_field);
        std::string IndexCreate(const TMetaIndex* meta_index);
        std::string IndexComment(const TMetaIndex* meta_index);
        std::string ViewCreate(const TMetaView* meta_view);
        std::string ViewComment(const TMetaView* meta_view);
        std::string ForeignCreate(const TMetaForeign* meta_foreign);
        std::string ForeignComment(const TMetaForeign* meta_foreign);
    }
};
//-----------------------------------------------------------------------------
