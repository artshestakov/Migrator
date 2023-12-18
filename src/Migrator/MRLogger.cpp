#include "MRLogger.h"
#include "MRException.h"
#include "MRUtils.h"
#include "MRConstants.h"
#include "MRString.h"
#include "MRFileSystem.h"
//-----------------------------------------------------------------------------
MRLogger::MRLogger()
    : m_IsInit(false),
    m_LogDirPath(MRUtils::GetApplicationDir() + MRConstants::PATH_SEPARATOR + "Logs" + MRConstants::PATH_SEPARATOR)
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

    if (!MRFileSystem::DirExist(m_LogDirPath))
    {
        if (!MRFileSystem::DirCreate(m_LogDirPath, &m_ErrorString))
        {
            return false;
        }
    }

    MRDateTime dt = MRDateTime::CurrentDateTime();
    std::string file_name = m_LogDirPath + MRString::StringF("Migrator%04d%02d%02d%02d%02d%02d.log",
        dt.Date.Year, dt.Date.Month, dt.Date.Day, dt.Time.Hour, dt.Time.Minute, dt.Time.Second);

    m_File.open(file_name, std::ios::out);
    if (!m_File.is_open())
    {
        m_ErrorString = MRString::StringF("Cannot open file %s: %s",
            file_name.c_str(), MRString::GetLastErrorS().c_str());
        return false;
    }

    m_IsInit = true;
    return true;
}
//-----------------------------------------------------------------------------
void MRLogger::Log(const char* format, ...)
{
    MRDateTime dt = MRDateTime::CurrentDateTime();

    char header_buffer[MRConstants::LOG_HEADER_SIZE] = { 0 };
    std::sprintf(header_buffer, "[%02d.%02d.%04d %02d:%02d:%02d:%03d] ",
        dt.Date.Day, dt.Date.Month, dt.Date.Year,
        dt.Time.Hour, dt.Time.Minute, dt.Time.Second, dt.Time.Milliseconds);

    va_list args;
    va_start(args, format);
    std::string s = std::string(header_buffer) + MRString::StringFA(format, args);
    va_end(args);

    Log(s);
}
//-----------------------------------------------------------------------------
void MRLogger::LogWH(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::string s = MRString::StringFA(format, args);
    va_end(args);

    Log(s);
}
//-----------------------------------------------------------------------------
void MRLogger::Log(const std::string& s)
{
    if (!m_IsInit)
    {
        throw MRException("Logger is not init! You must call MRLogger::Init()");
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
