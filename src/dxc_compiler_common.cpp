#include "shadercompile/detail/dxc_compiler_common.h"

#include "utility.h"

#include <dxcapi.h>

using namespace std::string_view_literals;

namespace shadercompile
{
DxcArtifact::DxcArtifact() noexcept = default;

DxcArtifact::DxcArtifact(DxcArtifact&&) noexcept = default;

DxcArtifact::DxcArtifact(DxcSinkType sinkType) noexcept
    : mSinkType(sinkType)
{}

DxcArtifact::DxcArtifact(std::vector<std::byte> buffer) noexcept
    : mStorage(std::move(buffer))
    , mSinkType(DxcSinkType::MemoryBuffer)
{}

DxcArtifact::DxcArtifact(std::filesystem::path path) noexcept
    : mPath(std::move(path))
    , mSinkType(DxcSinkType::File)
{}

DxcArtifact::DxcArtifact(Microsoft::WRL::ComPtr<IDxcBlob> buffer) noexcept
    : mStorage(std::move(buffer))
    , mSinkType(DxcSinkType::MemoryBuffer)
{}

DxcArtifact::~DxcArtifact() noexcept = default;

DxcArtifact& DxcArtifact::operator=(DxcArtifact&&) noexcept = default;

namespace detail
{
[[nodiscard]] DxcTargetProfile parseTargetProfile(std::string_view targetProfileStr)
{
    if(caseInsensitiveEqual(targetProfileStr, "ps_6_0")) { return DxcTargetProfile::ps_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, "ps_6_1")) { return DxcTargetProfile::ps_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, "ps_6_2")) { return DxcTargetProfile::ps_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, "ps_6_3")) { return DxcTargetProfile::ps_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, "ps_6_4")) { return DxcTargetProfile::ps_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, "ps_6_5")) { return DxcTargetProfile::ps_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, "ps_6_6")) { return DxcTargetProfile::ps_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, "ps_6_7")) { return DxcTargetProfile::ps_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, "vs_6_0")) { return DxcTargetProfile::vs_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, "vs_6_1")) { return DxcTargetProfile::vs_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, "vs_6_2")) { return DxcTargetProfile::vs_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, "vs_6_3")) { return DxcTargetProfile::vs_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, "vs_6_4")) { return DxcTargetProfile::vs_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, "vs_6_5")) { return DxcTargetProfile::vs_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, "vs_6_6")) { return DxcTargetProfile::vs_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, "vs_6_7")) { return DxcTargetProfile::vs_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, "gs_6_0")) { return DxcTargetProfile::gs_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, "gs_6_1")) { return DxcTargetProfile::gs_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, "gs_6_2")) { return DxcTargetProfile::gs_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, "gs_6_3")) { return DxcTargetProfile::gs_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, "gs_6_4")) { return DxcTargetProfile::gs_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, "gs_6_5")) { return DxcTargetProfile::gs_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, "gs_6_6")) { return DxcTargetProfile::gs_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, "gs_6_7")) { return DxcTargetProfile::gs_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, "hs_6_0")) { return DxcTargetProfile::hs_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, "hs_6_1")) { return DxcTargetProfile::hs_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, "hs_6_2")) { return DxcTargetProfile::hs_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, "hs_6_3")) { return DxcTargetProfile::hs_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, "hs_6_4")) { return DxcTargetProfile::hs_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, "hs_6_5")) { return DxcTargetProfile::hs_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, "hs_6_6")) { return DxcTargetProfile::hs_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, "hs_6_7")) { return DxcTargetProfile::hs_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, "ds_6_0")) { return DxcTargetProfile::ds_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, "ds_6_1")) { return DxcTargetProfile::ds_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, "ds_6_2")) { return DxcTargetProfile::ds_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, "ds_6_3")) { return DxcTargetProfile::ds_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, "ds_6_4")) { return DxcTargetProfile::ds_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, "ds_6_5")) { return DxcTargetProfile::ds_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, "ds_6_6")) { return DxcTargetProfile::ds_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, "ds_6_7")) { return DxcTargetProfile::ds_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, "cs_6_0")) { return DxcTargetProfile::cs_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, "cs_6_1")) { return DxcTargetProfile::cs_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, "cs_6_2")) { return DxcTargetProfile::cs_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, "cs_6_3")) { return DxcTargetProfile::cs_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, "cs_6_4")) { return DxcTargetProfile::cs_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, "cs_6_5")) { return DxcTargetProfile::cs_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, "cs_6_6")) { return DxcTargetProfile::cs_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, "cs_6_7")) { return DxcTargetProfile::cs_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, "lib_6_1")) { return DxcTargetProfile::lib_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, "lib_6_2")) { return DxcTargetProfile::lib_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, "lib_6_3")) { return DxcTargetProfile::lib_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, "lib_6_4")) { return DxcTargetProfile::lib_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, "lib_6_5")) { return DxcTargetProfile::lib_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, "lib_6_6")) { return DxcTargetProfile::lib_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, "lib_6_7")) { return DxcTargetProfile::lib_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, "ms_6_5")) { return DxcTargetProfile::ms_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, "ms_6_6")) { return DxcTargetProfile::ms_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, "ms_6_7")) { return DxcTargetProfile::ms_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, "as_6_5")) { return DxcTargetProfile::as_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, "as_6_6")) { return DxcTargetProfile::as_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, "as_6_7")) { return DxcTargetProfile::as_6_7; }

    return DxcTargetProfile::Unknown;
}

