#pragma once

#include <shadercompile/detail/dxc_compiler_common.h>
#include <tl/expected.hpp>

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

struct DxcBuffer;
struct IDxcUtils;

namespace shadercompile
{
class DxcLibraryCompiler : public detail::BaseDxcCompiler
{
public:
    ~DxcLibraryCompiler() override = default;

    void addArgument(std::string_view arg) override;
    void addArguments(std::span<const std::string> args) override;
    void addArguments(std::span<std::string_view> args) override;
    void addArgument(std::wstring_view arg) override;
    void addArguments(std::span<const std::wstring> args) override;
    void addArguments(std::span<std::wstring_view> args) override;

    [[nodiscard]] std::span<const std::wstring> arguments() const { return mArguments; }

    void reset() noexcept override;

    tl::expected<CompileSummary, std::errc> compileFromFile(const std::filesystem::path& shaderFilePath) override;
    tl::expected<CompileSummary, std::errc> compileFromBuffer(std::span<const std::byte> shaderSource,
                                                              std::string_view shaderSourceName) override;
    tl::expected<CompileSummary, std::errc> compileFromBuffer(std::span<const std::byte> shaderSource,
                                                              std::wstring_view shaderSourceName) override;

private:
    tl::expected<CompileSummary, std::errc>
    compileFromBuffer(DxcBuffer source, std::wstring_view shaderSourceName, Microsoft::WRL::ComPtr<IDxcUtils> utils);

    std::vector<std::wstring> mArguments;
    std::vector<const std::wstring::value_type*> mArgumentsBuffer;
};
} // namespace shadercompile