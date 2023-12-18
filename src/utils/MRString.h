#pragma once
//-----------------------------------------------------------------------------
#include "MRTypedefs.h"
//-----------------------------------------------------------------------------
namespace MRString
{
    //! Получить описание последней ошибки
    std::string GetLastErrorS();

    //! Разделить строку
    //! \param String строка
    //! \param Separator разделитель
    //! \return возвращает вектор разделЄнных строк
    ISVectorString StringSplit(const std::string& String, char Separator);

    //! Разделить строку
    //! \param String строка
    //! \param Separator разделитель
    //! \param Count количество строк на выходе
    //! \return возвращает вектор разделЄнных строк
    ISVectorString StringSplit(const std::string& String, char Separator, size_t& Count);

    //! Проверка строки на число
    //! \param String строка
    //! \return возвращает true если строка ¤вл¤етс¤ число, иначе - false
    bool StringIsNumber(const std::string& String);

    //! Приведение строки к нижнему регистру
    //! \param String строка
    void StringToLower(std::string& String);

    //! Приведение строки к нижнему регистру
    //! \param String строка
    //! \return возвращает строку в нижнем регистре
    std::string StringToLowerGet(const std::string& String);

    //! Получить N символов строки слева
    //! \param String строка
    //! \param N количество символов
    //! \return возвращает строку
    std::string StringLeft(const std::string& String, size_t N);

    //! Удалить N символов в конце строки
    //! \param String строка
    //! \param N количество символов
    void StringChop(std::string& String, size_t N = 1);

    //! Удалить все вхождения символа
    //! \param String строка
    //! \param Char удал¤емый символ
    void StringRemoveAllChar(std::string& String, char Char);

    //! Форматирование строки
    //! \param Format формат сообщени¤
    //! \param ... аргументы
    //! \return возвращает форматированную строку
    std::string StringF(const char* Format, ...);

    //! Форматирование строки (имплементация с аргументами)
    //! \param fmt формат сообщени¤
    //! \param args аргументы
    //! \return возвращает форматированную строку
    std::string StringFA(const char* fmt, va_list args);

    //! Соединить список строк в одну
    //! \param vec список строк
    //! \param sep разделитель строк
    //! \return возвращает готовую строку
    std::string StringJoin(const ISVectorString& vec, const std::string& sep = std::string());
}
//-----------------------------------------------------------------------------
