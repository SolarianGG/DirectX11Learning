#include <windows.h>
#include <exception>
#include <string>

namespace DX
{
    std::string HrToString(HRESULT hr);

    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr);

        const char* what() const noexcept override;

    private:
        HRESULT result;
    };

    void ThrowIfFailed(HRESULT hr);
}