#include "StdAfx.h"
#include "argparse.hpp"
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    argparse::ArgumentParser args("Migrator", "1.0");
    args.add_argument("--resources").help("path to resources dir");
    args.add_argument("--database").help("path to database file");
    args.add_argument("--action").help("action that will be executed: create");

    try
    {
        args.parse_args(argc, argv);
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "Can't parse argmuents. " << e.what() << std::endl;
        std::cout << args << std::endl;
        return EXIT_FAILURE;
    }

    //Проверяем наличие параметра с ресурсами
    std::string resources_dir;
    if (!args.is_used("resources"))
    {
        std::cout << "You didn't specified resources dir" << std::endl;
        std::cout << args << std::endl;
        return EXIT_FAILURE;
    }
    resources_dir = args.get<std::string>("resources");

    //Проверяем, что директория существует
    if (!std::filesystem::exists(resources_dir))
    {
        std::cout << "Directory '" << resources_dir << "' isn't exist" << std::endl;
        return EXIT_FAILURE;
    }

    //И что это вообще директория, а не файл
    if (!std::filesystem::is_directory(resources_dir))
    {
        std::cout << "Path '" << resources_dir << "' isn't a directory, it's a file" << std::endl;
        return EXIT_FAILURE;
    }

    //И что в этой директории есть файлы
    if ((std::size_t)std::distance(std::filesystem::directory_iterator{ resources_dir }, std::filesystem::directory_iterator{}) == 0)
    {
        std::cout << "Directory '" << resources_dir << "' contains no files" << std::endl;
        return EXIT_FAILURE;
    }

    std::string database_path;
    if (!args.is_used("database"))
    {
        std::cout << "You didn't specified database path" << std::endl;
        std::cout << args << std::endl;
        return EXIT_FAILURE;
    }
    database_path = args.get<std::string>("database");

    std::string action;
    if (!args.is_used("action"))
    {
        std::cout << "You didn't specified action" << std::endl;
        std::cout << args << std::endl;
        return EXIT_FAILURE;
    }
    action = args.get<std::string>("action");

    if (action == "create")
    {
        //create sqlite db...
    }
    else
    {
        std::cout << "Action '" << action << "' isn't support" << std::endl;
        std::cout << args << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------
