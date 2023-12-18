#pragma once
//-----------------------------------------------------------------------------
#include "MRDateTime.h"
#include "MRTypedefs.h"
//-----------------------------------------------------------------------------
namespace MRFileSystem
{
    struct ISFileInfo
    {
        ISFileInfo()
            : Size(0) { }

        std::string Name; //Имя файла
        std::string ShortName; //Имя файла без расширения
        std::string Extension; //Расширение файла
        std::string PathFull; //Путь к файлу
        std::string PathParent; //Путь к родительской папке
        MRDateTime DateTimeEdit; //Дата изменения файла
        uintmax_t Size; //Размер файла
    };

    class ISExtensionFilter : public ISVectorString //Класс для приведения фильтра расширений к нижнему регистру
    {
    public:
        ISExtensionFilter();
        ISExtensionFilter(const std::initializer_list<std::string>& init_list);
    };

    //Тип сортировки файлов
    enum class DirFileSorting
    {
        DoNotSort, //Не сортировать
        EditDate, //По дате изменения
        Size, //По размеру
        Name //По имени
    };

    //Направление сортировки
    enum class SortingOrder
    {
        Ascending,
        Descending
    };

    //! Проверка существования папки
    //! \param DirPath путь к папке
    //! \return возвращает true в случае существования папки, иначе - false
    bool DirExist(const std::string& DirPath);

    //! Создание папки рекусивно
    //! \param DirPath путь к папке
    //! \param ErrorString указатель на строку с ошибкой. Если указатель передан, то строка будет заполнена в случае ошибки
    //! \return возвращает true в случае успешного создания папки, иначе - false
    bool DirCreate(const std::string& DirPath, std::string* ErrorString = nullptr);

    //! Получить список файлов в папке
    //! \param DirPath путь к папке
    //! \param ErrorString указатель на строку с ошибкой. Если указатель передан, то строка будет заполнена в случае ошибки
    //! \return возвращает список имён файлов. Если список пустой - произошла ошибка
    std::vector<ISFileInfo> DirFiles(bool IsRecursive, const std::string& DirPath, std::string* ErrorString = nullptr,
        MRFileSystem::DirFileSorting SortType = MRFileSystem::DirFileSorting::DoNotSort,
        MRFileSystem::SortingOrder SortOrder = MRFileSystem::SortingOrder::Ascending,
        const ISExtensionFilter& Extensions = ISExtensionFilter());

    //! Проверка существования файла
    //! \param FilePath путь к файлу
    //! \return возвращает true в случае существования файла, иначе - false
    bool FileExist(const std::string& FilePath);

    //! Перемещение файла
    //! \param SourcePath исходный путь к файлу
    //! \param DestPath конечный путь к файлу
    //! \param ErrorString указатель на строку с ошибкой. Если указатель передан, то строка будет заполнена в случае ошибки
    //! \return возвращает true в случае успешного перемещения, иначе - false
    bool FileMove(const std::string& SourcePath, const std::string& DestPath, std::string* ErrorString);

    //! Получить дату изменения файла
    //! \param FilePath путь к файлу
    //! \param DateTime дата и время изменения
    //! \param ErrorString указатель на строку с ошибкой. Если указатель передан, то строка будет заполнена в случае ошибки
    //! \return возвращает true в случае успешного получения даты и времени, иначе - false
    bool FileDateTimeWrite(const std::string& FilePath, MRDateTime& DateTime, std::string* ErrorString = nullptr);

    //! Получить информацию о файле
    //! \param file_path путь к файлу
    //! \return возвращает структуру с информацией о файле
    bool MakeFileInfo(const std::string& file_path, ISFileInfo& file_info, std::string* error_string = nullptr);

    //! Получить информацию о файле
    //! \param file_path путь к файлу
    //! \return возвращает структуру с информацией о файле
    bool MakeFileInfo(const std::filesystem::path& p, ISFileInfo& file_info, std::string* error_string = nullptr);

    //! Получить информацию о файле
    //! \param file_path путь к файлу
    //! \return возвращает структуру с информацией о файле
    bool MakeFileInfo(const std::filesystem::directory_entry& entry, ISFileInfo& file_info, std::string* error_string = nullptr);
}
//-----------------------------------------------------------------------------
