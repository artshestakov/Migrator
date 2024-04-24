#pragma once
//-----------------------------------------------------------------------------
#include "TMetaClass.h"
#include "MRUtils.h"
#include "tinyxml2.h"
#include "ISException.h"
#include "ISString.h"
//-----------------------------------------------------------------------------
namespace MRTemplate
{
    template <typename T> struct type_false : std::false_type {};
    template<typename T>
    void GetAttributeXML(
        T& dest_value,
        const std::string& attribute_name,
        const tinyxml2::XMLElement* xml_element,
        bool not_null = false,
        bool to_lower_value = true)
    {
        const tinyxml2::XMLAttribute* xml_attribute = xml_element->FindAttribute(attribute_name.c_str());

        if (xml_attribute) //Атрибут есть
        {
            if (strlen(xml_attribute->Value()) == 0 && not_null) //Он пустой, но не должен быть пустым - ругаемся
            {
                throw ISException(ISString::F("attribute '%s' is empty on line %d",
                    attribute_name.c_str(), xml_element->GetLineNum()));
            }
        }
        else //Атрибута нет
        {
            if (not_null) //Но он должен быть - ругаемся
            {
                throw ISException(ISString::F("attribute '%s' is not specified on line %d",
                    attribute_name.c_str(), xml_element->GetLineNum()));
            }
            else //Он не является обязатепльным - нам пофиг. Значение уже было проинициализированно по умолчанию
            {
                return;
            }
        }

        static auto check_type_assert = [&attribute_name, &xml_element](const tinyxml2::XMLAttribute* xml_attribute, bool ok)
        {
            //Ошибку (а именно поле ok) учитываем только тогда, когда у нас значение в поле есть и оно не пустое
            //пустые значения за ошибку не считаем, они будут инициализированны далее в структурах
            if (auto v = xml_attribute->Value(); v && strlen(v) > 0 && !ok)
            {
                throw ISException(ISString::F("attribute '%s' has invalid type on line %d",
                    attribute_name.c_str(), xml_element->GetLineNum()));
            }
        };

        if constexpr (std::is_same_v<std::string, T>)
        {
            dest_value = xml_attribute->Value();

            if (to_lower_value)
            {
                ISString::ToLower(dest_value);
            }
        }
        else if constexpr (std::is_same_v<int, T>)
        {
            int i = 0;
            check_type_assert(xml_attribute, xml_attribute->QueryIntValue(&i) == tinyxml2::XML_SUCCESS);
            dest_value = i;
        }
        else if constexpr (std::is_same_v<bool, T>)
        {
            bool b = false;
            check_type_assert(xml_attribute, xml_attribute->QueryBoolValue(&b) == tinyxml2::XML_SUCCESS);
            dest_value = b;
        }
        else
        {
            static_assert(type_false<T>::value, "Not support template type");
        }
    }

    template <typename T>
    bool ExistsObject(const std::vector<const T*>& v, const TMetaBase* object, const std::filesystem::path& file_path, const tinyxml2::XMLElement* xml_element, std::string& error_string)
    {
        for (const TMetaBase* meta_object : v)
        {
            if (meta_object->Name == object->Name)
            {
                error_string = ISString::F("duplicate object \"%s\" in file \"%s\" on line %d",
                    object->Name.c_str(), file_path.u8string().c_str(), xml_element->GetLineNum());
                return true;
            }
        }
        return false;
    }

    template <typename T>
    const T* GetObject(const std::vector<const T*>& vec, const std::string& object_name)
    {
        for (const T* obj : vec)
        {
            if (obj->Name == object_name)
            {
                return obj;
            }
        }
        return nullptr;
    }
}
//-----------------------------------------------------------------------------
