#include <windows.h>

#include <exception>

#include <SimpleMath.h>


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

    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        const char* what() const noexcept override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Error: %s\n", HrToString(result).c_str());
            return s_str;
        }

    private:
        HRESULT result;
    };

    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }


    namespace Color
    {
        constexpr DirectX::SimpleMath::Color White = { 1, 1, 1, 1 };
        constexpr DirectX::SimpleMath::Color Black = { 0, 0, 0, 1 };
        constexpr DirectX::SimpleMath::Color Red = { 1, 0, 0, 1 };
        constexpr DirectX::SimpleMath::Color Green = { 0, 1, 0, 1 };
        constexpr DirectX::SimpleMath::Color Blue = { 0, 0, 1, 1 };
        constexpr DirectX::SimpleMath::Color Yellow = { 1, 1, 0, 1 };
        constexpr DirectX::SimpleMath::Color Purple = { 1, 0, 1, 1 };
    }
}