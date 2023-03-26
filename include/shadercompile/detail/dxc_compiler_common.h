#pragma once

#include <shadercompile/detail/compiler_common.h>
#ifdef _WIN32
#include <wrl/client.h>
#endif

#include <array>
#include <string_view>
#include <variant>

struct IDxcBlob;

namespace shadercompile
{
enum class DxcTargetProfile
{
    // clang-format off
    Unknown,
    ps_6_0, ps_6_1, ps_6_2, ps_6_3, ps_6_4, ps_6_5, ps_6_6, ps_6_7,
    vs_6_0, vs_6_1, vs_6_2, vs_6_3, vs_6_4, vs_6_5, vs_6_6, vs_6_7,
    gs_6_0, gs_6_1, gs_6_2, gs_6_3, gs_6_4, gs_6_5, gs_6_6, gs_6_7,
    hs_6_0, hs_6_1, hs_6_2, hs_6_3, hs_6_4, hs_6_5, hs_6_6, hs_6_7,
    ds_6_0, ds_6_1, ds_6_2, ds_6_3, ds_6_4, ds_6_5, ds_6_6, ds_6_7,
    cs_6_0, cs_6_1, cs_6_2, cs_6_3, cs_6_4, cs_6_5, cs_6_6, cs_6_7,
    lib_6_1, lib_6_2, lib_6_3, lib_6_4, lib_6_5, lib_6_6, lib_6_7,
    ms_6_5, ms_6_6, ms_6_7,
    as_6_5, as_6_6, as_6_7
    // clang-format on
};

[[nodiscard]] constexpr std::string_view toStringView(DxcTargetProfile targetProfile) noexcept
{
    switch(targetProfile)
    {
    case DxcTargetProfile::ps_6_0: return "ps_6_0";
    case DxcTargetProfile::ps_6_1: return "ps_6_1";
    case DxcTargetProfile::ps_6_2: return "ps_6_2";
    case DxcTargetProfile::ps_6_3: return "ps_6_3";
    case DxcTargetProfile::ps_6_4: return "ps_6_4";
    case DxcTargetProfile::ps_6_5: return "ps_6_5";
    case DxcTargetProfile::ps_6_6: return "ps_6_6";
    case DxcTargetProfile::ps_6_7: return "ps_6_7";
    case DxcTargetProfile::vs_6_0: return "vs_6_0";
    case DxcTargetProfile::vs_6_1: return "vs_6_1";
    case DxcTargetProfile::vs_6_2: return "vs_6_2";
    case DxcTargetProfile::vs_6_3: return "vs_6_3";
    case DxcTargetProfile::vs_6_4: return "vs_6_4";
    case DxcTargetProfile::vs_6_5: return "vs_6_5";
    case DxcTargetProfile::vs_6_6: return "vs_6_6";
    case DxcTargetProfile::vs_6_7: return "vs_6_7";
    case DxcTargetProfile::gs_6_0: return "gs_6_0";
    case DxcTargetProfile::gs_6_1: return "gs_6_1";
    case DxcTargetProfile::gs_6_2: return "gs_6_2";
    case DxcTargetProfile::gs_6_3: return "gs_6_3";
    case DxcTargetProfile::gs_6_4: return "gs_6_4";
    case DxcTargetProfile::gs_6_5: return "gs_6_5";
    case DxcTargetProfile::gs_6_6: return "gs_6_6";
    case DxcTargetProfile::gs_6_7: return "gs_6_7";
    case DxcTargetProfile::hs_6_0: return "hs_6_0";
    case DxcTargetProfile::hs_6_1: return "hs_6_1";
    case DxcTargetProfile::hs_6_2: return "hs_6_2";
    case DxcTargetProfile::hs_6_3: return "hs_6_3";
    case DxcTargetProfile::hs_6_4: return "hs_6_4";
    case DxcTargetProfile::hs_6_5: return "hs_6_5";
    case DxcTargetProfile::hs_6_6: return "hs_6_6";
    case DxcTargetProfile::hs_6_7: return "hs_6_7";
    case DxcTargetProfile::ds_6_0: return "ds_6_0";
    case DxcTargetProfile::ds_6_1: return "ds_6_1";
    case DxcTargetProfile::ds_6_2: return "ds_6_2";
    case DxcTargetProfile::ds_6_3: return "ds_6_3";
    case DxcTargetProfile::ds_6_4: return "ds_6_4";
    case DxcTargetProfile::ds_6_5: return "ds_6_5";
    case DxcTargetProfile::ds_6_6: return "ds_6_6";
    case DxcTargetProfile::ds_6_7: return "ds_6_7";
    case DxcTargetProfile::cs_6_0: return "cs_6_0";
    case DxcTargetProfile::cs_6_1: return "cs_6_1";
    case DxcTargetProfile::cs_6_2: return "cs_6_2";
    case DxcTargetProfile::cs_6_3: return "cs_6_3";
    case DxcTargetProfile::cs_6_4: return "cs_6_4";
    case DxcTargetProfile::cs_6_5: return "cs_6_5";
    case DxcTargetProfile::cs_6_6: return "cs_6_6";
    case DxcTargetProfile::cs_6_7: return "cs_6_7";
    case DxcTargetProfile::lib_6_1: return "lib_6_1";
    case DxcTargetProfile::lib_6_2: return "lib_6_2";
    case DxcTargetProfile::lib_6_3: return "lib_6_3";
    case DxcTargetProfile::lib_6_4: return "lib_6_4";
    case DxcTargetProfile::lib_6_5: return "lib_6_5";
    case DxcTargetProfile::lib_6_6: return "lib_6_6";
    case DxcTargetProfile::lib_6_7: return "lib_6_7";
    case DxcTargetProfile::ms_6_5: return "ms_6_5";
    case DxcTargetProfile::ms_6_6: return "ms_6_6";
    case DxcTargetProfile::ms_6_7: return "ms_6_7";
    case DxcTargetProfile::as_6_5: return "as_6_5";
    case DxcTargetProfile::as_6_6: return "as_6_6";
    case DxcTargetProfile::as_6_7: return "as_6_7";
    default: return "Unknown DxcTargetProfile";
    }
}

[[nodiscard]] constexpr std::wstring_view toWStringView(DxcTargetProfile targetProfile) noexcept
{
    switch(targetProfile)
    {
    case DxcTargetProfile::ps_6_0: return L"ps_6_0";
    case DxcTargetProfile::ps_6_1: return L"ps_6_1";
    case DxcTargetProfile::ps_6_2: return L"ps_6_2";
    case DxcTargetProfile::ps_6_3: return L"ps_6_3";
    case DxcTargetProfile::ps_6_4: return L"ps_6_4";
    case DxcTargetProfile::ps_6_5: return L"ps_6_5";
    case DxcTargetProfile::ps_6_6: return L"ps_6_6";
    case DxcTargetProfile::ps_6_7: return L"ps_6_7";
    case DxcTargetProfile::vs_6_0: return L"vs_6_0";
    case DxcTargetProfile::vs_6_1: return L"vs_6_1";
    case DxcTargetProfile::vs_6_2: return L"vs_6_2";
    case DxcTargetProfile::vs_6_3: return L"vs_6_3";
    case DxcTargetProfile::vs_6_4: return L"vs_6_4";
    case DxcTargetProfile::vs_6_5: return L"vs_6_5";
    case DxcTargetProfile::vs_6_6: return L"vs_6_6";
    case DxcTargetProfile::vs_6_7: return L"vs_6_7";
    case DxcTargetProfile::gs_6_0: return L"gs_6_0";
    case DxcTargetProfile::gs_6_1: return L"gs_6_1";
    case DxcTargetProfile::gs_6_2: return L"gs_6_2";
    case DxcTargetProfile::gs_6_3: return L"gs_6_3";
    case DxcTargetProfile::gs_6_4: return L"gs_6_4";
    case DxcTargetProfile::gs_6_5: return L"gs_6_5";
    case DxcTargetProfile::gs_6_6: return L"gs_6_6";
    case DxcTargetProfile::gs_6_7: return L"gs_6_7";
    case DxcTargetProfile::hs_6_0: return L"hs_6_0";
    case DxcTargetProfile::hs_6_1: return L"hs_6_1";
    case DxcTargetProfile::hs_6_2: return L"hs_6_2";
    case DxcTargetProfile::hs_6_3: return L"hs_6_3";
    case DxcTargetProfile::hs_6_4: return L"hs_6_4";
    case DxcTargetProfile::hs_6_5: return L"hs_6_5";
    case DxcTargetProfile::hs_6_6: return L"hs_6_6";
    case DxcTargetProfile::hs_6_7: return L"hs_6_7";
    case DxcTargetProfile::ds_6_0: return L"ds_6_0";
    case DxcTargetProfile::ds_6_1: return L"ds_6_1";
    case DxcTargetProfile::ds_6_2: return L"ds_6_2";
    case DxcTargetProfile::ds_6_3: return L"ds_6_3";
    case DxcTargetProfile::ds_6_4: return L"ds_6_4";
    case DxcTargetProfile::ds_6_5: return L"ds_6_5";
    case DxcTargetProfile::ds_6_6: return L"ds_6_6";
    case DxcTargetProfile::ds_6_7: return L"ds_6_7";
    case DxcTargetProfile::cs_6_0: return L"cs_6_0";
    case DxcTargetProfile::cs_6_1: return L"cs_6_1";
    case DxcTargetProfile::cs_6_2: return L"cs_6_2";
    case DxcTargetProfile::cs_6_3: return L"cs_6_3";
    case DxcTargetProfile::cs_6_4: return L"cs_6_4";
    case DxcTargetProfile::cs_6_5: return L"cs_6_5";
    case DxcTargetProfile::cs_6_6: return L"cs_6_6";
    case DxcTargetProfile::cs_6_7: return L"cs_6_7";
    case DxcTargetProfile::lib_6_1: return L"lib_6_1";
    case DxcTargetProfile::lib_6_2: return L"lib_6_2";
    case DxcTargetProfile::lib_6_3: return L"lib_6_3";
    case DxcTargetProfile::lib_6_4: return L"lib_6_4";
    case DxcTargetProfile::lib_6_5: return L"lib_6_5";
    case DxcTargetProfile::lib_6_6: return L"lib_6_6";
    case DxcTargetProfile::lib_6_7: return L"lib_6_7";
    case DxcTargetProfile::ms_6_5: return L"ms_6_5";
    case DxcTargetProfile::ms_6_6: return L"ms_6_6";
    case DxcTargetProfile::ms_6_7: return L"ms_6_7";
    case DxcTargetProfile::as_6_5: return L"as_6_5";
    case DxcTargetProfile::as_6_6: return L"as_6_6";
    case DxcTargetProfile::as_6_7: return L"as_6_7";
    default: return L"Unknown DxcTargetProfile";
    }
}

enum class DxcArtifactType
{
    AssemblyCodeListing,
    Debug,
    Object,
    Reflection,
    RootSignature,
    ShaderHash,
    _count,
    _first = 0,
    _last = _count - 1
};

enum class DxcSinkType
{
    None,
    File,
    MemoryBuffer
};

class DxcArtifact
{
public:
    DxcArtifact() noexcept;
    DxcArtifact(const DxcArtifact&) = delete;
    DxcArtifact(DxcArtifact&&) noexcept;
    explicit DxcArtifact(DxcSinkType sinkType) noexcept;
    explicit DxcArtifact(std::vector<std::byte> buffer) noexcept;
    explicit DxcArtifact(std::filesystem::path path) noexcept;
    explicit DxcArtifact(Microsoft::WRL::ComPtr<IDxcBlob> buffer) noexcept;
    ~DxcArtifact() noexcept;

