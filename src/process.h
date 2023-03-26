#pragma once

#include <tl/expected.hpp>

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <system_error>
#include <vector>

namespace shadercompile
{
class Process
{
public:
    Process() = default;
    explicit Process(std::string_view command);
    explicit Process(std::wstring_view command);
    explicit Process(std::wstring command);
    explicit Process(const std::filesystem::path& command);
    Process(const Process&) = delete;
    Process(Process&&) = default;
    ~Process();

    Process& operator=(const Process&) = delete;
    Process& operator=(Process&&) = default;

    void addArgument(std::string_view arg);
    void addArgument(std::wstring_view arg);
    void addArgument(const std::string& arg);
    void addArgument(std::wstring arg);
    void addArgument(const std::filesystem::path& path);

    template<std::input_iterator ArgIterator>
    void addArguments(ArgIterator first, ArgIterator last)
    {
        mArguments.reserve(mArguments.size() + std::distance(first, last));
        
        for(ArgIterator itr = first; itr != last; ++itr)
        {
            addArgument(*itr);
        }
    }

    void clearArguments() { mArguments.clear(); }

    void clearOutput() { mOutput.clear(); }

    [[nodiscard]] std::span<const std::wstring> arguments() const noexcept { return mArguments; }

    [[nodiscard]] std::wstring_view command() const noexcept { return mCommand; }

    tl::expected<int, std::errc> execute();

    [[nodiscard]] std::span<const std::byte> output() const& { return mOutput; }

    [[nodiscard]] std::vector<std::byte>& output() && { return mOutput; }

private:
    bool createStdIOHandles();
    tl::expected<int, std::errc> createChildProcess();

    void readChildOutput();

    std::wstring mCommand;
    std::vector<std::wstring> mArguments;
    std::wstring mArgumentsString;
    std::vector<std::byte> mOutput;

    void* mChildStdOutRead = nullptr;
    void* mChildStdOutWrite = nullptr;
    void* mChildStdInRead = nullptr;
    void* mChildStdInWrite = nullptr;

    void* mProcessHandle = nullptr;
    void* mThreadHandle = nullptr;
};
} // namespace shadercompile