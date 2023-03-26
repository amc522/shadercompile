#include "process.h"

#include "utility.h"

#include <Windows.h>

#include <array>

#ifdef UNICODE
#define CHAR_LITERAL(x) L#x
#else
#define CHAR_LITERAL(x) x
#endif

namespace shadercompile
{
Process::Process(std::string_view command)
{
    if(command.starts_with('\"'))
    {
        // command is already wrapped in quotations so it can just be moved
        mCommand = utf8Decode(command);
    }
    else
    {
        // check if the command has spaces
        const size_t firstSpaceIndex = command.find(' ');

        if(firstSpaceIndex == std::string::npos)
        {
            // if there are no spaces, the command can be used as is
            mCommand = utf8Decode(command);
        }
        else
        {
            // otherwise the comman needs to be wrapped in quotations
            mCommand.reserve(command.size() + 2);
            mCommand.append(L"\"");
            utf8Decode(command, mCommand);
            mCommand.append(L"\"");
        }
    }
}

Process::Process(std::wstring_view command)
    : mCommand(command)
{}

Process::Process(std::wstring command)
{
    if(command.starts_with('\"'))
    {
        // command is already wrapped in quotations so it can just be moved
        mCommand = std::move(command);
    }
    else
    {
        // check if the command has spaces
        const size_t firstSpaceIndex = command.find(' ');

        if(firstSpaceIndex == std::string::npos)
        {
            // if there are no spaces, the command can be used as is
            mCommand = std::move(command);
        }
        else
        {
            // otherwise the comman needs to be wrapped in quotations
            mCommand.reserve(command.size() + 2);
            mCommand.append(L"\"");
            mCommand.append(command);
            mCommand.append(L"\"");
        }
    }
}

Process::Process(const std::filesystem::path& command)
    : mCommand(command.native())
{}

Process::~Process()
{
    if(mChildStdOutRead != nullptr) { CloseHandle(mChildStdOutRead); }

    if(mChildStdOutWrite != nullptr) { CloseHandle(mChildStdOutWrite); }

    if(mChildStdInRead != nullptr) { CloseHandle(mChildStdInRead); }

    if(mChildStdInWrite != nullptr) { CloseHandle(mChildStdInWrite); }

    if(mProcessHandle != nullptr) { CloseHandle(mProcessHandle); }

    if(mThreadHandle != nullptr) { CloseHandle(mThreadHandle); }
}

void Process::addArgument(std::string_view arg)
{
    mArguments.push_back(utf8Decode(arg));
}

void Process::addArgument(std::wstring_view arg)
{
    mArguments.emplace_back(arg);
}

void Process::addArgument(const std::string& arg)
{
    addArgument(std::string_view(arg));
}

void Process::addArgument(std::wstring arg)
{
    mArguments.push_back(std::move(arg));
}

void Process::addArgument(const std::filesystem::path& path)
{
    mArguments.emplace_back(path.native());
}

tl::expected<int, std::errc> Process::execute()
{
    if(!createStdIOHandles()) { return tl::make_unexpected(std::errc::broken_pipe); }

    return createChildProcess();
}

bool Process::createStdIOHandles()
{
    SECURITY_ATTRIBUTES securityAttributes;
    securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttributes.bInheritHandle = TRUE;
    securityAttributes.lpSecurityDescriptor = nullptr;

    if(!CreatePipe(&mChildStdOutRead, &mChildStdOutWrite, &securityAttributes, 0)) { return false; }

    if(!SetHandleInformation(mChildStdOutRead, HANDLE_FLAG_INHERIT, 0)) { return false; }

    if(!CreatePipe(&mChildStdInRead, &mChildStdInWrite, &securityAttributes, 0)) { return false; }

    if(!SetHandleInformation(mChildStdInWrite, HANDLE_FLAG_INHERIT, 0)) { return false; }

    return true;
}

tl::expected<int, std::errc> Process::createChildProcess()
{
    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));

    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdError = mChildStdOutWrite;
    startupInfo.hStdOutput = mChildStdOutWrite;
    startupInfo.hStdInput = mChildStdInRead;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));

    size_t finalStringSize = 1;

    for(const std::wstring& arg : mArguments)
    {
        finalStringSize += arg.size() + 1;
    }

    mArgumentsString.clear();
    mArgumentsString.reserve(finalStringSize);

    for(const std::wstring& arg : mArguments)
    {
        mArgumentsString.push_back(L' ');
        mArgumentsString.append(arg);
    }

    const BOOL success = CreateProcess(mCommand.c_str(),
                                       mArgumentsString.data(),
                                       nullptr,
                                       nullptr,
                                       TRUE,
                                       0,
                                       nullptr,
                                       nullptr,
                                       &startupInfo,
                                       &processInfo);

    if(!success) { return tl::make_unexpected(std::errc::invalid_argument); }

    mProcessHandle = processInfo.hProcess;
    mThreadHandle = processInfo.hThread;

    for(;;)
    {
        readChildOutput();

        DWORD ret = WaitForMultipleObjects(1, &mProcessHandle, false, 100);

        if(ret == WAIT_OBJECT_0)
        {
            readChildOutput();
            break;
        }
        else if(ret == WAIT_FAILED) { break; }
    }

    DWORD exitCode;
    if(GetExitCodeProcess(mProcessHandle, &exitCode)) { return exitCode; }

    return 0;
}

void Process::readChildOutput()
{
    std::array<std::byte, 4096> buffer;

    for(;;)
    {
        DWORD bytesRead = 0;
        DWORD bytesAvailable = 0;

        if(!PeekNamedPipe(mChildStdOutRead, nullptr, 0, nullptr, &bytesAvailable, nullptr)) { return; }

        if(bytesAvailable == 0) { return; }

        BOOL readSuccess = ReadFile(mChildStdOutRead, buffer.data(), (DWORD)buffer.size(), &bytesRead, nullptr);

        if(!readSuccess || bytesRead == 0) { return; }

        mOutput.insert(mOutput.cend(), buffer.cbegin(), buffer.cbegin() + bytesRead);
    }
}

} // namespace shadercompile