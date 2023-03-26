#include "utility.h"

#include <combaseapi.h>
#include <Windows.h>

#include <format>

namespace shadercompile
{
void utf8Encode(std::wstring_view wideStr, std::string& outUtf8Str)
{
    if(wideStr.empty()) { return; }

    const auto multiByteCount =
        WideCharToMultiByte(CP_UTF8, 0, wideStr.data(), static_cast<int>(wideStr.size()), nullptr, 0, nullptr, nullptr);

    const auto originalUtf8StrSize = outUtf8Str.size();
    outUtf8Str.resize(originalUtf8StrSize + multiByteCount);

    WideCharToMultiByte(CP_UTF8,
                        0,
                        wideStr.data(),
                        static_cast<int>(wideStr.size()),
                        outUtf8Str.data() + originalUtf8StrSize,
                        multiByteCount,
                        nullptr,
                        nullptr);
}

std::string utf8Encode(std::wstring_view str)
{
    std::string utf8Str;
    utf8Encode(str, utf8Str);
    return utf8Str;
}

void utf8Decode(std::string_view utf8Str, std::wstring& outWideStr)
{
    if(utf8Str.empty()) { return; }

    const auto wideCharCount =
        MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.size()), nullptr, 0);

    const auto originalWideStrSize = outWideStr.size();
    outWideStr.resize(outWideStr.size() + wideCharCount);

    MultiByteToWideChar(CP_UTF8,
                        0,
                        utf8Str.data(),
                        static_cast<int>(utf8Str.size()),
                        outWideStr.data() + originalWideStrSize,
                        wideCharCount);
}

std::wstring utf8Decode(std::string_view utf8Str)
{
    std::wstring wideStr;
    utf8Decode(utf8Str, wideStr);
    return wideStr;
}

tl::expected<std::filesystem::path, std::errc>
createTemporaryFilePath(std::wstring_view fileNamePrefix, std::wstring_view fileNameSuffix, std::wstring_view extension)
{
    GUID guid;
    const HRESULT hr = CoCreateGuid(&guid);

    if(FAILED(hr)) { return tl::make_unexpected(std::errc::state_not_recoverable); }

    const std::wstring fileName =
        std::format(L"{}{:08x}{:04x}{:04x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{}{}",
                    fileNamePrefix,
                    guid.Data1,
                    guid.Data2,
                    guid.Data3,
                    guid.Data4[0],
                    guid.Data4[1],
                    guid.Data4[2],
                    guid.Data4[3],
                    guid.Data4[4],
                    guid.Data4[5],
                    guid.Data4[6],
                    guid.Data4[7],
                    fileNameSuffix,
                    extension);

    DWORD tempPathSize = GetTempPath2W(0, nullptr);

    if(tempPathSize == 0) { return tl::make_unexpected(std::errc::state_not_recoverable); }

    std::wstring tempPathStr(tempPathSize - 1, 0);
    tempPathSize = GetTempPath2W((DWORD)tempPathStr.size() + 1, tempPathStr.data());

    if(tempPathSize == 0) { return tl::make_unexpected(std::errc::state_not_recoverable); }

    return std::filesystem::path(tempPathStr) / fileName;
}
} // namespace shadercompile