[[nodiscard]] DxcTargetProfile parseTargetProfile(std::wstring_view targetProfileStr)
{
    if(caseInsensitiveEqual(targetProfileStr, L"ps_6_0")) { return DxcTargetProfile::ps_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ps_6_1")) { return DxcTargetProfile::ps_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ps_6_2")) { return DxcTargetProfile::ps_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ps_6_3")) { return DxcTargetProfile::ps_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ps_6_4")) { return DxcTargetProfile::ps_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ps_6_5")) { return DxcTargetProfile::ps_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ps_6_6")) { return DxcTargetProfile::ps_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ps_6_7")) { return DxcTargetProfile::ps_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, L"vs_6_0")) { return DxcTargetProfile::vs_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, L"vs_6_1")) { return DxcTargetProfile::vs_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, L"vs_6_2")) { return DxcTargetProfile::vs_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, L"vs_6_3")) { return DxcTargetProfile::vs_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, L"vs_6_4")) { return DxcTargetProfile::vs_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, L"vs_6_5")) { return DxcTargetProfile::vs_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, L"vs_6_6")) { return DxcTargetProfile::vs_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, L"vs_6_7")) { return DxcTargetProfile::vs_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, L"gs_6_0")) { return DxcTargetProfile::gs_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, L"gs_6_1")) { return DxcTargetProfile::gs_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, L"gs_6_2")) { return DxcTargetProfile::gs_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, L"gs_6_3")) { return DxcTargetProfile::gs_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, L"gs_6_4")) { return DxcTargetProfile::gs_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, L"gs_6_5")) { return DxcTargetProfile::gs_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, L"gs_6_6")) { return DxcTargetProfile::gs_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, L"gs_6_7")) { return DxcTargetProfile::gs_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, L"hs_6_0")) { return DxcTargetProfile::hs_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, L"hs_6_1")) { return DxcTargetProfile::hs_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, L"hs_6_2")) { return DxcTargetProfile::hs_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, L"hs_6_3")) { return DxcTargetProfile::hs_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, L"hs_6_4")) { return DxcTargetProfile::hs_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, L"hs_6_5")) { return DxcTargetProfile::hs_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, L"hs_6_6")) { return DxcTargetProfile::hs_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, L"hs_6_7")) { return DxcTargetProfile::hs_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ds_6_0")) { return DxcTargetProfile::ds_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ds_6_1")) { return DxcTargetProfile::ds_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ds_6_2")) { return DxcTargetProfile::ds_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ds_6_3")) { return DxcTargetProfile::ds_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ds_6_4")) { return DxcTargetProfile::ds_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ds_6_5")) { return DxcTargetProfile::ds_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ds_6_6")) { return DxcTargetProfile::ds_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ds_6_7")) { return DxcTargetProfile::ds_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, L"cs_6_0")) { return DxcTargetProfile::cs_6_0; }
    else if(caseInsensitiveEqual(targetProfileStr, L"cs_6_1")) { return DxcTargetProfile::cs_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, L"cs_6_2")) { return DxcTargetProfile::cs_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, L"cs_6_3")) { return DxcTargetProfile::cs_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, L"cs_6_4")) { return DxcTargetProfile::cs_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, L"cs_6_5")) { return DxcTargetProfile::cs_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, L"cs_6_6")) { return DxcTargetProfile::cs_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, L"cs_6_7")) { return DxcTargetProfile::cs_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, L"lib_6_1")) { return DxcTargetProfile::lib_6_1; }
    else if(caseInsensitiveEqual(targetProfileStr, L"lib_6_2")) { return DxcTargetProfile::lib_6_2; }
    else if(caseInsensitiveEqual(targetProfileStr, L"lib_6_3")) { return DxcTargetProfile::lib_6_3; }
    else if(caseInsensitiveEqual(targetProfileStr, L"lib_6_4")) { return DxcTargetProfile::lib_6_4; }
    else if(caseInsensitiveEqual(targetProfileStr, L"lib_6_5")) { return DxcTargetProfile::lib_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, L"lib_6_6")) { return DxcTargetProfile::lib_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, L"lib_6_7")) { return DxcTargetProfile::lib_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ms_6_5")) { return DxcTargetProfile::ms_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ms_6_6")) { return DxcTargetProfile::ms_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, L"ms_6_7")) { return DxcTargetProfile::ms_6_7; }
    else if(caseInsensitiveEqual(targetProfileStr, L"as_6_5")) { return DxcTargetProfile::as_6_5; }
    else if(caseInsensitiveEqual(targetProfileStr, L"as_6_6")) { return DxcTargetProfile::as_6_6; }
    else if(caseInsensitiveEqual(targetProfileStr, L"as_6_7")) { return DxcTargetProfile::as_6_7; }

    return DxcTargetProfile::Unknown;
}

