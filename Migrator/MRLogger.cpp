#include "MRLogger.h"
#include "MRUtils.h"
#include "MRConstants.h"
#include "ISDateTime.h"
#include "ISException.h"
//-----------------------------------------------------------------------------
inline constexpr size_t LOG_HEADER_SIZE = 128; //Максимальный размер заголовка
//-----------------------------------------------------------------------------
MRLogger::MRLogger()
    : m_IsInit(false),
    m_LogDirPath(ISAlgorithm::GetApplicationDirP() / "Logs")
{

}
//-----------------------------------------------------------------------------
MRLogger::~MRLogger()
{
    if (m_File.is_open() && m_IsInit)
    {
        m_File.close();
        m_IsInit = false;
    }
}
//-----------------------------------------------------------------------------
MRLogger& MRLogger::Instance()
{
    static MRLogger logger;
    return logger;
}
//-----------------------------------------------------------------------------
const std::string& MRLogger::GetErrorString() const
{
    return m_ErrorString;
}
//-----------------------------------------------------------------------------
bool MRLogger::Init()
{
    if (m_IsInit)
    {
        return true;
    }

    if (!ISAlgorithm::DirExist(m_LogDirPath))
    {
        if (!ISAlgorithm::DirCreate(m_LogDirPath, &m_ErrorString))
        {
            return false;
        }
    }

    ISDateTime dt = ISDateTime::CurrentDateTime();
    std::filesystem::path file_name = m_LogDirPath / ISAlgorithm::StringF("Migrator%04d%02d%02d%02d%02d%02d.log",
        dt.Date.Year, dt.Date.Month, dt.Date.Day, dt.Time.Hour, dt.Time.Minute, dt.Time.Second);

    m_File.open(file_name, std::ios::out);
    if (!m_File.is_open())
    {
        m_ErrorString = ISAlgorithm::StringF("Cannot open file %s: %s",
            file_name.c_str(), ISAlgorithm::GetLastErrorS().c_str());
        return false;
    }

    m_IsInit = true;
    return true;
}
//-----------------------------------------------------------------------------
void MRLogger::Log(const char* format, ...)
{
    ISDateTime dt = ISDateTime::CurrentDateTime();

    char header_buffer[LOG_HEADER_SIZE] = { 0 };
    std::sprintf(header_buffer, "[%02d.%02d.%04d %02d:%02d:%02d:%03d] ",
        dt.Date.Day, dt.Date.Month, dt.Date.Year,
        dt.Time.Hour, dt.Time.Minute, dt.Time.Second, dt.Time.Milliseconds);

    va_list args;
    va_start(args, format);
    std::string s = std::string(header_buffer) + ISAlgorithm::StringFA(format, args);
    va_end(args);

    Log(s);
}
//-----------------------------------------------------------------------------
void MRLogger::LogWH(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::string s = ISAlgorithm::StringFA(format, args);
    va_end(args);

    Log(s);
}
//-----------------------------------------------------------------------------
void MRLogger::Log(const std::string& s)
{
    if (!m_IsInit)
    {
        throw ISException("Logger is not init! You must call MRLogger::Init()");
    }

    LogConsole(s);
    LogFile(s);
}
//-----------------------------------------------------------------------------
void MRLogger::LogConsole(const std::string& s)
{
    std::cout << s << std::endl;
}
//-----------------------------------------------------------------------------
void MRLogger::LogFile(const std::string& s)
{
    if (m_File.is_open() && m_IsInit)
    {
        m_File << s << std::endl;
    }
}
//-----------------------------------------------------------------------------
