#include "MRString.h"
#include "MRConstants.h"
//-----------------------------------------------------------------------------
std::string MRString::GetLastErrorS()
{
#ifdef WIN32
    auto error_number = GetLastError();
#else
    auto error_number = errno;
#endif

    std::string ErrorString;
#ifdef WIN32
    if (error_number != 0) //Код ошибки валиден
    {
        LPSTR Buffer = nullptr;
        size_t MessageSize = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error_number, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&Buffer, 0, NULL);
        if (MessageSize > 0 && Buffer)
        {
            ErrorString = std::string(Buffer, MessageSize - 2);
            LocalFree(Buffer);
        }
    }
#else
    ErrorString = strerror(error_number);
#endif
    return ErrorString;
}
//-----------------------------------------------------------------------------
ISVectorString MRString::StringSplit(const std::string& String, char Separator)
{
    size_t Count = 0;
    return StringSplit(String, Separator, Count);
}
//-----------------------------------------------------------------------------
ISVectorString MRString::StringSplit(const std::string& String, char Separator, size_t& Count)
{
    Count = 0;

    ISVectorString VectorString;
    if (!String.empty()) //Если строка не пустая - анализируем
    {
        size_t Pos = 0, LastPos = 0;
        while ((Pos = String.find(Separator, LastPos)) != MRConstants::NPOS)
        {
            if (Pos != 0)
            {
                if ((Pos - LastPos) > 0)
                {
                    VectorString.emplace_back(String.substr(LastPos, Pos - LastPos));
                    ++Count;
                }
                LastPos = ++Pos;
            }
            else
            {
                ++LastPos;
            }
        }

        if (Pos == MRConstants::NPOS)
        {
            size_t Size = String.size();
            if (LastPos < Size)
            {
                VectorString.emplace_back(String.substr(LastPos, Size - LastPos));
                ++Count;
            }
        }
    }
    return VectorString;
}
//-----------------------------------------------------------------------------
bool MRString::StringIsNumber(const std::string& String)
{
    for (size_t i = 0, c = String.size(); i < c; ++i)
    {
        if (!std::isdigit(String[i]))
        {
            return false;
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
void MRString::StringToLower(std::string& String)
{
    for (size_t i = 0, c = String.size(); i < c; ++i)
    {
        String[i] = std::move((char)std::tolower((int)String[i]));
    }
}
//-----------------------------------------------------------------------------
std::string MRString::StringToLowerGet(const std::string& String)
{
    std::string Temp = String;
    StringToLower(Temp);
    return Temp;
}
//-----------------------------------------------------------------------------
std::string MRString::StringLeft(const std::string& String, size_t N)
{
    return String.substr(0, N);
}
//-----------------------------------------------------------------------------
void MRString::StringChop(std::string& String, size_t N)
{
    //Получаем размер строки
    size_t Size = String.size();
    if (Size == 0) //Если строка пустая - выходим
    {
        return;
    }

    if (N >= Size) //Если кол-во символов больше либо равно размеру строки - очищаем её
    {
        String.clear();
    }
    else //Вырезаем N символов с конце строки
    {
        String.erase(Size - N);
    }
}
//-----------------------------------------------------------------------------
void MRString::StringRemoveAllChar(std::string& String, char Char)
{
    String.erase(std::remove(String.begin(), String.end(), Char), String.end());
}
//-----------------------------------------------------------------------------
std::string MRString::StringF(const char* Format, ...)
{
    va_list Arguments;
    va_start(Arguments, Format);
    std::string res = MRString::StringFA(Format, Arguments);
    va_end(Arguments);

    return res;
}
//-----------------------------------------------------------------------------
std::string MRString::StringFA(const char* fmt, va_list args)
{
    static ISVectorChar Buffer;
    if (Buffer.empty())
    {
        Buffer.resize(2048);
    }
    static size_t BufferSize = 0, BufferSizeNew = 0;

    while (true)
    {
        //Делаем копию аргументов заранее
        va_list args_copy;
        va_copy(args_copy, args);

        //Форматируем
        BufferSize = Buffer.size();
        int Writed = std::vsnprintf(Buffer.data(), BufferSize, fmt, args_copy);
        va_end(args_copy);

        if ((Writed >= 0) && (Writed < (int)BufferSize)) //Форматирование удалось
        {
            std::string res(Buffer.data());
            Buffer.assign(BufferSize, 0);
            return res;
        }

        //Если форматировании что-то пошло не так и std::vsnprintf вернула ошибку (-1) - увеличиваем размер буфера вдвое
        //В ином случае - делаем переразмер на +1
        BufferSizeNew = Writed < 0 ? (BufferSizeNew * 2) : (Writed + 1);
        Buffer.resize(BufferSizeNew);
    }
}
//-----------------------------------------------------------------------------
std::string MRString::StringJoin(const ISVectorString& vec, const std::string& sep)
{
    if (vec.empty())
    {
        return std::string();
    }

    //Реализация с двойным std::move оказалась самой быстрой
    std::string res;
    for (const std::string& s : vec)
    {
        res += std::move(s);
        res += std::move(sep);
    }
    size_t sep_size = sep.size();
    res.erase(res.size() - sep_size, sep_size);
    return std::move(res);
}
//-----------------------------------------------------------------------------