void BaseDxcCompiler::setTargetProfile(std::string_view targetProfile) noexcept
{
    mTargetProfile = parseTargetProfile(targetProfile);
    addArgument(L"-T");
    addArgument(targetProfile);
}

void BaseDxcCompiler::setTargetProfile(std::wstring_view targetProfile) noexcept
{
    mTargetProfile = parseTargetProfile(targetProfile);
    std::array args = {L"-T"sv, targetProfile};
    addArguments(args);
}

void BaseDxcCompiler::setTargetProfile(std::wstring&& targetProfile) noexcept
{
    mTargetProfile = parseTargetProfile(targetProfile);
    addArgument(L"-T");
    addArgument(std::move(targetProfile));
}

void BaseDxcCompiler::setTargetProfile(DxcTargetProfile targetProfile) noexcept
{
    mTargetProfile = targetProfile;
    std::array args = {L"-T"sv, toWStringView(targetProfile)};
    addArguments(args);
}

void BaseDxcCompiler::setEntryPoint(std::string_view entryPoint) noexcept
{
    setEntryPoint(utf8Decode(entryPoint));
}

void BaseDxcCompiler::setEntryPoint(std::wstring_view entryPoint) noexcept
{
    mEntryPoint = entryPoint;
    std::array args = {L"-E"sv, std::wstring_view(mEntryPoint)};
    addArguments(args);
}

void BaseDxcCompiler::setEntryPoint(std::wstring&& entryPoint) noexcept
{
    mEntryPoint = std::move(entryPoint);
    std::array args = {L"-E"sv, std::wstring_view(mEntryPoint)};
    addArguments(args);
}

void BaseDxcCompiler::enableArtifactWithFileSink(DxcArtifactType type, std::filesystem::path path)
{
    accessArtifact(type) = DxcArtifact(std::move(path));
}

void BaseDxcCompiler::enableArtifactWithMemorySink(DxcArtifactType type)
{
    accessArtifact(type) = DxcArtifact(DxcSinkType::MemoryBuffer);
}

void BaseDxcCompiler::disableArtifact(DxcArtifactType type)
{
    accessArtifact(type) = DxcArtifact();
}

const DxcArtifact& BaseDxcCompiler::getArtifact(DxcArtifactType type) const noexcept
{
    if(type >= DxcArtifactType::_count)
    {
        static DxcArtifact kEmptyArtifact{};
        return kEmptyArtifact;
    }

    return mArtifacts[(size_t)type];
}

