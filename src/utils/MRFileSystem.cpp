#include "MRFileSystem.h"
#include "MRString.h"
//-----------------------------------------------------------------------------
MRFileSystem::ISExtensionFilter::ISExtensionFilter()
    : ISVectorString()
{

}
//-----------------------------------------------------------------------------
MRFileSystem::ISExtensionFilter::ISExtensionFilter(const std::initializer_list<std::string>& init_list)
    : ISVectorString()
{
    reserve(init_list.size());
    for (const std::string& extension : init_list)
    {
        std::string tmp = extension;
        MRString::StringToLower(tmp);
        emplace_back(tmp);
    }
}
//-----------------------------------------------------------------------------
bool MRFileSystem::DirExist(const std::string& DirPath)
{
    return std::filesystem::exists(DirPath);
}
//-----------------------------------------------------------------------------
bool MRFileSystem::DirCreate(const std::string& DirPath, std::string* ErrorString)
{
    //Пытаемся создать директорию только в том случае, если директория не существует
    //В ином случае возвращаем true

    std::error_code ErrorCode;
    if (!DirExist(DirPath) &&
        !std::filesystem::create_directories(DirPath, ErrorCode))
    {
        if (ErrorString)
        {
            *ErrorString = ErrorCode.message();
        }
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
std::vector<MRFileSystem::ISFileInfo> MRFileSystem::DirFiles(bool IsRecursive, const std::string& DirPath, std::string* ErrorString,
    MRFileSystem::DirFileSorting SortType, MRFileSystem::SortingOrder SortOrder, const ISExtensionFilter& Extensions)
{
    std::vector<MRFileSystem::ISFileInfo> Vector;
    if (DirExist(DirPath))
    {
        //Лямбда-функция для вычитывания файлов из директории
        auto read_directory = [&Vector, &Extensions](auto It)
        {
            for (const auto& file_item : It)
            {
                if (file_item.is_directory())
                {
                    continue;
                }

                std::string extension = file_item.path().extension().u8string();
                MRString::StringRemoveAllChar(extension, '.');
                MRString::StringToLower(extension);

                //Учитываем фильтрацию: если расширение текущего файла не попадает в список доступных расширений - пропускаем
                if (!Extensions.empty()
                    /*&& !MRTemplate::VectorContains(Extensions, extension)*/) //???
                {
                    continue;
                }

                MRFileSystem::ISFileInfo fi;
                (void)MRFileSystem::MakeFileInfo(file_item, fi);
                Vector.emplace_back(std::move(fi));
            }
        };

        IsRecursive ?
            read_directory(std::filesystem::recursive_directory_iterator(DirPath)) :
            read_directory(std::filesystem::directory_iterator(DirPath));

        if (!Vector.empty()) //Если файлы есть - сортируем
        {
            switch (SortType)
            {
            case MRFileSystem::DirFileSorting::EditDate:
                std::sort(Vector.begin(), Vector.end(), [SortOrder](const MRFileSystem::ISFileInfo& FileInfo1, const MRFileSystem::ISFileInfo& FileInfo2)
                    {
                        return SortOrder == MRFileSystem::SortingOrder::Ascending ?
                            FileInfo1.DateTimeEdit < FileInfo2.DateTimeEdit :
                            FileInfo1.DateTimeEdit > FileInfo2.DateTimeEdit;
                    });
                break;

            case MRFileSystem::DirFileSorting::Size:
                std::sort(Vector.begin(), Vector.end(), [SortOrder](const MRFileSystem::ISFileInfo& FileInfo1, const MRFileSystem::ISFileInfo& FileInfo2)
                    {
                        return SortOrder == MRFileSystem::SortingOrder::Ascending ?
                            FileInfo1.Size < FileInfo2.Size :
                            FileInfo1.Size > FileInfo2.Size;
                    });
                break;

            case MRFileSystem::DirFileSorting::Name:
                std::sort(Vector.begin(), Vector.end(), [SortOrder](const MRFileSystem::ISFileInfo& FileInfo1, const MRFileSystem::ISFileInfo& FileInfo2)
                    {
                        //Учитываем регистр для правильной сортировки
                        std::string file_name_1 = MRString::StringToLowerGet(FileInfo1.Name);
                        std::string file_name_2 = MRString::StringToLowerGet(FileInfo2.Name);

                        return SortOrder == MRFileSystem::SortingOrder::Ascending ?
                            file_name_1 < file_name_2 :
                            file_name_1 > file_name_2;
                    });
                break;

            default: { } //Сортировка не указана
            }
        }
    }
    else
    {
        if (ErrorString)
        {
            *ErrorString = "dir is not exist";
        }
    }
    return Vector;
}
//-----------------------------------------------------------------------------
bool MRFileSystem::FileExist(const std::string& FilePath)
{
    return std::filesystem::exists(FilePath);
}
//-----------------------------------------------------------------------------
bool MRFileSystem::FileMove(const std::string& SourcePath, const std::string& DestPath, std::string* ErrorString)
{
    std::error_code ErrorCode;
    std::filesystem::rename(SourcePath, DestPath, ErrorCode);
    if (ErrorCode)
    {
        if (ErrorString)
        {
            *ErrorString = ErrorCode.message();
        }
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool MRFileSystem::FileDateTimeWrite(const std::string& FilePath, MRDateTime& DateTime, std::string* ErrorString)
{
    std::error_code ErrorCode;
    std::filesystem::file_time_type Time = std::filesystem::last_write_time(FilePath, ErrorCode);
    if (ErrorCode)
    {
        if (ErrorString)
        {
            *ErrorString = ErrorCode.message();
        }
        return false;
    }

    DateTime = MRDateTime::FromFileTimeType(Time);
    return true;
}
//-----------------------------------------------------------------------------
bool MRFileSystem::MakeFileInfo(const std::string& file_path, MRFileSystem::ISFileInfo& file_info, std::string* error_string)
{
    return std::move(MRFileSystem::MakeFileInfo(std::filesystem::directory_entry(std::filesystem::path(file_path)), file_info, error_string));
}
//-----------------------------------------------------------------------------
bool MRFileSystem::MakeFileInfo(const std::filesystem::path& p, MRFileSystem::ISFileInfo& file_info, std::string* error_string)
{
    return std::move(MRFileSystem::MakeFileInfo(std::filesystem::directory_entry(p), file_info, error_string));
}
//-----------------------------------------------------------------------------
bool MRFileSystem::MakeFileInfo(const std::filesystem::directory_entry& entry, MRFileSystem::ISFileInfo& file_info, std::string* error_string)
{
    try
    {
        std::string extension = entry.path().extension().u8string();
        MRString::StringRemoveAllChar(extension, '.');
        MRString::StringToLower(extension);

        file_info.Name = entry.path().filename().u8string();
        file_info.ShortName = entry.path().stem().u8string();
        file_info.Extension = extension;
        file_info.PathFull = entry.path().u8string();
        file_info.PathParent = entry.path().parent_path().u8string();
        file_info.Size = entry.file_size();
        file_info.DateTimeEdit = MRDateTime::FromFileTimeType(entry.last_write_time());
        return true;
    }
    catch (const std::exception& e)
    {
        if (error_string)
        {
            *error_string = e.what();
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
