#pragma once

#include <tl/expected.hpp>

#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace shadercompile
{
enum class CompilerMessageType
{
    Unknown,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

[[nodiscard]] constexpr std::string_view toStringView(CompilerMessageType type) noexcept
{
    switch(type)
    {
    case CompilerMessageType::Debug: return "Debug";
    case CompilerMessageType::Info: return "Info";
    case CompilerMessageType::Warning: return "Warning";
    case CompilerMessageType::Error: return "Error";
    case CompilerMessageType::Critical: return "Critical";
    case CompilerMessageType::Unknown: return "Unknown";
    default: return "Unknown CompilerMessageType";
    }
}

struct CompilerMessage
{
    std::string fullMessage;
    int line = -1;
    int column = -1;
    int filePathOffset = -1;
    int filePathCount = 0;
    CompilerMessageType type = CompilerMessageType::Unknown;
    int messageOffset = -1;
    int messageCount = 0;

    [[nodiscard]] bool hasLine() const { return line >= 0; }

    [[nodiscard]] bool hasColumn() const { return column >= 0; }

    [[nodiscard]] bool hasFilePath() const { return filePathOffset >= 0; }

    [[nodiscard]] bool hasMessage() const { return messageOffset >= 0; }

    [[nodiscard]] std::string_view filePath() const
    {
        return std::string_view(fullMessage).substr(filePathOffset, filePathCount);
    }

    [[nodiscard]] std::string_view message() const
    {
        return std::string_view(fullMessage).substr(messageOffset, messageCount);
    }
};

struct CompileSummary
{
    int returnCode = 0;
    std::span<const std::wstring> arguments;
    std::span<const CompilerMessage> messages;
    int errorCount = 0;
    int warningCount = 0;
};

class ICompiler
{
public:
    virtual ~ICompiler() = default;

    virtual void addArgument(std::string_view arg) = 0;
    virtual void addArguments(std::span<const std::string> args) = 0;
    virtual void addArguments(std::span<std::string_view> args) = 0;
    virtual void addArgument(std::wstring_view arg) = 0;
    virtual void addArguments(std::span<const std::wstring> args) = 0;
    virtual void addArguments(std::span<std::wstring_view> args) = 0;

    virtual tl::expected<CompileSummary, std::errc> compileFromFile(const std::filesystem::path& shaderFilePath) = 0;

    virtual tl::expected<CompileSummary, std::errc> compileFromBuffer(std::span<const std::byte> shaderSource,
                                                                      std::string_view shaderSourceName) = 0;

    virtual tl::expected<CompileSummary, std::errc> compileFromBuffer(std::span<const std::byte> shaderSource,
                                                                      std::wstring_view shaderSourceName) = 0;

    [[nodiscard]] virtual std::span<const CompilerMessage> messages() const = 0;
};
} // namespace shadercompile