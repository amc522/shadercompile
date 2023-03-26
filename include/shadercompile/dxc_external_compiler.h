#pragma once

#include <shadercompile/detail/dxc_compiler_common.h>
#include <tl/expected.hpp>

#include <filesystem>
#include <span>
#include <string_view>

namespace shadercompile
{
class Process;

class DxcExternalCompiler : public detail::BaseDxcCompiler
{
public:
    DxcExternalCompiler(std::filesystem::path compilerPath);
    ~DxcExternalCompiler() override;

    void addArgument(std::string_view arg) override;
    void addArguments(std::span<const std::string> args) override;
    void addArguments(std::span<std::string_view> args) override;
    void addArgument(std::wstring_view arg) override;
    void addArguments(std::span<const std::wstring> args) override;
    void addArguments(std::span<std::wstring_view> args) override;

    [[nodiscard]] std::span<const std::wstring> arguments() const;
    [[nodiscard]] std::wstring_view command() const;

    tl::expected<CompileSummary, std::errc> compileFromFile(const std::filesystem::path& shaderFilePath) override;
    tl::expected<CompileSummary, std::errc> compileFromBuffer(std::span<const std::byte> shaderSource,
                                                              std::string_view shaderSourceName = {}) override;
    tl::expected<CompileSummary, std::errc> compileFromBuffer(std::span<const std::byte> shaderSource,
                                                              std::wstring_view shaderSourceName = {}) override;

    void reset() noexcept override;

private:
    tl::expected<CompileSummary, std::errc> compileFromFile(const std::filesystem::path& shaderFilePath,
                                                            std::string_view shaderSourceName);

    void addArtifactArguments(DxcArtifactType artifactType);

    std::unique_ptr<Process> mProcess;
};
} // namespace shadercompile