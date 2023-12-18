#include "MRLibrary.h"
#include "MRFileSystem.h"
//-----------------------------------------------------------------------------
MRLibrary::MRLibrary(const std::string& path)
    : m_Path(path),
    m_UseException(false),
    m_Library(nullptr)
{

}
//-----------------------------------------------------------------------------
MRLibrary::~MRLibrary()
{
    Unload();
}
//-----------------------------------------------------------------------------
const std::string& MRLibrary::GetErrorString() const
{
    return m_ErrorString;
}
//-----------------------------------------------------------------------------
void MRLibrary::SetPath(const std::string& path)
{
    m_Path = path;
}
//-----------------------------------------------------------------------------
void MRLibrary::SetUseException(bool use)
{
    m_UseException = use;
}
//-----------------------------------------------------------------------------
bool MRLibrary::Load()
{
    if (!MRFileSystem::FileExist(m_Path))
    {
        m_ErrorString = MRString::StringF("library doesn't exist on path \"%s\"", m_Path.c_str());
        if (m_UseException)
        {
            throw MRException(m_ErrorString.c_str());
        }
        return false;
    }

    if (m_Library)
    {
        m_ErrorString = "library already loaded";
        return false;
    }

#ifdef WIN32
    m_Library = LoadLibrary(m_Path.c_str());
#else
    m_Library = dlopen(m_Path.c_str(), RTLD_NOW);
#endif
    if (!m_Library)
    {
        m_ErrorString = MRString::GetLastErrorS();
        if (m_UseException)
        {
            throw MRException(m_ErrorString.c_str());
        }
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool MRLibrary::Unload()
{
    if (m_Library)
    {
#ifdef WIN32
        bool ok = FreeLibrary(m_Library) == TRUE;
#else
        bool ok = dlclose(m_Library) == 0;
#endif
        if (ok)
        {
            m_Library = nullptr;
        }
        else
        {
            m_ErrorString = MRString::GetLastErrorS();
            if (m_UseException)
            {
                throw MRException(m_ErrorString.c_str());
            }
            return false;
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