void BaseDxcCompiler::reset() noexcept
{
    mEntryPoint.clear();
    mShaderFilePath.clear();
    mCompilerMessages.clear();

    forEachEnum<DxcArtifactType>(mArtifacts, [&](DxcArtifactType /*type*/, DxcArtifact& artifact) { artifact = {}; });
}

DxcArtifact& BaseDxcCompiler::accessArtifact(DxcArtifactType type) noexcept
{
    if(type >= DxcArtifactType::_count)
    {
        static DxcArtifact kEmptyArtifact{};
        return kEmptyArtifact;
    }

    return mArtifacts[(size_t)type];
}

bool BaseDxcCompiler::shouldOutputArtifact(DxcArtifactType type) const
{
    return getArtifact(type).sinkType() != DxcSinkType::None;
}

[[nodiscard]] bool startsWithWhiteSpace(std::string_view str)
{
    if(str.empty()) { return false; }

    return std::isblank(str.front());
}

struct FindNewlineResult
{
    size_t pos = std::string_view::npos;
    size_t newlineCharCount = 0;
};

[[nodiscard]] FindNewlineResult findNewline(std::string_view str, size_t offset = 0u)
{
    FindNewlineResult result;
    result.pos = str.find_first_of("\r\n", offset);

    if(result.pos == std::string_view::npos) { return result; }

    if(str[result.pos] == '\r' && (result.pos + 1) < str.size() && str[result.pos + 1] == '\n')
    {
        result.newlineCharCount = 2;
    }
    else { result.newlineCharCount = 1; }

    return result;
}

std::string_view trimNewlines(std::string_view str)
{
    while(!str.empty() && (str.ends_with('\r') || str.ends_with('\n')))
    {
        str = str.substr(0, str.size() - 1);
    }

    return str;
}

[[nodiscard]] std::optional<size_t> findSectionBreak(std::string_view str, size_t offset)
{
    const size_t sectionBreakStart = str.find(": ", offset);

    if(sectionBreakStart == std::string_view::npos) { return std::nullopt; }

    return sectionBreakStart;
}

void splitAllMessages(std::string_view str, std::vector<CompilerMessage>& compilerMessages)
{
    size_t messageStartOffset = 0;
    size_t searchOffset = 0;

    while(searchOffset < str.size())
    {
        const FindNewlineResult findNewlineResult = findNewline(str, searchOffset);

        if(findNewlineResult.pos == std::string_view::npos)
        {
            if(searchOffset > messageStartOffset)
            {
                CompilerMessage compilerMessage;
                compilerMessage.fullMessage = str.substr(messageStartOffset, searchOffset - messageStartOffset - 1);

                if(!compilerMessage.fullMessage.empty()) { compilerMessages.push_back(std::move(compilerMessage)); }

                messageStartOffset = searchOffset;
            }

            CompilerMessage compilerMessage;
            compilerMessage.fullMessage = trimNewlines(str.substr(messageStartOffset));

            if(!compilerMessage.fullMessage.empty()) { compilerMessages.push_back(std::move(compilerMessage)); }

            return;
        }

        std::string_view lineStr =
            str.substr(searchOffset, (findNewlineResult.pos - searchOffset) + findNewlineResult.newlineCharCount);

        if(messageStartOffset == searchOffset || startsWithWhiteSpace(lineStr))
        {
            searchOffset += lineStr.size();
            continue;
        }

        CompilerMessage compilerMessage;
        compilerMessage.fullMessage = trimNewlines(str.substr(messageStartOffset, searchOffset - messageStartOffset));

        if(!compilerMessage.fullMessage.empty()) { compilerMessages.push_back(std::move(compilerMessage)); }

        messageStartOffset = searchOffset;
        searchOffset += lineStr.size();
    }
}

