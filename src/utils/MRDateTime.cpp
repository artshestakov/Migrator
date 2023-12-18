#include "MRDateTime.h"
#include "MRConstants.h"
#include "MRString.h"
//-----------------------------------------------------------------------------
inline constexpr size_t DAY_IN_SECONDS = 86400;
inline constexpr size_t MAX_VALID_YEAR = 9999; //Максимально допустимый год
inline constexpr size_t MIN_VALID_YEAR = 1800; //Минимально допустимый год
//-----------------------------------------------------------------------------
MRDate::MRDate()
    : Day(0), Month(0), Year(0)
{

}
//-----------------------------------------------------------------------------
MRDate::MRDate(unsigned short day, unsigned short month, unsigned short year)
    : Day(day), Month(month), Year(year)
{

}
//-----------------------------------------------------------------------------
MRDate MRDate::CurrentDate()
{
    return MRDateTime::CurrentDateTime().Date;
}
//-----------------------------------------------------------------------------
MRDate MRDate::FromString(const char* String, const char* Format)
{
    int Day = 0, Month = 0, Year = 0;
    (void)sscanf(String, Format, &Day, &Month, &Year);
    return MRDate((unsigned short)Day, (unsigned short)Month, (unsigned short)Year);
}
//-----------------------------------------------------------------------------
unsigned int MRDate::DaysInMonth(MRDate::MonthType month, unsigned int year)
{
    unsigned int Days = 0;
    switch (month)
    {
    case MRDate::MonthType::January: Days = 31; break;
    case MRDate::MonthType::February: Days = year % 4 == 0 ? 29 : 28; break;
    case MRDate::MonthType::March: Days = 31; break;
    case MRDate::MonthType::April: Days = 30; break;
    case MRDate::MonthType::May: Days = 31; break;
    case MRDate::MonthType::June: Days = 30; break;
    case MRDate::MonthType::July: Days = 31; break;
    case MRDate::MonthType::August: Days = 31; break;
    case MRDate::MonthType::September: Days = 30; break;
    case MRDate::MonthType::October: Days = 31; break;
    case MRDate::MonthType::November: Days = 30; break;
    case MRDate::MonthType::December: Days = 31; break;
    default:
        Days = 0;
    }
    return Days;
}
//-----------------------------------------------------------------------------
bool MRDate::IsNull() const
{
    return Day == 0 && Month == 0 && Year == 0;
}
//-----------------------------------------------------------------------------
std::string MRDate::ToString() const
{
    return MRString::StringF("%02d.%02d.%04d", Day, Month, Year);
}
//-----------------------------------------------------------------------------
MRDate MRDate::AddDays(int Days)
{
    //Если дни не указаны или дата пустая - возвращаем пустую дату
    if (Days == 0 || IsNull())
    {
        return MRDate();
    }

    //Получаем текущую дату и модернизируем её
    time_t Now = time(NULL);
    tm* TM = localtime(&Now);
    TM->tm_mday = Day;
    TM->tm_mon = Month - 1;
    TM->tm_year = Year - 1900;
    time_t Time = mktime(TM);

    //Проверяем, нужно добавлять дни или отнимать
    bool IsAdd = Days > 0;
    if (!IsAdd) //Если нужно отнимать - меняем знак
    {
        Days = -Days;
    }

    for (int i = 0; i < Days; ++i)
    {
        IsAdd ? Time += (int)DAY_IN_SECONDS : Time -= (int)DAY_IN_SECONDS;
    }
    return MRDateTime::FromUnixTime(Time).Date;
}
//-----------------------------------------------------------------------------
bool MRDate::IsValid() const
{
    if (Year > MAX_VALID_YEAR || Year < MIN_VALID_YEAR)
    {
        return false;
    }

    if (Month < 1 || Month > 12)
    {
        return false;
    }

    if (Day < 1 || Day > 31)
    {
        return false;
    }

    if (Month == 2)
    {
        if (IsLeap())
        {
            return (Day <= 29);
        }
        else
        {
            return (Day <= 28);
        }
    }

    if (Month == 4 || Month == 6 || Month == 9 || Month == 11)
    {
        return (Day <= 30);
    }

    return true;
}
//-----------------------------------------------------------------------------
unsigned int MRDate::DayOfWeek() const
{
    static const int Days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    int D = Day;
    for (int i = 0; i < Month - 1; i++)
    {
        D += Days[i];
    }

    if (((Year % 4 == 0 && Year % 100 != 0) || Year % 400 == 0) && Month > 2)
    {
        D++;
    }

    for (int i = 1971; i < Year; i++)
    {
        if ((i % 4 == 0 && i % 100 != 0) || i % 400 == 0)
        {
            D += 366;
        }
        else
        {
            D += 365;
        }
    }

    return (D % 7 + 3) % 7 + 1;
}
//-----------------------------------------------------------------------------
bool MRDate::operator<(const MRDate& Date) const
{
    if (Year < Date.Year)
    {
        return true;
    }
    else if (Year == Date.Year)
    {
        if (Month < Date.Month)
        {
            return true;
        }
        else if (Month == Date.Month)
        {
            if (Day < Date.Day)
            {
                return true;
            }
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
bool MRDate::operator>(const MRDate& Date) const
{
    if (Year > Date.Year)
    {
        return true;
    }
    else if (Year == Date.Year)
    {
        if (Month > Date.Month)
        {
            return true;
        }
        else if (Month == Date.Month)
        {
            if (Day > Date.Day)
            {
                return true;
            }
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
bool MRDate::operator==(const MRDate& Date) const
{
    return Day == Date.Day && Month == Date.Month && Year == Date.Year;
}
//-----------------------------------------------------------------------------
bool MRDate::IsLeap() const
{
    return (((Year % 4 == 0) && (Year % 100 != 0)) || (Year % 400 == 0));
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
MRTime::MRTime()
    : Hour(0), Minute(0), Second(0), Milliseconds(0)
{

}
//-----------------------------------------------------------------------------
MRTime::MRTime(unsigned short hour, unsigned short minute, unsigned short second, unsigned short milliseconds)
    : Hour(hour), Minute(minute), Second(second), Milliseconds(milliseconds)
{

}
//-----------------------------------------------------------------------------
MRTime MRTime::CurrentTime()
{
    return MRDateTime::CurrentDateTime().Time;
}
//-----------------------------------------------------------------------------
MRTime MRTime::FromSecond(uint64_t Seconds)
{
    uint64_t Hour = Seconds / 3600,
        Minute = (Seconds - Hour * 3600) / 60,
        Second = int(Seconds % 60);
    return MRTime((unsigned short)Hour, (unsigned short)Minute, (unsigned short)Second);
}
//-----------------------------------------------------------------------------
MRTime MRTime::FromString(const char* String, const char* Format)
{
    int Hour = 0, Minute = 0, Second = 0;
    (void)sscanf(String, Format, &Hour, &Minute, &Second);
    return MRTime((unsigned short)Hour, (unsigned short)Minute, (unsigned short)Second);
}
//-----------------------------------------------------------------------------
bool MRTime::IsNull() const
{
    return Hour == 0 && Minute == 0 && Second == 0 && Milliseconds == 0;
}
//-----------------------------------------------------------------------------
std::string MRTime::ToString() const
{
    return MRString::StringF("%02d:%02d:%02d", Hour, Minute, Second);
}
//-----------------------------------------------------------------------------
size_t MRTime::ToMSec() const
{
    return (Hour * 3600000) + (Minute * 60000) + (Second * 1000) + Milliseconds;
}
//-----------------------------------------------------------------------------
bool MRTime::operator<(const MRTime& Time) const
{
    size_t t1 = ToMSec(), t2 = Time.ToMSec();
    if (t1 == t2)
    {
        return false;
    }
    return t1 < t2;
}
//-----------------------------------------------------------------------------
bool MRTime::operator>(const MRTime& Time) const
{
    size_t t1 = ToMSec(), t2 = Time.ToMSec();
    if (t1 == t2)
    {
        return false;
    }
    return t1 > t2;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
MRDateTime::MRDateTime()
{

}
//-----------------------------------------------------------------------------
MRDateTime::MRDateTime(const MRDate& date, const MRTime& time)
    : Date(date), Time(time)
{

}
//-----------------------------------------------------------------------------
MRDateTime::MRDateTime(unsigned short day, unsigned short month, unsigned short year, unsigned short hour, unsigned short minute, unsigned short second, unsigned short milliseconds)
    : MRDateTime(MRDate(day, month, year), MRTime(hour, minute, second, milliseconds))
{

}
//-----------------------------------------------------------------------------
MRDateTime MRDateTime::CurrentDateTime()
{
#ifdef WIN32
    SYSTEMTIME ST;
    GetLocalTime(&ST);
    return{ { ST.wDay, ST.wMonth, ST.wYear },
        { ST.wHour, ST.wMinute, ST.wSecond, ST.wMilliseconds } };
#else
    struct timeval TimeValue;
    gettimeofday(&TimeValue, NULL);
    struct tm* ST = localtime(&TimeValue.tv_sec);
    return { { (unsigned short)ST->tm_mday, (unsigned short)(ST->tm_mon + 1), (unsigned short)(ST->tm_year + 1900) },
        { (unsigned short)ST->tm_hour, (unsigned short)ST->tm_min, (unsigned short)ST->tm_sec, (unsigned short)(TimeValue.tv_usec / 1000) } };
#endif
}
//-----------------------------------------------------------------------------
MRDateTime MRDateTime::FromUnixTime(uint64_t UnixTime)
{
    time_t Time = UnixTime;
    struct tm* TM = localtime(&Time);
    return MRDateTime((unsigned short)TM->tm_mday, (unsigned short)TM->tm_mon + 1, (unsigned short)TM->tm_year + 1900,
        (unsigned short)TM->tm_hour, (unsigned short)TM->tm_min, (unsigned short)TM->tm_sec);
}
//-----------------------------------------------------------------------------
MRDateTime MRDateTime::FromFileTimeType(const std::filesystem::file_time_type& file_time_type)
{
    auto Auto = std::chrono::time_point_cast<std::chrono::system_clock::duration>
        (file_time_type - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    return MRDateTime::FromUnixTime(std::chrono::system_clock::to_time_t(Auto));
}
//-----------------------------------------------------------------------------
bool MRDateTime::IsNull() const
{
    return Date.IsNull() && Time.IsNull();
}
//-----------------------------------------------------------------------------
std::string MRDateTime::ToString() const
{
    std::string Result;
    if (!IsNull())
    {
        Result = Date.ToString() + ' ' + Time.ToString();
    }
    return Result;
}
//-----------------------------------------------------------------------------
uint64_t MRDateTime::ToUTC()
{
    time_t RawTime = 0;
    if (!IsNull())
    {
        struct tm T;
        memset(&T, 0, sizeof(tm));

        T.tm_mday = Date.Day;
        T.tm_mon = Date.Month - 1;
        T.tm_year = Date.Year - 1900;
        T.tm_hour = Time.Hour - 3;
        T.tm_min = Time.Minute;
        T.tm_sec = Time.Second;
        RawTime = mktime(&T);
        RawTime += (RawTime - mktime(gmtime(&RawTime)));
    }
    return RawTime;
}
//-----------------------------------------------------------------------------
bool MRDateTime::operator<(const MRDateTime& DateTime) const
{
    if (Date < DateTime.Date)
    {
        return true;
    }
    else if (Date == DateTime.Date)
    {
        if (Time < DateTime.Time)
        {
            return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
bool MRDateTime::operator>(const MRDateTime& DateTime) const
{
    if (Date > DateTime.Date)
    {
        return true;
    }
    else if (Date == DateTime.Date)
    {
        if (Time > DateTime.Time)
        {
            return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