    DxcArtifact& operator=(const DxcArtifact&) = delete;
    DxcArtifact& operator=(DxcArtifact&&) noexcept;

    [[nodiscard]] constexpr DxcSinkType sinkType() const noexcept { return mSinkType; }

    [[nodiscard]] constexpr const std::filesystem::path& path() const noexcept { return mPath; }

    void setPath(std::filesystem::path path) noexcept { mPath = std::move(path); }

private:
    DxcSinkType mSinkType = DxcSinkType::None;
    std::filesystem::path mPath;
    std::variant<std::vector<std::byte>, Microsoft::WRL::ComPtr<IDxcBlob>> mStorage;
};

namespace detail
{
class BaseDxcCompiler : public ICompiler
{
public:
    ~BaseDxcCompiler() noexcept override = default;

    void setTargetProfile(std::string_view targetProfile) noexcept;
    void setTargetProfile(std::wstring_view targetProfile) noexcept;
    void setTargetProfile(std::wstring&& targetProfile) noexcept;
    void setTargetProfile(DxcTargetProfile targetProfile) noexcept;
    void setEntryPoint(std::string_view entryPoint) noexcept;
    void setEntryPoint(std::wstring_view entryPoint) noexcept;
    void setEntryPoint(std::wstring&& entryPoint) noexcept;

    void enableArtifactWithFileSink(DxcArtifactType type, std::filesystem::path path);
    void enableArtifactWithMemorySink(DxcArtifactType type);
    void disableArtifact(DxcArtifactType type);

    [[nodiscard]] const DxcArtifact& getArtifact(DxcArtifactType type) const noexcept;

    [[nodiscard]] std::span<const CompilerMessage> messages() const noexcept override { return mCompilerMessages; }

    virtual void reset() noexcept;

protected:
    [[nodiscard]] DxcArtifact& accessArtifact(DxcArtifactType type) noexcept;

    [[nodiscard]] bool shouldOutputArtifact(DxcArtifactType type) const;

    static void parseCompilerMessages(std::string_view compilerOutput, std::vector<CompilerMessage>& compilerMessages);

    std::array<DxcArtifact, (size_t)DxcArtifactType::_count> mArtifacts;

    DxcTargetProfile mTargetProfile = DxcTargetProfile::Unknown;
    std::wstring mEntryPoint;
    std::filesystem::path mShaderFilePath;
    std::vector<CompilerMessage> mCompilerMessages;
};
} // namespace detail

} // namespace shadercompile