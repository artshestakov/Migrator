#include "MRSQLProcessor.h"
#include "MRString.h"
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::SQLite::TableCreate(const TMetaTable* meta_table)
{
    std::string sql_text = MRString::StringF("CREATE TABLE \"%s\"\n(\n", meta_table->Name.c_str());

    for (const TMetaField* meta_field : meta_table->Fields)
    {
        sql_text += MRString::StringF("    \"%s\" %s",
            meta_field->Name.c_str(), meta_field->Type.c_str());

        if (!meta_field->DefaultValue.empty())
        {
            sql_text += MRString::StringF(" DEFAULT %s", meta_field->DefaultValue.c_str());
        }

        if (meta_field->NotNull)
        {
            sql_text += " NOT NULL";
        }
        sql_text += ",\n";
    }

    if (!meta_table->PrimaryFields.empty())
    {
        sql_text += "    PRIMARY KEY (";
        for (const std::string& primary_field : meta_table->PrimaryFields)
        {
            sql_text += primary_field + ", ";
        }
        MRString::StringChop(sql_text, 2);
        sql_text += ")\n);";
    }
    else
    {
        MRString::StringChop(sql_text, 2);
        sql_text += "\n);\n";
    }
    return sql_text;
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::SQLite::FieldCreate(const TMetaField* meta_field)
{
    std::string sql_text = MRString::StringF("ALTER TABLE %s ADD COLUMN %s %s",
        meta_field->MetaTable->Name.c_str(), meta_field->Name.c_str(), meta_field->Type.c_str());

    if (meta_field->NotNull)
    {
        sql_text += " NOT NULL";
    }

    if (!meta_field->DefaultValue.empty())
    {
        sql_text += MRString::StringF(" DEFAULT(%s)", meta_field->DefaultValue.c_str());
    }

    return sql_text + ';';
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::SQLite::IndexCreate(const TMetaIndex* meta_index)
{
    std::string sql_text = MRString::StringF("CREATE%s INDEX %s ON %s(",
        (meta_index->Unique ? " UNIQUE" : ""),
        meta_index->Name.c_str(), meta_index->TableName.c_str());

    for (const std::string& field_name : meta_index->Fields)
    {
        sql_text += field_name + ", ";
    }

    MRString::StringChop(sql_text, 2);
    sql_text += ");";
    return sql_text;
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::SQLite::ViewCreate(const TMetaView* meta_view)
{
    return MRString::StringF("CREATE VIEW \"%s\" AS\n%s",
        meta_view->Name.c_str(), meta_view->SQL.c_str());
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::TableCreate(const TMetaTable* meta_table)
{
    std::string sql_text = MRString::StringF("CREATE TABLE public.\"%s\"\n(\n", meta_table->Name.c_str());

    for (const TMetaField* meta_field : meta_table->Fields)
    {
        sql_text += MRString::StringF("    \"%s\" %s",
            meta_field->Name.c_str(), meta_field->Type.c_str());

        if (!meta_field->DefaultValue.empty())
        {
            sql_text += " DEFAULT " + meta_field->DefaultValue;
        }

        if (meta_field->NotNull)
        {
            sql_text += " NOT NULL";
        }
        sql_text += ",\n";
    }

    if (!meta_table->PrimaryFields.empty())
    {
        sql_text += "    PRIMARY KEY (" +
            MRString::StringJoin(meta_table->PrimaryFields, ", ") +
            ")\n);";
    }
    else
    {
        MRString::StringChop(sql_text, 2);
        sql_text += "\n);\n";
    }
    return sql_text;
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::TableComment(const TMetaTable* meta_table)
{
    return MRString::StringF("COMMENT ON TABLE \"%s\" IS '%s'",
        meta_table->Name.c_str(), meta_table->Comment.c_str());
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::FieldComment(const TMetaField* meta_field)
{
    return MRString::StringF("COMMENT ON COLUMN \"%s\".\"%s\" IS '%s'",
        meta_field->MetaTable->Name.c_str(), meta_field->Name.c_str(), meta_field->Comment.c_str());
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::FieldCreate(const TMetaField* meta_field)
{
    std::string sql_text = MRString::StringF("ALTER TABLE \"%s\" ADD COLUMN \"%s\" %s",
        meta_field->MetaTable->Name.c_str(), meta_field->Name.c_str(), meta_field->Type.c_str());

    if (!meta_field->DefaultValue.empty())
    {
        sql_text += " DEFAULT " + meta_field->DefaultValue;
    }

    if (meta_field->NotNull)
    {
        sql_text += " NOT NULL";
    }

    return sql_text + ';';
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::FieldDefault(const TMetaField* meta_field)
{
    std::string sql_text = MRString::StringF("ALTER TABLE \"%s\" ALTER COLUMN \"%s\"",
        meta_field->MetaTable->Name.c_str(), meta_field->Name.c_str());

    if (meta_field->DefaultValue.empty())
    {
        sql_text += " DROP DEFAULT";
    }
    else
    {
        sql_text += " SET DEFAULT " + meta_field->DefaultValue;
    }

    return sql_text;
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::IndexCreate(const TMetaIndex* meta_index)
{
    std::string sql_text = "CREATE";

    if (meta_index->Unique)
    {
        sql_text += " UNIQUE";
    }

    sql_text += MRString::StringF(" INDEX %s ON \"%s\" USING BTREE(",
        meta_index->Name.c_str(), meta_index->TableName.c_str());

    for (const std::string& field_name : meta_index->Fields)
    {
        sql_text += "\"" + field_name + "\", ";
    }
    MRString::StringChop(sql_text, 2);
    sql_text += ")";
    return sql_text;
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::IndexComment(const TMetaIndex* meta_index)
{
    return MRString::StringF("COMMENT ON INDEX \"%s\" IS '%s'",
        meta_index->Name.c_str(), meta_index->Comment.c_str());
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::ViewCreate(const TMetaView* meta_view)
{
    //Чтобы не дублировать код - переиспользуем функцию для SQLite. Синтаксис такой же
    return SQLite::ViewCreate(meta_view);
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::ViewComment(const TMetaView* meta_view)
{
    return MRString::StringF("COMMENT ON VIEW \"%s\" IS '%s'",
        meta_view->Name.c_str(), meta_view->Comment.c_str());
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::ForeignCreate(const TMetaForeign* meta_foreign)
{
    return MRString::StringF(
        "ALTER TABLE \"%s\"\n"
        "ADD CONSTRAINT \"%s\" FOREIGN KEY (\"%s\")\n"
        "REFERENCES \"%s\" (\"%s\")\n"
        "ON DELETE CASCADE\n"
        "ON UPDATE NO ACTION\n"
        "NOT DEFERRABLE",
        meta_foreign->TableName.c_str(),
        meta_foreign->Name.c_str(), meta_foreign->FieldName.c_str(),
        meta_foreign->ReferenceTable.c_str(), meta_foreign->ReferenceField.c_str());
}
//-----------------------------------------------------------------------------
std::string MRSQLProcessor::PostgreSQL::ForeignComment(const TMetaForeign* meta_foreign)
{
    return MRString::StringF("COMMENT ON CONSTRAINT \"%s\" ON \"%s\" IS '%s'",
        meta_foreign->Name.c_str(), meta_foreign->TableName.c_str(), meta_foreign->Comment.c_str());
}
//-----------------------------------------------------------------------------
