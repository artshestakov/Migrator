#pragma once
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "tinyxml2.h"
#include "TMetaClass.h"
//-----------------------------------------------------------------------------
class MRMetaData
{
public:
    static MRMetaData& Instance();

    const std::string& GetErrorString() const;
    const std::vector<const TMetaExecute*>& GetExecutes() const;
    const std::vector<const TMetaTable*>& GetTables() const;
    const std::vector<const TMetaView*>& GetViews() const;
    const std::vector<const TMetaIndex*>& GetIndexes() const;
    const std::vector<const TMetaForeign*>& GetForeigns() const;
    std::optional<const TMetaTable*> TakeMetaTable(const std::string& table_name);
    const TMetaTable* GetMetaTable(const std::string& table_name) const;
    const TMetaField* GetMetaField(const std::string& table_name, const std::string& field_name) const;
    const TMetaView* GetMetaView(const std::string& view_name) const;
    const TMetaIndex* GetMetaIndex(const std::string& index_name) const;
    const TMetaForeign* GetMetaForeign(const std::string& foreign_name) const;

    bool Init(const ISVectorString& path_list);

private:
    bool InitFile(const std::filesystem::path& file_path);
    bool InitContent(const char* content, size_t size);
    bool InitExecute(tinyxml2::XMLElement* xml_execute);
    bool InitTable(tinyxml2::XMLElement* xml_table);
    bool InitView(tinyxml2::XMLElement* xml_view);
    bool InitFields(TMetaTable* meta_table, tinyxml2::XMLElement* xml_first_field);
    bool InitIndexes(TMetaTable* meta_table, tinyxml2::XMLElement* xml_first_index);
    bool InitForeigns(TMetaTable* meta_table, tinyxml2::XMLElement* xml_first_foreign);

private:
    tinyxml2::XMLElement* FindSection(tinyxml2::XMLElement* xml_sections, const std::string& section_name);
    bool CheckSign(const std::string &content);
    bool CheckForeigns();

private:
    MRMetaData();
    ~MRMetaData();
    MRMetaData(const MRMetaData&) = delete;
    MRMetaData(MRMetaData&&) = delete;
    MRMetaData& operator=(const MRMetaData&) = delete;
    MRMetaData& operator=(MRMetaData&&) = delete;

private:
    std::string m_ErrorString;
    std::filesystem::path m_CurrentFilePath;

    std::vector<const TMetaExecute*> m_Executes;
    std::vector<const TMetaTable*> m_Tables;
    std::vector<const TMetaView*> m_Views;
    std::vector<const TMetaIndex*> m_Indexes;
    std::vector<const TMetaForeign*> m_Foreigns;
};
//-----------------------------------------------------------------------------
