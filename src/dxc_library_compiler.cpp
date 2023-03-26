#include "shadercompile/dxc_library_compiler.h"

#include "utility.h"

#include <dxcapi.h>

#include <fstream>

using namespace Microsoft::WRL;
using namespace std::string_view_literals;

namespace shadercompile
{
void DxcLibraryCompiler::addArgument(std::string_view arg)
{
    mArguments.push_back(utf8Decode(arg));
}

void DxcLibraryCompiler::addArguments(std::span<const std::string> args)
{
    mArguments.reserve(mArguments.size() + args.size());

    for(const std::string& arg : args)
    {
        mArguments.push_back(utf8Decode(arg));
    }
}

void DxcLibraryCompiler::addArguments(std::span<std::string_view> args)
{
    mArguments.reserve(mArguments.size() + args.size());

    for(const std::string_view arg : args)
    {
        mArguments.push_back(utf8Decode(arg));
    }
}

void DxcLibraryCompiler::addArgument(std::wstring_view arg)
{
    mArguments.emplace_back(arg);
}

void DxcLibraryCompiler::addArguments(std::span<const std::wstring> args)
{
    mArguments.reserve(mArguments.size() + args.size());

    for(const std::wstring& arg : args)
    {
        mArguments.emplace_back(arg);
    }
}

void DxcLibraryCompiler::addArguments(std::span<std::wstring_view> args)
{
    mArguments.reserve(mArguments.size() + args.size());

    for(const std::wstring_view arg : args)
    {
        mArguments.emplace_back(arg);
    }
}

void DxcLibraryCompiler::reset() noexcept
{
    BaseDxcCompiler::reset();
    mArguments.clear();
    mArgumentsBuffer.clear();
}

tl::expected<CompileSummary, std::errc> DxcLibraryCompiler::compileFromFile(const std::filesystem::path& shaderFilePath)
{
    ComPtr<IDxcUtils> utils;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

    ComPtr<IDxcBlobEncoding> sourceBlob;
    HRESULT hr = utils->LoadFile(shaderFilePath.c_str(), nullptr, &sourceBlob);

    if(FAILED(hr)) { return tl::make_unexpected(std::errc::no_such_file_or_directory); }

    BOOL known;
    UINT encoding;
    hr = sourceBlob->GetEncoding(&known, &encoding);

    if(!known || FAILED(hr)) { encoding = DXC_CP_ACP; }
    
    return compileFromBuffer(
        DxcBuffer{.Ptr = sourceBlob->GetBufferPointer(), .Size = sourceBlob->GetBufferSize(), .Encoding = encoding},
        shaderFilePath.native(),
        utils);
}

tl::expected<CompileSummary, std::errc> DxcLibraryCompiler::compileFromBuffer(std::span<const std::byte> shaderSource,
                                                                              std::string_view shaderSourceName)
{
    const std::wstring wideSourceName = utf8Decode(shaderSourceName);
    return compileFromBuffer(shaderSource, wideSourceName);
}

tl::expected<CompileSummary, std::errc>
DxcLibraryCompiler::compileFromBuffer(std::span<const std::byte> shaderContentBuffer,
                                      std::wstring_view shaderSourceName)
{
    ComPtr<IDxcUtils> utils;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

    return compileFromBuffer(DxcBuffer{.Ptr = shaderContentBuffer.data(),
                                       .Size = (UINT32)shaderContentBuffer.size(),
                                       .Encoding = DXC_CP_ACP},
                             shaderSourceName,
                             utils);
}

tl::expected<CompileSummary, std::errc> DxcLibraryCompiler::compileFromBuffer(DxcBuffer source,
                                                                              std::wstring_view sourceName,
                                                                              Microsoft::WRL::ComPtr<IDxcUtils> utils)
{
    ComPtr<IDxcCompiler3> compiler;
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

    ComPtr<IDxcIncludeHandler> includeHandler;
    utils->CreateDefaultIncludeHandler(&includeHandler);

    addArgument(sourceName);

    // Compile
    mArgumentsBuffer.clear();

    for(const std::wstring& argument : mArguments)
    {
        mArgumentsBuffer.push_back(argument.c_str());
    }

    ComPtr<IDxcResult> results;
    HRESULT hr = compiler->Compile(&source,
                                   mArgumentsBuffer.data(),
                                   (UINT32)mArgumentsBuffer.size(),
                                   includeHandler.Get(),
                                   IID_PPV_ARGS(&results));

    if(FAILED(hr)) { return tl::make_unexpected(std::errc::state_not_recoverable); }

    CompileSummary summary;
    summary.arguments = mArguments;

    // Check for errors
    ComPtr<IDxcBlobUtf8> errors;
    hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

    if(FAILED(hr) || (errors != nullptr && errors->GetStringLength() != 0))
    {
        parseCompilerMessages(std::string_view(errors->GetStringPointer(), errors->GetStringLength()),
                              mCompilerMessages);
        summary.messages = mCompilerMessages;

        for(const CompilerMessage& message : mCompilerMessages)
        {
            summary.errorCount += (message.type == CompilerMessageType::Error) ? 1 : 0;
            summary.warningCount += (message.type == CompilerMessageType::Warning) ? 1 : 0;
        }

        return summary;
    }

    forEachEnum<DxcArtifactType>(
        mArtifacts,
        [&](DxcArtifactType artifactType, DxcArtifact& artifact)
        {
            if(artifact.sinkType() == DxcSinkType::None) { return; }

            constexpr std::array<DXC_OUT_KIND, (size_t)DxcArtifactType::_count> kDxcOutKindMap = {
                DXC_OUT_DISASSEMBLY,    // AssemblyCodeListing,
                DXC_OUT_PDB,            // Debug,
                DXC_OUT_OBJECT,         // Object,
                DXC_OUT_REFLECTION,     // Reflection,
                DXC_OUT_ROOT_SIGNATURE, // RootSignature,
                DXC_OUT_SHADER_HASH     // ShaderHash,
            };

            ComPtr<IDxcBlob> output = nullptr;
            ComPtr<IDxcBlobUtf16> outputName = nullptr;
            hr = results->GetOutput(kDxcOutKindMap[(size_t)artifactType],
                                    IID_PPV_ARGS(&output),
                                    outputName.GetAddressOf());

            if(artifact.sinkType() == DxcSinkType::File)
            {
                std::ofstream fileStream(artifact.path(), std::ios_base::out | std::ios_base::binary);

                if(!fileStream.is_open()) { return; }

                fileStream.write(reinterpret_cast<const char*>(output->GetBufferPointer()), output->GetBufferSize());
            }
            else if(artifact.sinkType() == DxcSinkType::MemoryBuffer) { artifact = DxcArtifact(std::move(output)); }
        });

    return summary;
}
} // namespace shadercompile