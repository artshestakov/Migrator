#pragma once
//-----------------------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------------------
class MRLogger
{
public:
    static MRLogger& Instance();

    const std::string& GetErrorString() const;
    bool Init();

    void Log(const char* format, ...);
    void LogWH(const char* format, ...);

private:
    void Log(const std::string& s);
    void LogConsole(const std::string& s);
    void LogFile(const std::string& s);

private:
    MRLogger();
    ~MRLogger();
    MRLogger(const MRLogger&) = delete;
    MRLogger(MRLogger&&) = delete;
    MRLogger& operator=(const MRLogger&) = delete;
    MRLogger& operator=(MRLogger&&) = delete;

private:
    std::string m_ErrorString;
    bool m_IsInit;
    std::filesystem::path m_LogDirPath;
    std::ofstream m_File;
};
//-----------------------------------------------------------------------------
#define MR_LOG MRLogger::Instance()
//-----------------------------------------------------------------------------
