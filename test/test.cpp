#include <shadercompile/dxc.h>
#include <shadercompile/shadercompile.h>

#include <iostream>

using namespace shadercompile;

constexpr std::string_view kErrorHlsl = R"(struct IAInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 binormal : BINORMAL;
    float3 tangent : TANGENT;
    float2 texCoord: TEXCOORD0;
};

float4x4 gObjectToClip;

struct VertexOutput {
    float3 clipPosition : SV_POSITION;
};

VertexOutput main(IAInput input) {
    VertexOutput output;
    output.clipPosition = mul(gObjectToClip, float4(input.position, 1.0));

    return output;
})";

constexpr std::string_view kVertexHlsl = R"(struct IAInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 binormal : BINORMAL;
    float3 tangent : TANGENT;
    float2 texCoord: TEXCOORD0;
};

float4x4 gObjectToClip;

struct VertexOutput {
    float4 clipPosition : SV_POSITION;
};

VertexOutput main(IAInput input) {
    VertexOutput output;
    output.clipPosition = mul(gObjectToClip, float4(input.position, 1.0));

    return output;
})";

void compileShader(ICompiler& compiler, std::string_view shaderSource)
{
    tl::expected<CompileSummary, std::errc> summary =
        compiler.compileFromBuffer(std::as_bytes(std::span<const char>(shaderSource)), "shader.hlsl");

    if(!summary)
    {
        std::cout << "Failed to start compiler process" << std::endl;
        return;
    }

    for(const std::wstring& arg : summary->arguments)
    {
        std::wcout << L" " << arg;
    }

    std::wcout << std::endl;

    for(CompilerMessage message : summary->messages)
    {
        bool printed = false;
        if(message.hasFilePath())
        {
            printed = true;
            std::cout << message.filePath();
        }
        if(message.hasLine())
        {
            printed = true;
            std::cout << ":" << message.line;
        }
        if(message.hasColumn())
        {
            printed = true;
            std::cout << ":" << message.column;
        }
        if(message.type != CompilerMessageType::Unknown)
        {
            if(printed) { std::cout << ": "; }
            std::cout << toStringView(message.type);

            printed = true;
        }
        if(message.hasMessage())
        {
            if(printed) { std::cout << ": "; }
            std::cout << message.message();

            printed = true;
        }

        if(!printed) { std::cout << message.fullMessage; }

        std::cout << std::endl;
    }
}

int main(int /*argc*/, char** /*argv*/)
{
    DxcReleaseManager releaseManager("");
    releaseManager.downloadLatestRelease();

    const std::filesystem::path& dxcDirectory =
        releaseManager.getReleaseDirectory(releaseManager.getLastestReleaseVersion().value());

    DxcExternalCompiler compiler(dxcDirectory / "bin/x64/dxc.exe");
    //DxcLibraryCompiler compiler;
    {
        compiler.setTargetProfile(DxcTargetProfile::vs_6_7);
        compiler.setEntryPoint("main");
        compiler.enableArtifactWithFileSink(DxcArtifactType::Object, L"error.bin");

        std::wcout << compiler.command();
        compileShader(compiler, kErrorHlsl);
    }

    std::cout << std::endl;

    {
        compiler.reset();
        compiler.setTargetProfile(DxcTargetProfile::vs_6_7);
        compiler.setEntryPoint("main");
        compiler.addArgument("-Zi");
        compiler.enableArtifactWithFileSink(DxcArtifactType::Object, L"vertex.bin");
        compiler.enableArtifactWithFileSink(DxcArtifactType::AssemblyCodeListing, L"vertex.assem");
        compiler.enableArtifactWithFileSink(DxcArtifactType::Debug, L".\\");
        compiler.enableArtifactWithFileSink(DxcArtifactType::Reflection, L"vertex.refl");
        compiler.enableArtifactWithFileSink(DxcArtifactType::RootSignature, L"vertex.rootsig");
        compiler.enableArtifactWithMemorySink(DxcArtifactType::ShaderHash);

        std::wcout << compiler.command();
        compileShader(compiler, kVertexHlsl);
    }

    return 0;
}