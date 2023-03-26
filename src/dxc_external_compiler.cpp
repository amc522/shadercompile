#include "shadercompile/dxc_external_compiler.h"

#include "process.h"
#include "utility.h"

#include <fstream>

using namespace std::string_view_literals;

namespace shadercompile
{
DxcExternalCompiler::DxcExternalCompiler(std::filesystem::path compilerPath)
{
    mProcess = std::make_unique<Process>(compilerPath);
}

DxcExternalCompiler::~DxcExternalCompiler() = default;

void DxcExternalCompiler::addArgument(std::string_view arg)
{
    if(arg.empty()) { return; }
    mProcess->addArgument(arg);
}

void DxcExternalCompiler::addArguments(std::span<const std::string> args)
{
    mProcess->addArguments(args.begin(), args.end());
}

void DxcExternalCompiler::addArguments(std::span<std::string_view> args)
{
    mProcess->addArguments(args.begin(), args.end());
}

void DxcExternalCompiler::addArgument(std::wstring_view arg)
{
    mProcess->addArgument(arg);
}

void DxcExternalCompiler::addArguments(std::span<const std::wstring> args)
{
    mProcess->addArguments(args.begin(), args.end());
}

void DxcExternalCompiler::addArguments(std::span<std::wstring_view> args)
{
    mProcess->addArguments(args.begin(), args.end());
}

std::span<const std::wstring> DxcExternalCompiler::arguments() const
{
    return (mProcess != nullptr) ? mProcess->arguments() : std::span<const std::wstring>{};
}

std::wstring_view DxcExternalCompiler::command() const
{
    return (mProcess != nullptr) ? mProcess->command() : std::wstring_view{};
}

tl::expected<CompileSummary, std::errc>
DxcExternalCompiler::compileFromFile(const std::filesystem::path& shaderFilePath)
{
    return compileFromFile(shaderFilePath, {});
}

tl::expected<CompileSummary, std::errc> DxcExternalCompiler::compileFromBuffer(std::span<const std::byte> shaderSource,
                                                                               std::string_view shaderSourceName)
{
    tl::expected<std::filesystem::path, std::errc> createFileResult =
        createTemporaryFilePath(L"shader-", L"", L".hlsl");

    if(!createFileResult) { return tl::make_unexpected(createFileResult.error()); }

    mShaderFilePath = std::move(createFileResult.value());

    {
        std::ofstream tempFileStream(mShaderFilePath);

        if(!tempFileStream.is_open()) { return tl::make_unexpected(std::errc::io_error); }

        tempFileStream.write(reinterpret_cast<const char*>(shaderSource.data()), shaderSource.size());
    }

    return compileFromFile(mShaderFilePath, shaderSourceName);
}

tl::expected<CompileSummary, std::errc> DxcExternalCompiler::compileFromBuffer(std::span<const std::byte> shaderSource,
                                                                               std::wstring_view shaderSourceName)
{
    std::string utf8SourceName = utf8Encode(shaderSourceName);
    return compileFromBuffer(shaderSource, utf8SourceName);
}

void DxcExternalCompiler::reset() noexcept
{
    detail::BaseDxcCompiler::reset();
    mProcess->clearArguments();
    mProcess->clearOutput();
}

tl::expected<CompileSummary, std::errc>
DxcExternalCompiler::compileFromFile(const std::filesystem::path& shaderFilePath, std::string_view shaderSourceName)
{
    forEachEnum<DxcArtifactType>(
        [&](DxcArtifactType type)
        {
            if(!shouldOutputArtifact(type)) { return; }

            addArtifactArguments(type);
        });

    mProcess->addArgument(shaderFilePath);

    tl::expected<int, std::errc> executeResult = mProcess->execute();

    if(!executeResult) { return tl::make_unexpected(executeResult.error()); }

    forEachEnum<DxcArtifactType>(mArtifacts,
                                 [](DxcArtifactType /*type*/, DxcArtifact& artifact)
                                 {
                                     if(artifact.sinkType() != DxcSinkType::MemoryBuffer) { return; }

                                     std::ifstream artifactFileStream(artifact.path(),
                                                                      std::ios_base::in | std::ios_base::binary);

                                     if(!artifactFileStream.is_open()) { return; }

                                     const std::streampos beginPos = artifactFileStream.tellg();
                                     artifactFileStream.seekg(0, std::ios_base::end);
                                     const std::streampos endPos = artifactFileStream.tellg();
                                     artifactFileStream.seekg(0, std::ios_base::beg);

                                     std::vector<std::byte> buffer(endPos - beginPos);
                                     artifactFileStream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

                                     artifact = DxcArtifact(std::move(buffer));
                                 });

    std::span<const std::byte> byteOutput = mProcess->output();

    std::string_view compilerOutput(reinterpret_cast<const char*>(byteOutput.data()), byteOutput.size());

    // replace the file name (probably a temporary file name), with shaderSourceName
    std::string modifiedOutput;

    if(!shaderSourceName.empty())
    {
        modifiedOutput.reserve(compilerOutput.size());

        const std::string utf8FilePath = shaderFilePath.string();

        const std::boyer_moore_horspool_searcher searcher(utf8FilePath.begin(), utf8FilePath.end());

        auto searchStartItr = compilerOutput.begin();

        while(searchStartItr != compilerOutput.end())
        {
            auto searchItr = std::search(searchStartItr, compilerOutput.end(), searcher);

            modifiedOutput.append(searchStartItr, searchItr);

            if(searchItr != compilerOutput.end())
            {
                modifiedOutput.append(shaderSourceName);
                searchStartItr = searchItr + utf8FilePath.size();
            }
            else { searchStartItr = compilerOutput.end(); }
        }

        compilerOutput = modifiedOutput;
    }

    mCompilerMessages.clear();
    parseCompilerMessages(compilerOutput, mCompilerMessages);

    CompileSummary summary;
    summary.returnCode = executeResult.value();
    summary.messages = mCompilerMessages;
    summary.arguments = mProcess->arguments();

    for(const CompilerMessage& message : mCompilerMessages)
    {
        summary.errorCount += (message.type == CompilerMessageType::Error) ? 1 : 0;
        summary.warningCount += (message.type == CompilerMessageType::Warning) ? 1 : 0;
    }

    return summary;
}

void DxcExternalCompiler::addArtifactArguments(DxcArtifactType artifactType)
{
    static constexpr std::array<std::wstring_view, (size_t)DxcArtifactType::_count> kArtifactArgumentPrefixes = {
        L"-Fc"sv,  // AssemblyCodeListing
        L"-Fd"sv,  // Debug
        L"-Fo"sv,  // Object
        L"-Fre"sv, // Reflection
        L"-Frs"sv, // Root signature
        L"-Fsh"sv  // Shader hash
    };

    DxcArtifact& artifact = accessArtifact(artifactType);

    if(artifact.sinkType() == DxcSinkType::None) { return; }

    if(artifact.sinkType() == DxcSinkType::File && artifact.path().empty()) { return; }

    if(artifact.sinkType() == DxcSinkType::MemoryBuffer)
    {
        tl::expected<std::filesystem::path, std::errc> createFilePathResult =
            createTemporaryFilePath(L"", kArtifactArgumentPrefixes[(size_t)artifactType], L".tmp");

        if(!createFilePathResult) { return; }

        artifact.setPath(std::move(createFilePathResult.value()));
    }

    mProcess->addArgument(kArtifactArgumentPrefixes[(size_t)artifactType]);
    mProcess->addArgument(artifact.path());
}
} // namespace shadercompile