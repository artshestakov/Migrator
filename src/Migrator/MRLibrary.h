#pragma once
//-----------------------------------------------------------------------------
#include "MRUtils.h"
#include "MRException.h"
#include "MRString.h"
//-----------------------------------------------------------------------------
class MRLibrary
{
public:
    MRLibrary(const std::string& path = std::string());
    ~MRLibrary();

    const std::string& GetErrorString() const;
    void SetPath(const std::string& path);
    void SetUseException(bool use);

    bool Load();
    bool Unload();

    template <typename T>
    T GetFunction(const std::string& function_name)
    {
        if (!m_Library)
        {
            m_ErrorString = "library is not initialized";
            return nullptr;
        }
#ifdef WIN32
        auto addr = (T)GetProcAddress(m_Library, function_name.c_str());
#else
        auto addr = (T)dlsym(m_Library, function_name.c_str());
#endif
        if (!addr)
        {
            m_ErrorString = MRString::GetLastErrorS();
            if (m_UseException)
            {
                throw MRException(m_ErrorString.c_str());
            }
            return nullptr;
        }
        return addr;
    }

private:
    std::string m_ErrorString;
    std::string m_Path;
    bool m_UseException;

#ifdef WIN32
    HMODULE m_Library;
#else
    void* m_Library;
#endif
};
//-----------------------------------------------------------------------------
