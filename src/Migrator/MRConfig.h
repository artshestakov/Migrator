#pragma once
//-----------------------------------------------------------------------------
#include "SimpleIni.h"
#include "MRTypedefs.h"
#include "MRDateTime.h"
//-----------------------------------------------------------------------------
struct MRConfigParameterBase
{
    enum class ParameterType
    {
        Unknown,
        String,
        Int,
        Bool
    };

    MRConfigParameterBase(ParameterType type, const std::string& name = std::string());
    virtual ~MRConfigParameterBase();

    ParameterType GetType() const;
    const std::string& GetName() const;

    bool GetNotNull() const;
    void SetNotNull(bool not_null);

    const std::string& GetComment() const;
    void SetComment(const std::string& comment);

    virtual std::string GetDefaultValue() const = 0;

private:
    ParameterType Type; //Тип параметра
    std::string Name; //имя параметра
    bool NotNull; //Флаг обязательности заполнения
    std::string Comment; //Комментарий параметра
};
//-----------------------------------------------------------------------------
struct MRConfigString : public MRConfigParameterBase
{
    MRConfigString(const std::string& name = std::string());
    virtual ~MRConfigString();

    std::string GetDefaultValue() const override;
    void SetDefaultValue(const std::string& default_value);

private:
    std::string DefaultValue;
    std::string Value;
};
//-----------------------------------------------------------------------------
struct MRConfigInt : public MRConfigParameterBase
{
    MRConfigInt(const std::string& name = std::string());
    virtual ~MRConfigInt();

    std::string GetDefaultValue() const override;
    void SetDefaultValue(int default_value);

    int GetMinimum() const;
    void SetMinimum(int minimum);

    int GetMaximum() const;
    void SetMaximum(int maximum);

private:
    int DefaultValue;
    int Minimum;
    int Maximum;
    int Value;
};
//-----------------------------------------------------------------------------
struct MRConfigBool : public MRConfigParameterBase
{
    MRConfigBool(const std::string& name = std::string());
    virtual ~MRConfigBool();

    std::string GetDefaultValue() const override;
    void SetDefaultValue(bool default_value);

private:
    bool DefaultValue;
    bool Value;
};
//-----------------------------------------------------------------------------
struct ISConfigSection
{
    ISConfigSection(const std::string& name = std::string());
    ~ISConfigSection();

    const std::string& GetName() const;
    const std::vector<MRConfigParameterBase*>& GetParameters() const;

    void AddString(const std::string& name, const std::string& default_value, bool not_null = false, const std::string& comment = std::string());
    void AddInt(const std::string& name, int default_value, int minimum, int maximum, bool not_null = false, const std::string& comment = std::string());
    void AddBool(const std::string& name, bool default_value, bool not_null = false, const std::string& comment = std::string());

private:
    std::string Name;
    std::vector<MRConfigParameterBase*> Parameters;
};
//-----------------------------------------------------------------------------
class MRConfig
{
public:
    static MRConfig& Instance();

    const std::string& GetErrorString() const;
    const std::string& GetFilePath() const;
    bool IsValid(); //Проверить корректность заполнения конфигурационного файла
    bool IsEmpty(const std::string& SectionName, const std::string& ParameterName);
    bool Initialize(const std::string& file_name);
    bool ReInitialize(const std::string& file_path);
    bool Reload(); //Проверка наобходимости перечитать файл

    ISConfigSection* AddSection(const std::string& SectionName);

    //Функции получения значений
    std::string GetString(const std::string& SectionName, const std::string& ParameterName);
    const char* GetCString(const std::string& SectionName, const std::string& ParameterName);
    int GetInt(const std::string& SectionName, const std::string& ParameterName);
    unsigned short GetUShort(const std::string& SectionName, const std::string& ParameterName);
    bool GetBool(const std::string& SectionName, const std::string& ParameterName);

    //Функции изменения значений
    void SetString(const std::string& SectionName, const std::string& ParameterName, const std::string& Value);
    void SetUShort(const std::string& SectionName, const std::string& ParameterName, unsigned short Value);
    void SetBool(const std::string& SectionName, const std::string& ParameterName, bool Value);
    void SetInt(const std::string& SectionName, const std::string& ParameterName, int Value);

private:
    void SetValue(const std::string& SectionName, const std::string& ParameterName, const std::string& Value);
    std::string GetValue(const std::string& SectionName, const std::string& ParameterName); //Получить значение параметра
    bool Update(); //Обновление файла
    bool Create(); //Генерация файла из шаблона

private:
    bool ContainsSection(const std::string& SectionName);
    bool ContainsKey(const std::string& SectionName, const std::string& ParameterName);
    std::string SimpleErrorToString(SI_Error ErrorCode);

private:
    MRConfig();
    ~MRConfig();
    MRConfig(const MRConfig&) = delete;
    MRConfig(MRConfig&&) = delete;
    MRConfig& operator=(const MRConfig&) = delete;
    MRConfig& operator=(MRConfig&&) = delete;

private:
    std::vector<ISConfigSection*> SectionList;
    std::string ErrorString;
    std::string PathConfigFile;
    CSimpleIni SimpleINI;
    ISCriticalSection CriticalSection;
    MRDateTime DTWrite; //Дата и время последней записи файла
};
//-----------------------------------------------------------------------------
