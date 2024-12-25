#include "DXHelper.hpp"

namespace DX
{
    std::string HrToString(HRESULT hr) {
        char* errorMsg = nullptr;

        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&errorMsg, 0, nullptr
        );

        std::string result = errorMsg ? errorMsg : "Unknown error.";
        LocalFree(errorMsg);

        return result;
    }

    com_exception::com_exception(HRESULT hr) : result(hr) {}

    const char* com_exception::what() const noexcept
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Error: %s\n", HrToString(result).c_str());
        return s_str;
    }

    void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}
