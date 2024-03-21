#include "MRConstants.h"
#include "ISFileInfo.h"
#include "ISAlgorithm.h"
#include <argparse.hpp>
#include "tinyxml2.h"
//-----------------------------------------------------------------------------
std::string error_string;
//-----------------------------------------------------------------------------
bool GetFileList(const ISVectorString &path_list, std::vector<ISFileInfo> &files);
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    argparse::ArgumentParser args("Signer", "");
    args.add_argument("files")
        .help("Path to SMD-file. You can specify multiple paths, including directories")
        .nargs(argparse::nargs_pattern::any);
    try
    {
        args.parse_args(argc, argv);
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        std::cout << args << std::endl;
        return EXIT_FAILURE;
    }

    ISVectorString path_list = args.get<ISVectorString>("files");
    std::vector<ISFileInfo> files;
    if (!GetFileList(path_list, files))
    {
        return EXIT_FAILURE;
    }

    for (const ISFileInfo& file_info : files)
    {
        std::cout << "Signing file \"" << file_info.PathFull << "\"...";

        //Читаем файл
        std::ifstream file_in(file_info.PathFull);
        if (!file_in.is_open())
        {
            std::cout << std::endl << ISAlgorithm::GetLastErrorS() << std::endl;
            return EXIT_FAILURE;
        }

        std::string content((std::istreambuf_iterator<char>(file_in)), std::istreambuf_iterator<char>());
        file_in.close();

        //Проверим, что файл является XML-файлом
        tinyxml2::XMLDocument xml_doc;
        if (xml_doc.Parse(content.c_str(), content.size()) != tinyxml2::XMLError::XML_SUCCESS)
        {
            std::cout << std::endl << "File is not xml-file" << std::endl;
            return EXIT_FAILURE;
        }

        //Вытаскиваем первую строку
        std::string first_line;
        size_t p = content.find('\n');
        if (p == std::string::npos)
        {
            std::cout << std::endl << "Invalid file" << std::endl;
            return EXIT_FAILURE;
        }
        first_line = content.substr(0, p);

        std::regex regexp(MRConstants::SIGNER_REGEXP);
        std::smatch match;
        std::string sign;

        //Если нашли подпись
        if (std::regex_search(first_line.cbegin(), first_line.cend(), match, regexp))
        {
            //Вытаскиваем подписываемую часть
            content = content.substr(p + 1, content.size() - p - 1);
            sign = ISAlgorithm::StringToSHA(ISAlgorithm::SHAType::SHA512, content);
        }
        else //Подпись не нашли - подписываем
        {
            sign = ISAlgorithm::StringToSHA(ISAlgorithm::SHAType::SHA512, content);
        }

        std::string tmp_path = file_info.PathFull + ".tmp";
        std::ofstream file_out(tmp_path);
        if (!file_out.is_open())
        {
            std::cout << std::endl << ISAlgorithm::GetLastErrorS() << std::endl;
            return EXIT_FAILURE;
        }

        file_out << ISAlgorithm::StringF("<!--%s-->", sign.c_str()) << std::endl << content;
        file_out.close();

        //Переименовываем файл в исходный (с заменой)
        if (!ISAlgorithm::FileMove(tmp_path, file_info.PathFull, &error_string))
        {
            std::cout << ISAlgorithm::StringF("Can't rename file from \"%s\" to \"%s\": %s",
                tmp_path.c_str(), file_info.PathFull.c_str(), error_string.c_str()) << std::endl;
            return EXIT_FAILURE;
        }
        std::cout << " OK" << std::endl;
    }
    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
bool GetFileList(const ISVectorString& path_list, std::vector<ISFileInfo>& files)
{
    if (path_list.empty())
    {
        std::cout << "Files is not specified" << std::endl;
        return false;
    }

    std::error_code ec;

    //Вытаскиваем пути к указанным файлам
    for (const std::string& path : path_list)
    {
        bool is_dir = std::filesystem::is_directory(path, ec);
        if (ec)
        {
            std::cout << ISAlgorithm::StringF("Can't check path \"%s\": %s",
                path.c_str(), ec.message().c_str()) << std::endl;
            return false;
        }

        if (is_dir) //Если путь ведёт к директории - вытаскиваем все из неё
        {
            auto l = ISAlgorithm::DirFiles(true, path, &error_string,
                ISAlgorithm::DirFileSorting::DoNotSort, ISAlgorithm::SortingOrder::Ascending, { "smd" });
            if (l.empty())
            {
                std::cout << ISAlgorithm::StringF("Can't read directory \"%s\": %s", path.c_str(), error_string.c_str()) << std::endl;
                return false;
            }
            files.insert(files.end(), l.begin(), l.end());
        }
        else //Путь ведёт к файлу
        {
            ISFileInfo fi;
            (void)ISFileInfo::MakeFileInfo(path, fi);
            files.emplace_back(fi);
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
