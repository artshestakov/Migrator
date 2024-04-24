#include "MRMetaData.h"
#include "MRTemplate.h"
#include "MRUtils.h"
#include "MRConstants.h"
#include "ISLogger.h"
#include "ISFileInfo.h"
#include "ISDir.h"
#include "ISString.h"
#include "ISHash.h"
//-----------------------------------------------------------------------------
MRMetaData::MRMetaData()
{

}
//-----------------------------------------------------------------------------
MRMetaData::~MRMetaData()
{
    ISAlgorithm::VectorDeletePtrs(m_Executes);
    ISAlgorithm::VectorDeletePtrs(m_Tables);
    ISAlgorithm::VectorDeletePtrs(m_Views);
}
//-----------------------------------------------------------------------------
MRMetaData& MRMetaData::Instance()
{
    static MRMetaData meta_data;
    return meta_data;
}
//-----------------------------------------------------------------------------
const std::string& MRMetaData::GetErrorString() const
{
    return m_ErrorString;
}
//-----------------------------------------------------------------------------
const std::vector<const TMetaExecute*>& MRMetaData::GetExecutes() const
{
    return m_Executes;
}
//-----------------------------------------------------------------------------
const std::vector<const TMetaTable*>& MRMetaData::GetTables() const
{
    return m_Tables;
}
//-----------------------------------------------------------------------------
const std::vector<const TMetaView*>& MRMetaData::GetViews() const
{
    return m_Views;
}
//-----------------------------------------------------------------------------
const std::vector<const TMetaIndex*>& MRMetaData::GetIndexes() const
{
    return m_Indexes;
}
//-----------------------------------------------------------------------------
const std::vector<const TMetaForeign*>& MRMetaData::GetForeigns() const
{
    return m_Foreigns;
}
//-----------------------------------------------------------------------------
std::optional<const TMetaTable*> MRMetaData::TakeMetaTable(const std::string& table_name)
{
    for (const TMetaTable* meta_table : m_Tables)
    {
        if (meta_table->Name == table_name)
        {
            return ISAlgorithm::VectorTake(m_Tables, meta_table);
        }
    }
    return nullptr;
}
//-----------------------------------------------------------------------------
const TMetaTable* MRMetaData::GetMetaTable(const std::string& table_name) const
{
    return MRTemplate::GetObject<TMetaTable>(m_Tables, table_name);
}
//-----------------------------------------------------------------------------
const TMetaField* MRMetaData::GetMetaField(const std::string& table_name, const std::string& field_name) const
{
    if (const TMetaTable* meta_table = MRTemplate::GetObject<TMetaTable>(m_Tables, table_name); meta_table)
    {
        if (const TMetaField* meta_field = MRTemplate::GetObject<TMetaField>(meta_table->Fields, field_name))
        {
            return meta_field;
        }
    }
    return nullptr;
}
//-----------------------------------------------------------------------------
const TMetaView* MRMetaData::GetMetaView(const std::string& view_name) const
{
    return MRTemplate::GetObject<TMetaView>(m_Views, view_name);
}
//-----------------------------------------------------------------------------
const TMetaIndex* MRMetaData::GetMetaIndex(const std::string& index_name) const
{
    return MRTemplate::GetObject<TMetaIndex>(m_Indexes, index_name);
}
//-----------------------------------------------------------------------------
const TMetaForeign* MRMetaData::GetMetaForeign(const std::string& foreign_name) const
{
    return MRTemplate::GetObject<TMetaForeign>(m_Foreigns, foreign_name);
}
//-----------------------------------------------------------------------------
bool MRMetaData::Init(const ISVectorString& path_list)
{
    std::vector<ISFileInfo> files;
    std::string err_msg;

    for (const std::string& path : path_list)
    {
        //Если нам дали путь к файлу, то его и будем инициализировать
        std::filesystem::path p(path);
        if (p.has_filename())
        {
            ISFileInfo fi;
            (void)ISFileInfo::MakeFileInfo(p, fi);
            files.emplace_back(fi);
        }
        else //Это директория - вытаскиваем файлы из неё
        {
            files = ISDir::Files(true, path, &err_msg,
                ISDir::DirFileSorting::Name, ISDir::SortingOrder::Ascending,
                { "SMD" });
        }

        if (files.empty() && !err_msg.empty())
        {
            //Что-то пошло не так
            m_ErrorString = ISString::F("cannot iterate path '%s': %s", path.c_str(), err_msg.c_str());
            return false;
        }

        int file_amount = 0;
        for (const ISFileInfo& file_info : files)
        {
            auto t = ISAlgorithm::GetTick();
            if (!InitFile(file_info.PathFull))
            {
                return false;
            }
            ISLOGGER_I(__CLASS__, "Init SMD-file \"%s\" success for %lld msec", file_info.PathFull.u8string().c_str(), ISAlgorithm::GetTickDiff(t));
            ++file_amount;
        }

        if (!file_amount)
        {
            m_ErrorString = ISString::F("directory has no smd-files or invalid path: %s", path.c_str());
            return false;
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
bool MRMetaData::InitFile(const std::filesystem::path& file_path)
{
    m_CurrentFilePath = file_path;

    //Читаем содержимое файла
    std::ifstream smd_file(m_CurrentFilePath);
    if (!smd_file.good())
    {
        m_ErrorString = ISString::F("cannot read file '%s': %s", m_CurrentFilePath.u8string().c_str(), ISAlgorithm::GetLastErrorS().c_str());
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(smd_file)), std::istreambuf_iterator<char>());
    smd_file.close();

    //Проверку подписи выполняем только в релизной версии
#ifndef DEBUG
    if (!CheckSign(content))
    {
        return false;
    }
#endif
    return InitContent(content.c_str(), content.size());
}
//-----------------------------------------------------------------------------
bool MRMetaData::InitContent(const char* content, size_t size)
{
    tinyxml2::XMLDocument xml_doc;
    if (xml_doc.Parse(content, size) != tinyxml2::XMLError::XML_SUCCESS)
    {
        m_ErrorString = ISString::F("cannot parse file '%s': %s", m_CurrentFilePath.u8string().c_str(), xml_doc.ErrorStr());
        return false;
    }

    //Получаем имя главного тэга
    tinyxml2::XMLElement* xml_element_main = xml_doc.FirstChildElement();
    std::string main_tag_name = std::string(xml_element_main->Name());
    if (main_tag_name != "SMD")
    {
        m_ErrorString = ISString::F("name of the main tag is not valid: %s", main_tag_name.c_str());
        return false;
    }

    //Спускаемся на уровень объектов (таблицы, представления и т.д.)
    tinyxml2::XMLElement* xml_object = xml_element_main->FirstChildElement();
    if (!xml_object)
    {
        m_ErrorString = ISString::F("file '%s' has no elements", m_CurrentFilePath.u8string().c_str());
        return false;
    }

    bool res = false;
    do
    {
        std::string object_name = xml_object->Name();
        if (object_name == "Execute")
        {
            res = InitExecute(xml_object);
        }
        else if (object_name == "Table")
        {
            res = InitTable(xml_object);
        }
        else if (object_name == "View")
        {
            res = InitView(xml_object);
        }
        else //Попался неизвестный тэг - уйдём с ошибкой
        {
            m_ErrorString = ISString::F("Tag \"%s\" is not support on file %d",
                object_name.c_str(), xml_object->GetLineNum());
        }

        if (!res)
        {
            return false;
        }
        xml_object = xml_object->NextSiblingElement();
    } while (xml_object);

    return CheckForeigns();
}
//-----------------------------------------------------------------------------
bool MRMetaData::InitExecute(tinyxml2::XMLElement* xml_execute)
{
    TMetaExecute* meta_execute = new TMetaExecute();
    try
    {
        MRTemplate::GetAttributeXML(meta_execute->Name, "Name", xml_execute, true);
        MRTemplate::GetAttributeXML(meta_execute->Comment, "Comment", xml_execute);
    }
    catch (const ISException& e)
    {
        m_ErrorString = ISString::F("cannot parse attribute. %s", e.what());
        return false;
    }

    if (MRTemplate::ExistsObject(m_Executes, meta_execute, m_CurrentFilePath, xml_execute, m_ErrorString))
    {
        return false;
    }
    meta_execute->SQL = xml_execute->GetText();

    m_Executes.emplace_back(meta_execute);
    return true;
}
//-----------------------------------------------------------------------------
bool MRMetaData::InitTable(tinyxml2::XMLElement* xml_table)
{
    TMetaTable* meta_table = new TMetaTable();
    try
    {
        MRTemplate::GetAttributeXML(meta_table->Name, "Name", xml_table, true);

        std::string tmp;
        MRTemplate::GetAttributeXML(tmp, "PrimaryField", xml_table);
        meta_table->PrimaryFields = ISString::Split(tmp, ';');
        std::sort(meta_table->PrimaryFields.begin(), meta_table->PrimaryFields.end());

        MRTemplate::GetAttributeXML(meta_table->Comment, "Comment", xml_table, false, false);
    }
    catch (const ISException& e)
    {
        m_ErrorString = ISString::F("cannot parse attribute. %s", e.what());
        return false;
    }

    if (MRTemplate::ExistsObject(m_Tables, meta_table, m_CurrentFilePath, xml_table, m_ErrorString))
    {
        return false;
    }

    //Спускаемся на уровень секций
    tinyxml2::XMLElement* xml_sections = xml_table->FirstChildElement();

    //И инициализируем поля таблицы
    if (auto a = FindSection(xml_sections, "Fields"); !a)
    {
        return false;
    }
    else if (!InitFields(meta_table, a))
    {
        return false;
    }

    //Инициализируем индексы. Если их нет - идём дальше
    if (auto a = FindSection(xml_sections, "Indexes"); !a)
    {
    }
    else if (!InitIndexes(meta_table, a))
    {
        return false;
    }

    //Инициализируем внешние ключи. Если их нет - идём дальше
    if (auto a = FindSection(xml_sections, "Foreigns"); !a)
    {
    }
    else if (!InitForeigns(meta_table, a))
    {
        return false;
    }

    m_Tables.emplace_back(meta_table);
    return true;
}
//-----------------------------------------------------------------------------
bool MRMetaData::InitView(tinyxml2::XMLElement* xml_view)
{
    TMetaView* meta_view = new TMetaView();
    try
    {
        MRTemplate::GetAttributeXML(meta_view->Name, "Name", xml_view, true);
        MRTemplate::GetAttributeXML(meta_view->Comment, "Comment", xml_view);
    }
    catch (const ISException& e)
    {
        m_ErrorString = ISString::F("cannot parse attribute. %s", e.what());
        return false;
    }

    if (MRTemplate::ExistsObject(m_Views, meta_view, m_CurrentFilePath, xml_view, m_ErrorString))
    {
        return false;
    }
    meta_view->SQL = xml_view->GetText();

    m_Views.emplace_back(meta_view);
    return true;
}
//-----------------------------------------------------------------------------
bool MRMetaData::InitFields(TMetaTable* meta_table, tinyxml2::XMLElement* xml_first_field)
{
    do
    {
        TMetaField* meta_field = new TMetaField(meta_table);
        try
        {
            MRTemplate::GetAttributeXML(meta_field->Name, "Name", xml_first_field, true);
            MRTemplate::GetAttributeXML(meta_field->Type, "Type", xml_first_field, true);
            MRTemplate::GetAttributeXML(meta_field->DefaultValue, "DefaultValue", xml_first_field, false, false);
            MRTemplate::GetAttributeXML(meta_field->NotNull, "NotNull", xml_first_field);
            MRTemplate::GetAttributeXML(meta_field->Comment, "Comment", xml_first_field, false, false);

            //Выставляем принудительно true только в том случае, когда поля является часть первичного ключа
            if (ISAlgorithm::VectorContains(meta_table->PrimaryFields, meta_field->Name))
            {
                meta_field->NotNull = true;
            }
        }
        catch (const ISException& e)
        {
            m_ErrorString = ISString::F("cannot parse attribute. %s", e.what());
            return false;
        }

        if (MRTemplate::ExistsObject(meta_table->Fields, meta_field, m_CurrentFilePath, xml_first_field, m_ErrorString))
        {
            return false;
        }
        meta_table->Fields.emplace_back(meta_field);
    } while ((xml_first_field = xml_first_field->NextSiblingElement()) != nullptr);

    return true;
}
//-----------------------------------------------------------------------------
bool MRMetaData::InitIndexes(TMetaTable* meta_table, tinyxml2::XMLElement* xml_first_index)
{
    do
    {
        TMetaIndex* meta_index = new TMetaIndex(meta_table->Name);
        try
        {
            MRTemplate::GetAttributeXML(meta_index->Name, "Name", xml_first_index, true);

            std::string tmp;
            MRTemplate::GetAttributeXML(tmp, "Fields", xml_first_index, true);
            meta_index->Fields = ISString::Split(tmp, ';');
            std::sort(meta_index->Fields.begin(), meta_index->Fields.end());

            MRTemplate::GetAttributeXML(meta_index->Unique, "Unique", xml_first_index, false);
            MRTemplate::GetAttributeXML(meta_index->Comment, "Comment", xml_first_index);
        }
        catch (const ISException& e)
        {
            m_ErrorString = ISString::F("cannot parse attribute in file %s: %s", m_CurrentFilePath.u8string().c_str(), e.what());
            return false;
        }

        //Проверяем, нет ли уже индекса с таким именем
        if (MRTemplate::ExistsObject(m_Indexes, meta_index, m_CurrentFilePath, xml_first_index, m_ErrorString))
        {
            return false;
        }

        //Проверим, что все поля индекса существуют
        //Алгоритм посчитает кол-во неправильных полей и если нашлось хотя бы одно - считаем за ошибку
        if (auto r = std::count_if(meta_index->Fields.begin(), meta_index->Fields.end(), [&meta_table](const std::string& field_name)
            {
                return !meta_table->GetField(field_name);
            }); r > 0)
        {
            m_ErrorString = ISString::F("Invalid field name in index \"%s\" of table \"%s\" in file %s",
                meta_index->Name.c_str(), meta_table->Name.c_str(), m_CurrentFilePath.u8string().c_str());
            return false;
        }

        meta_table->Indexes.emplace_back(meta_index);
        m_Indexes.emplace_back(meta_index);
    } while ((xml_first_index = xml_first_index->NextSiblingElement()) != nullptr);

    return true;
}
//-----------------------------------------------------------------------------
bool MRMetaData::InitForeigns(TMetaTable* meta_table, tinyxml2::XMLElement* xml_first_foreign)
{
    do
    {
        TMetaForeign* meta_foreign = new TMetaForeign(meta_table->Name);
        try
        {
            MRTemplate::GetAttributeXML(meta_foreign->Name, "Name", xml_first_foreign, true);
            MRTemplate::GetAttributeXML(meta_foreign->FieldName, "Field", xml_first_foreign, true);
            MRTemplate::GetAttributeXML(meta_foreign->ReferenceTable, "ReferenceTable", xml_first_foreign, true);
            MRTemplate::GetAttributeXML(meta_foreign->ReferenceField, "ReferenceField", xml_first_foreign, true);
            MRTemplate::GetAttributeXML(meta_foreign->Comment, "Comment", xml_first_foreign);
        }
        catch (const ISException& e)
        {
            m_ErrorString = ISString::F("cannot parse attribute in file %s: %s", m_CurrentFilePath.u8string().c_str(), e.what());
            return false;
        }

        //Проверяем, нет ли уже внешнего ключа с таким именем
        if (MRTemplate::ExistsObject(m_Foreigns, meta_foreign, m_CurrentFilePath, xml_first_foreign, m_ErrorString))
        {
            return false;
        }

        //Проверим, что внешний ключ будет установлен на существующем поле
        if (!meta_table->GetField(meta_foreign->FieldName))
        {
            m_ErrorString = ISString::F("Invalid field \"%s\" in the foreign \"%s\"",
                meta_foreign->FieldName.c_str(), meta_foreign->Name.c_str());
            return false;
        }

        meta_table->Foreigns.emplace_back(meta_foreign);
        m_Foreigns.emplace_back(meta_foreign);
    } while ((xml_first_foreign = xml_first_foreign->NextSiblingElement()) != nullptr);

    return true;
}
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* MRMetaData::FindSection(tinyxml2::XMLElement* xml_sections, const std::string& section_name)
{
    while (xml_sections)
    {
        if (strcmp(xml_sections->Name(), section_name.c_str()) == 0)
        {
            tinyxml2::XMLElement* first_child_element = xml_sections->FirstChildElement();
            if (first_child_element)
            {
                return first_child_element;
            }
            else
            {
                m_ErrorString = ISString::F("section '%s' is empty", section_name.c_str());
                return nullptr;
            }
        }
        xml_sections = xml_sections->NextSiblingElement();
    }
    return nullptr;
}
//-----------------------------------------------------------------------------
bool MRMetaData::CheckSign(const std::string& content)
{
    std::regex regexp(MRConstants::SIGNER_REGEXP);
    std::smatch match;
    if (!std::regex_search(content.cbegin(), content.cend(), match, regexp))
    {
        m_ErrorString = ISString::F("file \"%s\" is not signed", m_CurrentFilePath.u8string().c_str());
        return false;
    }

    size_t p = content.find('\n');
    if (p == std::string::npos)
    {
        m_ErrorString = ISString::F("invalid file \"%s\"", m_CurrentFilePath.u8string().c_str());
        return false;
    }

    std::string header = content.substr(0, p);
    ISString::RemoveAllChar(header, '<');
    ISString::RemoveAllChar(header, '!');
    ISString::RemoveAllChar(header, '-');
    ISString::RemoveAllChar(header, '>');

    //Проверяем подпись
    std::string sign_content = content.substr(p + 1, content.size() - p - 1);
    if (ISHash::StringToSHA(ISHash::SHAType::SHA512, sign_content) != header)
    {
        m_ErrorString = ISString::F("invalid sign of file \"%s\"", m_CurrentFilePath.u8string().c_str());
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool MRMetaData::CheckForeigns()
{
    for (const TMetaForeign* meta_foreign : m_Foreigns)
    {
        //Проверим, что внешний ключ ссылается на существующую таблицу
        if (!GetMetaTable(meta_foreign->ReferenceTable))
        {
            m_ErrorString = ISString::F("Invalid reference table \"%s\" in the foreign \"%s\"",
                meta_foreign->ReferenceTable.c_str(), meta_foreign->Name.c_str());
            return false;
        }

        //Проверим, что внешний ключ ссылается на существующее поле в таблице
        if (!GetMetaTable(meta_foreign->ReferenceTable)->GetField(meta_foreign->ReferenceField))
        {
            m_ErrorString = ISString::F("Invalid reference field \"%s\" in the foreign \"%s\"",
                meta_foreign->ReferenceField.c_str(), meta_foreign->Name.c_str());
            return false;
        }
    }

    return true;
}
//-----------------------------------------------------------------------------