void BaseDxcCompiler::parseCompilerMessages(std::string_view compilerOutput,
                                            std::vector<CompilerMessage>& compilerMessages)
{
    // Example output:
    // C:\Users\test\error.hlsl:17:25: warning: implicit truncation of vector type [-Wconversion]
    //     output.clipPosition = mul(gObjectToClip, float4(input.position, 1.0));
    //                         ^
    // error: validation errors
    //
    // C:\Users\test\error.hlsl:15: error: Not all elements of SV_Position were written.
    // Validation failed.
    //
    // Known message formats:
    // <path>:<line>:<column>: <type>: <message>
    //      <message line 2>
    //
    // <path>:<line>: <type>: <message>
    //
    // <type>: <message>
    //
    // Terms:
    //   section break - ': '
    //   identification section - <path>, <line>, and <column>
    //   type section - message type (i.e. warning, error)
    //   detail section - any text further clarifying and detailing the error or warning

    splitAllMessages(compilerOutput, compilerMessages);

    for(CompilerMessage& compilerMessage : compilerMessages)
    {
        // check
        const size_t sectionBreakOffset = compilerMessage.fullMessage.find(": ");

        if(sectionBreakOffset == std::string_view::npos) { continue; }

        size_t messageTypeOffset = 0;

        if(std::isdigit(compilerMessage.fullMessage[sectionBreakOffset - 1]))
        {
            // If there is a digit to left of the first section break, then this message is one of two forms
            // A: <path>:<line>:<column>: <type>: <message>
            // or
            // B: <path>:<line>: <type>: <message>

            const ptrdiff_t lineOrColumnNumberOffset = (ptrdiff_t)compilerMessage.fullMessage.rfind(':', sectionBreakOffset - 1);

            if(lineOrColumnNumberOffset == std::string::npos)
            {
                // This is an unknown message format. Expecting there to be at least one colon preceding the
                // section break.
                continue;
            }

            messageTypeOffset = sectionBreakOffset + 2;

            int lineOrColumn;

            std::string_view lineOrColumnStr =
                std::string_view(compilerMessage.fullMessage)
                    .substr(lineOrColumnNumberOffset + 1, sectionBreakOffset - lineOrColumnNumberOffset - 1);

            const std::from_chars_result lineOrColumnParseResult =
                std::from_chars(lineOrColumnStr.data(), lineOrColumnStr.data() + lineOrColumnStr.size(), lineOrColumn);

            if(lineOrColumnParseResult.ec != std::errc{})
            {
                // expected a number
                continue;
            }

            if(std::isdigit(compilerMessage.fullMessage[lineOrColumnNumberOffset - 1]))
            {
                // message contains line and column numbers
                compilerMessage.column = lineOrColumn;

                const ptrdiff_t lineNumberOffset = (ptrdiff_t)compilerMessage.fullMessage.rfind(":", lineOrColumnNumberOffset - 1);

                if(lineNumberOffset == std::string::npos)
                {
                    // expected another colon
                    continue;
                }

                std::string_view lineStr =
                    std::string_view(compilerMessage.fullMessage)
                        .substr(lineNumberOffset + 1, lineOrColumnNumberOffset - lineNumberOffset - 1);

                const std::from_chars_result lineParseResult =
                    std::from_chars(lineStr.data(), lineStr.data() + lineStr.size(), compilerMessage.line);

                if(lineParseResult.ec != std::errc{})
                {
                    // expected a number
                    continue;
                }

                compilerMessage.filePathOffset = 0;
                compilerMessage.filePathCount = (int)lineNumberOffset;
            }
            else
            {
                // message contains just a line number
                compilerMessage.line = lineOrColumn;
                compilerMessage.filePathOffset = 0;
                compilerMessage.filePathCount = (int)lineOrColumnNumberOffset;
            }
        }

        // parse message type
        const std::optional<size_t> sectionBreakResult =
            findSectionBreak(compilerMessage.fullMessage, messageTypeOffset);

        if(!sectionBreakResult) { continue; }

        std::string_view messageTypeStr =
            std::string_view(compilerMessage.fullMessage)
                .substr(messageTypeOffset, sectionBreakResult.value() - messageTypeOffset);

        if(caseInsensitiveEqual(messageTypeStr, "warning")) { compilerMessage.type = CompilerMessageType::Warning; }
        else if(caseInsensitiveEqual(messageTypeStr, "error")) { compilerMessage.type = CompilerMessageType::Error; }

        if(sectionBreakResult.value() + 2 >= compilerMessage.fullMessage.size())
        {
            // there is not enough room for a message
            continue;
        }

        compilerMessage.messageOffset = (int)std::min(sectionBreakResult.value() + 2, compilerMessage.fullMessage.size());
        compilerMessage.messageCount = int(std::ssize(compilerMessage.fullMessage) - compilerMessage.messageOffset);
    }
}
} // namespace detail
} // namespace shadercompile