#include "MRConfig.h"
#include "MRUtils.h"
#include "MRTemplate.h"
#include "MRConstants.h"
#include "MRFileSystem.h"
#include "MRString.h"
//-----------------------------------------------------------------------------
MRConfigParameterBase::MRConfigParameterBase(ParameterType type, const std::string& name)
    : Type(type),
    Name(name),
    NotNull(false)
{

}
//-----------------------------------------------------------------------------
MRConfigParameterBase::~MRConfigParameterBase()
{

}
//-----------------------------------------------------------------------------
MRConfigParameterBase::ParameterType MRConfigParameterBase::GetType() const
{
    return Type;
}
//-----------------------------------------------------------------------------
const std::string& MRConfigParameterBase::GetName() const
{
    return Name;
}
//-----------------------------------------------------------------------------
bool MRConfigParameterBase::GetNotNull() const
{
    return NotNull;
}
//-----------------------------------------------------------------------------
void MRConfigParameterBase::SetNotNull(bool not_null)
{
    NotNull = not_null;
}
//-----------------------------------------------------------------------------
const std::string& MRConfigParameterBase::GetComment() const
{
    return Comment;
}
//-----------------------------------------------------------------------------
void MRConfigParameterBase::SetComment(const std::string& comment)
{
    Comment = comment;
}
//-----------------------------------------------------------------------------
std::string MRConfigParameterBase::GetDefaultValue() const
{
    throw std::runtime_error("Error call GetDefaultValue()");
    return std::string();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
MRConfigString::MRConfigString(const std::string& name)
    : MRConfigParameterBase(MRConfigParameterBase::ParameterType::String, name)
{

}
//-----------------------------------------------------------------------------
MRConfigString::~MRConfigString()
{

}
//-----------------------------------------------------------------------------
std::string MRConfigString::GetDefaultValue() const
{
    return DefaultValue;
}
//-----------------------------------------------------------------------------
void MRConfigString::SetDefaultValue(const std::string& default_value)
{
    DefaultValue = default_value;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
MRConfigInt::MRConfigInt(const std::string& name)
    : MRConfigParameterBase(MRConfigParameterBase::ParameterType::Int, name),
    DefaultValue(0),
    Minimum(0),
    Maximum(0),
    Value(0)
{

}
//-----------------------------------------------------------------------------
MRConfigInt::~MRConfigInt()
{

}
//-----------------------------------------------------------------------------
std::string MRConfigInt::GetDefaultValue() const
{
    return std::to_string(DefaultValue);
}
//-----------------------------------------------------------------------------
void MRConfigInt::SetDefaultValue(int default_value)
{
    DefaultValue = default_value;
}
//-----------------------------------------------------------------------------
int MRConfigInt::GetMinimum() const
{
    return Minimum;
}
//-----------------------------------------------------------------------------
void MRConfigInt::SetMinimum(int minimum)
{
    Minimum = minimum;
}
//-----------------------------------------------------------------------------
int MRConfigInt::GetMaximum() const
{
    return Maximum;
}
//-----------------------------------------------------------------------------
void MRConfigInt::SetMaximum(int maximum)
{
    Maximum = maximum;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
MRConfigBool::MRConfigBool(const std::string& name)
    : MRConfigParameterBase(MRConfigParameterBase::ParameterType::Bool, name),
    DefaultValue(false),
    Value(false)
{

}
//-----------------------------------------------------------------------------
MRConfigBool::~MRConfigBool()
{

}
//-----------------------------------------------------------------------------
std::string MRConfigBool::GetDefaultValue() const
{
    return DefaultValue ? "true" : "false";
}
//-----------------------------------------------------------------------------
void MRConfigBool::SetDefaultValue(bool default_value)
{
    DefaultValue = default_value;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ISConfigSection::ISConfigSection(const std::string& name)
    : Name(name)
{

}
//-----------------------------------------------------------------------------
ISConfigSection::~ISConfigSection()
{
    for (MRConfigParameterBase* ParameterBase : Parameters)
    {
        delete ParameterBase;
    }
    Parameters.clear();
}
//-----------------------------------------------------------------------------
const std::string& ISConfigSection::GetName() const
{
    return Name;
}
//-----------------------------------------------------------------------------
const std::vector<MRConfigParameterBase*>& ISConfigSection::GetParameters() const
{
    return Parameters;
}
//-----------------------------------------------------------------------------
void ISConfigSection::AddString(const std::string& name, const std::string& default_value, bool not_null, const std::string& comment)
{
    MRConfigString* String = new MRConfigString(name);
    String->SetDefaultValue(default_value);
    String->SetNotNull(not_null);
    String->SetComment(comment);
    Parameters.emplace_back(String);
}
//-----------------------------------------------------------------------------
void ISConfigSection::AddInt(const std::string& name, int default_value, int minimum, int maximum, bool not_null, const std::string& comment)
{
    MRConfigInt* Int = new MRConfigInt(name);
    Int->SetDefaultValue(default_value);
    Int->SetMinimum(minimum);
    Int->SetMaximum(maximum);
    Int->SetNotNull(not_null);
    Int->SetComment(comment);
    Parameters.emplace_back(Int);
}
//-----------------------------------------------------------------------------
void ISConfigSection::AddBool(const std::string& name, bool default_value, bool not_null, const std::string& comment)
{
    MRConfigBool* Bool = new MRConfigBool(name);
    Bool->SetDefaultValue(default_value);
    Bool->SetNotNull(not_null);
    Bool->SetComment(comment);
    Parameters.emplace_back(Bool);
}
//-----------------------------------------------------------------------------
MRConfig::MRConfig()
    : ErrorString("No error"),
    m_IsFirstInit(false)
{
    CRITICAL_SECTION_INIT(&CriticalSection);
}
//-----------------------------------------------------------------------------
MRConfig::~MRConfig()
{
    CRITICAL_SECTION_DESTROY(&CriticalSection);

    for (ISConfigSection* Section : SectionList)
    {
        delete Section;
    }
    SectionList.clear();
}
//-----------------------------------------------------------------------------
MRConfig& MRConfig::Instance()
{
    static MRConfig Config;
    return Config;
}
//-----------------------------------------------------------------------------
const std::string& MRConfig::GetErrorString() const
{
    return ErrorString;
}
//-----------------------------------------------------------------------------
const std::string& MRConfig::GetFilePath() const
{
    return PathConfigFile;
}
//-----------------------------------------------------------------------------
bool MRConfig::GetIsFirstInit() const
{
    return m_IsFirstInit;
}
//-----------------------------------------------------------------------------
bool MRConfig::IsValid()
{
    //Проверяем параметры на заполняемость
    for (const ISConfigSection* Section : SectionList)
    {
        for (const MRConfigParameterBase* Parameter : Section->GetParameters())
        {
            std::string Value = GetValue(Section->GetName(), Parameter->GetName());

            //Если параметр обязателен для заполнения и он пустой - ошибка
            if (Parameter->GetNotNull() && Value.empty())
            {
                ErrorString = MRString::StringF("Parameter \"%s\\%s\" is empty", Section->GetName().c_str(), Parameter->GetName().c_str());
                return false;
            }
        }
    }

    //Проверяем корректность параметров
    for (const ISConfigSection* Section : SectionList)
    {
        for (MRConfigParameterBase* Parameter : Section->GetParameters())
        {
            std::string Value = GetValue(Section->GetName(), Parameter->GetName());

            switch (Parameter->GetType())
            {
            case MRConfigParameterBase::ParameterType::String:
                //Для строк ничего не делаем
                break;

            case MRConfigParameterBase::ParameterType::Int:
            {
                if (!MRString::StringIsNumber(Value))
                {
                    ErrorString = MRString::StringF("Invalid parameter value %s\\%s=%s",
                        Section->GetName().c_str(), Parameter->GetName().c_str(), Value.c_str());
                    return false;
                }

                int IntValue = 0;
                try
                {
                    IntValue = std::stoi(Value);
                }
                catch (const std::invalid_argument& e)
                {
                    ErrorString = MRString::StringF("Invalid argument \"%s\": %s", Value.c_str(), e.what());
                    return false;
                }

                //Проверяем вхождение значения в диапазон
                MRConfigInt* ParameterInt = dynamic_cast<MRConfigInt*>(Parameter);
                if (IntValue < ParameterInt->GetMinimum() || IntValue > ParameterInt->GetMaximum())
                {
                    ErrorString = MRString::StringF("Parameter %s\\%s=%s out of range [%d;%d]",
                        Section->GetName().c_str(), Parameter->GetName().c_str(), Value.c_str(),
                        ParameterInt->GetMinimum(), ParameterInt->GetMaximum());
                    return false;
                }
            }
            break;

            case MRConfigParameterBase::ParameterType::Bool:
            {
                MRString::StringToLower(Value);
                if (!(Value == "true" || Value == "false"))
                {
                    ErrorString = MRString::StringF("Invalid parameter value %s\\%s=%s (allower \"true\" or \"false\"",
                        Section->GetName().c_str(), Parameter->GetName().c_str(), Value.c_str());
                    return false;
                }
            }
            break;

            default: //Неизвестный тип
                ErrorString = MRString::StringF("Invalid parameter type %d from %s\\%s",
                    (int)Parameter->GetType(), Section->GetName().c_str(), Parameter->GetName().c_str());
                return false;
            }
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
bool MRConfig::IsEmpty(const std::string& SectionName, const std::string& ParameterName)
{
    return GetString(SectionName, ParameterName).empty();
}
//-----------------------------------------------------------------------------
bool MRConfig::Initialize(const std::string& file_name)
{
    PathConfigFile = MRUtils::GetApplicationDir() + MRConstants::PATH_SEPARATOR + file_name + ".ini";
    m_IsFirstInit = !MRFileSystem::FileExist(PathConfigFile);
    return MRFileSystem::FileExist(PathConfigFile) ? Update() : Create();
}
//-----------------------------------------------------------------------------
bool MRConfig::ReInitialize(const std::string& file_path)
{
    std::string temp = PathConfigFile;
    PathConfigFile = file_path;

    SimpleINI.Reset();
    bool res = Create();

    PathConfigFile = temp;
    SimpleINI.Reset();
    Update();

    return res;
}
//-----------------------------------------------------------------------------
bool MRConfig::Reload()
{
    MRDateTime DTWriteNew;
    if (!MRFileSystem::FileDateTimeWrite(PathConfigFile, DTWriteNew, &ErrorString))
    {
        ErrorString = MRString::StringF("Can't get write date time: %s", ErrorString.c_str());
        return false;
    }
    return DTWriteNew > DTWrite;
}
//-----------------------------------------------------------------------------
ISConfigSection* MRConfig::AddSection(const std::string& SectionName)
{
    ISConfigSection* Section = new ISConfigSection(SectionName);
    SectionList.emplace_back(Section);
    return Section;
}
//-----------------------------------------------------------------------------
std::string MRConfig::GetString(const std::string& SectionName, const std::string& ParameterName)
{
    return GetValue(SectionName, ParameterName);
}
//-----------------------------------------------------------------------------
const char* MRConfig::GetCString(const std::string& SectionName, const std::string& ParameterName)
{
    return SimpleINI.GetValue(SectionName.c_str(), ParameterName.c_str());
}
//-----------------------------------------------------------------------------
int MRConfig::GetInt(const std::string& SectionName, const std::string& ParameterName)
{
    std::string Value = GetValue(SectionName, ParameterName);

    int Result = 0;
    try
    {
        Result = std::stoi(Value);
    }
    catch (const std::invalid_argument& e)
    {
        (void)e;
    }
    return Result;
}
//-----------------------------------------------------------------------------
unsigned short MRConfig::GetUShort(const std::string& SectionName, const std::string& ParameterName)
{
    return static_cast<unsigned short>(GetInt(SectionName, ParameterName));
}
//-----------------------------------------------------------------------------
bool MRConfig::GetBool(const std::string& SectionName, const std::string& ParameterName)
{
    //Получаем значение, приводим его к нижнему регистру и проверяем
    std::string Value = GetValue(SectionName, ParameterName);
    MRString::StringToLower(Value);
    return Value == "true";
}
//-----------------------------------------------------------------------------
void MRConfig::SetString(const std::string& SectionName, const std::string& ParameterName, const std::string& Value)
{
    SetValue(SectionName, ParameterName, Value);
}
//-----------------------------------------------------------------------------
void MRConfig::SetUShort(const std::string& SectionName, const std::string& ParameterName, unsigned short Value)
{
    SetValue(SectionName, ParameterName, std::to_string(Value));
}
//-----------------------------------------------------------------------------
void MRConfig::SetBool(const std::string& SectionName, const std::string& ParameterName, bool Value)
{
    SetValue(SectionName, ParameterName, Value ? "true" : "false");
}
//-----------------------------------------------------------------------------
void MRConfig::SetInt(const std::string& SectionName, const std::string& ParameterName, int Value)
{
    SetValue(SectionName, ParameterName, std::to_string(Value));
}
//-----------------------------------------------------------------------------
void MRConfig::SetValue(const std::string& SectionName, const std::string& ParameterName, const std::string& Value)
{
    SimpleINI.SetValue(SectionName.c_str(), ParameterName.c_str(), Value.c_str(), nullptr, true);
    SimpleINI.SaveFile(PathConfigFile.c_str());
}
//-----------------------------------------------------------------------------
std::string MRConfig::GetValue(const std::string& SectionName, const std::string& ParameterName)
{
    std::string Value;
    CRITICAL_SECTION_LOCK(&CriticalSection);
    Value = SimpleINI.GetValue(SectionName.c_str(), ParameterName.c_str());
    CRITICAL_SECTION_UNLOCK(&CriticalSection);
    return Value;
}
//-----------------------------------------------------------------------------
bool MRConfig::Update()
{
    SI_Error LoadError = SimpleINI.LoadFile(PathConfigFile.c_str());
    if (LoadError != SI_Error::SI_OK)
    {
        ErrorString = SimpleErrorToString(LoadError);
        return false;
    }

    bool NeedSave = false; //Флаг необходимости сохранения
    ISVectorString TempKeys; //Временный список всех ключей
    CSimpleIni::TNamesDepend Sections, Keys;
    SimpleINI.GetAllSections(Sections);
    for (const CSimpleIni::Entry& Section : Sections) //Обходим секции в файле
    {
        //Проверим, есть ли такая секция в памяти. Если нет - удаляем
        if (!ContainsSection(Section.pItem))
        {
            SimpleINI.Delete(Section.pItem, nullptr);
            NeedSave = true;
        }

        SimpleINI.GetAllKeys(Section.pItem, Keys);
        for (const CSimpleIni::Entry& Key : Keys) //Обходим ключи
        {
            if (!ContainsKey(Section.pItem, Key.pItem)) //Если такого ключа в шаблоне нет - удаляем его из текущего файла
            {
                SimpleINI.Delete(Section.pItem, Key.pItem);
                NeedSave = true;
            }
            TempKeys.emplace_back(std::string(Section.pItem) + '/' + std::string(Key.pItem));
        }
    }

    //Теперь проверяем, не появились ли новые параметры
    for (const ISConfigSection* Section : SectionList) //Обходим секции в памяти
    {
        for (const MRConfigParameterBase* Parameter : Section->GetParameters()) //Обходим параметры
        {
            //Такого ключа в текущем конфигурационном файле нет - добавляем
            if (!MRContainer::VectorContains(TempKeys, Section->GetName() + '/' + Parameter->GetName()))
            {
                std::string DefaultValue = Parameter->GetDefaultValue();
                const char* comment = Parameter->GetComment().empty() ? nullptr : Parameter->GetComment().c_str();

                SI_Error Result = SimpleINI.SetValue(Section->GetName().c_str(),
                    Parameter->GetName().c_str(), DefaultValue.c_str(), comment);

                if (Result != SI_Error::SI_INSERTED)
                {
                    ErrorString = MRString::StringF("Not inserted value %s/%s=%s (error code %d)",
                        Section->GetName().c_str(), Parameter->GetName().c_str(), DefaultValue.c_str(), Result);
                    return false;
                }
                NeedSave = true;
            }
        }
    }

    if (NeedSave) //Требуется сохранение
    {
        SI_Error SaveCode = SimpleINI.SaveFile(PathConfigFile.c_str());
        if (SaveCode != SI_Error::SI_OK)
        {
            ErrorString = MRString::StringF("Error save file \"%s\": %s",
                PathConfigFile.c_str(), SimpleErrorToString(SaveCode).c_str());
            return false;
        }
    }

    //Получим дату последнего изменения файла
    if (!MRFileSystem::FileDateTimeWrite(PathConfigFile, DTWrite, &ErrorString))
    {
        ErrorString = MRString::StringF("Can't get write date time: %s", ErrorString.c_str());
        return false;
    }

    return true;
}
//-----------------------------------------------------------------------------
bool MRConfig::Create()
{
    //Обойдём все параметры
    for (const ISConfigSection* Section : SectionList)
    {
        for (const MRConfigParameterBase* Parameter : Section->GetParameters())
        {
            const char* comment = Parameter->GetComment().empty() ? nullptr : Parameter->GetComment().c_str();

            //Изменим значение
            SI_Error Result = SimpleINI.SetValue(Section->GetName().c_str(),
                Parameter->GetName().c_str(), Parameter->GetDefaultValue().c_str(), comment);
            if (Result != SI_Error::SI_INSERTED)
            {
                ErrorString = MRString::StringF("Not inserted value %s/%s=%s (error code %d)",
                    Section->GetName().c_str(), Parameter->GetName().c_str(), Parameter->GetDefaultValue().c_str(), Result);
                return false;
            }
        }
    }

    //Сохраним
    bool Result = SimpleINI.SaveFile(PathConfigFile.c_str()) == SI_Error::SI_OK;

    //Если сохранение прошло успешно - получим дату последнего изменения файла
    if (Result)
    {
        if (!MRFileSystem::FileDateTimeWrite(PathConfigFile, DTWrite, &ErrorString))
        {
            ErrorString = MRString::StringF("Can't get write date time: %s", ErrorString.c_str());
            return false;
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
bool MRConfig::ContainsSection(const std::string& SectionName)
{
    for (const ISConfigSection* Section : SectionList)
    {
        if (Section->GetName() == SectionName)
        {
            return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
bool MRConfig::ContainsKey(const std::string& SectionName, const std::string& ParameterName)
{
    for (const ISConfigSection* Section : SectionList) //Обходим секции
    {
        for (const MRConfigParameterBase* ParameterBase : Section->GetParameters()) //Обходим параметры
        {
            if (Section->GetName() == SectionName && ParameterBase->GetName() == ParameterName)
            {
                return true;
            }
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
std::string MRConfig::SimpleErrorToString(SI_Error ErrorCode)
{
    std::string String;
    switch (ErrorCode)
    {
    case SI_Error::SI_FAIL: String = "Generic failure"; break;
    case SI_Error::SI_NOMEM: String = "Out of memory error"; break;
    case SI_Error::SI_FILE: String = "File error (" + MRString::GetLastErrorS() + ")"; break;
    default:
        String = "Unknown error while update config file";
    }
    return String;
}
//-----------------------------------------------------------------------------
