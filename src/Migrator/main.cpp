#include "StdAfx.h"
#include "MRConfig.h"
#include "MRUtils.h"
#include "MRMigrator.h"
#include "MRLogger.h"
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    //Инициализируем логгер
    if (!MRLogger::Instance().Init())
    {
        std::cout << MRLogger::Instance().GetErrorString() << std::endl;
        return EXIT_FAILURE;
    }

    //Установим кодировку
    std::string e;
    if (!MRUtils::InstallEncoding(65001, &e))
    {
        MR_LOG.Log("Cannot changed console encoding: %s", e.c_str());
    }

    argparse::ArgumentParser args("Migrator", "1.0");
    args.add_description("Migrator is a portable utility for upgrade database structure by SMD (Schema Meta Data) files.");
    args.add_epilog("If you have a quiestion, please contact the developer.\n"
        "  Author:   Shestakov Artem, Russia\n"
        "  Telegram: @artem_shestakov\n"
        "  Email:    art.shestakov@icloud.com");

    //Пингуем БД
    argparse::ArgumentParser ping_command("ping");
    ping_command.add_description("Ping to selected database in config file and exits");
    args.add_subparser(ping_command);

    //Валидируем мета-данных
    argparse::ArgumentParser validate_command("validate");
    validate_command.add_description("Validate meta data and exits");
    validate_command.add_argument("paths")
        .help("List of directories or files. You can specify multiple directory paths and multiple file paths here.")
        .remaining();
    args.add_subparser(validate_command);

    //Обновляем БД
    argparse::ArgumentParser update_command("update");
    update_command.add_description("Update database");
    update_command.add_argument("paths")
        .help("List of directories or files. You can specify multiple directory paths and multiple file paths here.")
        .remaining();
    args.add_subparser(update_command);

    //Проверяем, что библиотека для работы с БД успешно загружается
    argparse::ArgumentParser check_library_command("check-library");
    check_library_command.add_description("Performs a library load check for the current module");
    args.add_subparser(check_library_command);

    //Выводим содержимое конфигурационного файла на консоль
    argparse::ArgumentParser show_config_command("show-config");
    show_config_command.add_description("Shows the configuration file");
    args.add_subparser(show_config_command);

    //Показываем старые объекты
    argparse::ArgumentParser show_old_command("show-old");
    show_old_command.add_argument("paths")
        .help("List of directories or files. You can specify multiple directory paths and multiple file paths here.")
        .remaining();
    show_old_command.add_argument("--tables").help("Add tables to output").default_value(false).implicit_value(true);
    show_old_command.add_argument("--fields").help("Add fields to output").default_value(false).implicit_value(true);
    show_old_command.add_argument("--views").help("Add views to output").default_value(false).implicit_value(true);
    show_old_command.add_argument("--indexes").help("Add indexes to output").default_value(false).implicit_value(true);
    show_old_command.add_argument("--foreigns").help("Add foreign keys to output").default_value(false).implicit_value(true);
    show_old_command.add_description("Show old objects");
    args.add_subparser(show_old_command);

    //Инициализируем новый конфигурационный файл
    argparse::ArgumentParser init_config_command("init-config");
    init_config_command.add_argument("path")
        .help("The path for the new config file")
        .required();
    init_config_command.add_description("Initialize the new config file by the path");
    args.add_subparser(init_config_command);

    try
    {
        if (argc == 1)
        {
            throw std::runtime_error("No arguments specified");
        }
        args.parse_args(argc, argv);
    }
    catch (const std::runtime_error& e)
    {
        MR_LOG.Log("Cannot parse argmuents. %s", e.what());
        std::cout << std::endl << args;
        return EXIT_FAILURE;
    }

    //Инициализируем конфигурационный файл
    if (!MRUtils::InitConfig())
    {
        return EXIT_FAILURE;
    }

    if (args.is_subcommand_used("ping"))
    {
        return MRMigrator::Ping();
    }
    else if (args.is_subcommand_used("validate"))
    {
        auto a = MRUtils::ExtractPaths("validate", args);
        if (a.has_value())
        {
            return MRMigrator::Validate(a.value());
        }
    }
    else if (args.is_subcommand_used("update"))
    {
        auto a = MRUtils::ExtractPaths("update", args);
        if (a.has_value())
        {
            return MRMigrator::Update(a.value());
        }
    }
    else if (args.is_subcommand_used("check-library"))
    {
        return MRMigrator::CheckModule();
    }
    else if (args.is_subcommand_used("show-config"))
    {
        return MRMigrator::ShowConfig();
    }
    else if (args.is_subcommand_used("show-old"))
    {
        auto a = MRUtils::ExtractPaths("show-old", args);
        if (a.has_value())
        {
            return MRMigrator::ShowOld(a.value(), show_old_command);
        }
    }
    else if (args.is_subcommand_used("init-config"))
    {
        return MRMigrator::InitConfig(init_config_command.get<std::string>("path"));
    }

    return EXIT_FAILURE;
}
//-----------------------------------------------------------------------------
