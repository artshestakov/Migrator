#pragma once
//-----------------------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------------------
struct MRDate
{
    enum class MonthType
    {
        Unknown,
        January,
        February,
        March,
        April,
        May,
        June,
        July,
        August,
        September,
        October,
        November,
        December
    };

    unsigned short Day;
    unsigned short Month;
    unsigned short Year;

    MRDate();
    MRDate(unsigned short day, unsigned short month, unsigned short year);

    static MRDate CurrentDate(); //Получить текущую дату
    static MRDate FromString(const char* String, const char* Format);
    static unsigned int DaysInMonth(MRDate::MonthType month, unsigned int year);

    bool IsNull() const; //Проверка даты на пустоту
    std::string ToString() const; //Привести дату к строке
    MRDate AddDays(int Days); //Добавить или отнять дни у даты
    bool IsValid() const; //Получить валидность даты
    unsigned int DayOfWeek() const; //Получить номер дня недели

    bool operator<(const MRDate& Date) const;
    bool operator>(const MRDate& Date) const;
    bool operator==(const MRDate& Date) const;

private:
    bool IsLeap() const;
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct MRTime
{
    unsigned short Hour;
    unsigned short Minute;
    unsigned short Second;
    unsigned short Milliseconds;

    MRTime();
    MRTime(unsigned short hour, unsigned short minute, unsigned short second = 0, unsigned short milliseconds = 0);

    static MRTime CurrentTime(); //Получить текущее время
    static MRTime FromSecond(uint64_t Seconds); //Перевод секунд во время
    static MRTime FromString(const char* String, const char* Format);

    bool IsNull() const; //Проверка времени на пустоту
    std::string ToString() const; //Привести время в строку
    size_t ToMSec() const; //Привести время к миллисекундам

    bool operator<(const MRTime& Time) const;
    bool operator>(const MRTime& Time) const;
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct MRDateTime
{
    MRDate Date;
    MRTime Time;

    MRDateTime();
    MRDateTime(const MRDate& date, const MRTime& time);
    MRDateTime(unsigned short day, unsigned short month, unsigned short year, unsigned short hour, unsigned short minute, unsigned short second = 0, unsigned short milliseconds = 0);

    static MRDateTime CurrentDateTime(); //Получить текущую дату и время

    //! //Конвертировать UnixTime в дату и время
    //! \param UnixTime метка времени
    //! \param Hour сдвиг часов. Может быть положительным или отрицательным числом. Например, если указано +3, то при конвертации будет сдвич часов вперед на три часа
    //! \return возвращает объект даты и времени
    static MRDateTime FromUnixTime(uint64_t UnixTime);

    static MRDateTime FromFileTimeType(const std::filesystem::file_time_type& file_time_type);

    bool IsNull() const; //Проверка даты и времени на пустоту
    std::string ToString() const; //Привести дату и время к строке
    uint64_t ToUTC(); //Преобразовать в метку времени

    bool operator<(const MRDateTime& DateTime) const;
    bool operator>(const MRDateTime& DateTime) const;
};
//-----------------------------------------------------------------------------
