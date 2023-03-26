#pragma once

#include <tl/expected.hpp>

#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace shadercompile
{
enum HttpRequestError
{
    FailedToOpenSession,
    InvalidUrl,
    UnrecognizedScheme,
    PlatformError,
};

struct HttpRequestHeader
{
    std::string_view name;
    std::string_view value;
};

class HttpRequest
{
public:
    HttpRequest() = default;
    HttpRequest(const HttpRequest&) = delete;
    HttpRequest(HttpRequest&& other) = default;
    HttpRequest(std::string_view server);

    HttpRequest& operator=(const HttpRequest&) = delete;
    HttpRequest& operator=(HttpRequest&&) = default;

    std::optional<HttpRequestError> openConnection();
    bool closeConnection();

    void setHeaders(std::span<const HttpRequestHeader> headers);

    tl::expected<std::vector<std::byte>, HttpRequestError> get(std::string_view location);

private:
    using WinHttpPtr = std::unique_ptr<void, int (*)(void*)>;

    std::wstring mServer;
    WinHttpPtr mSession;
    WinHttpPtr mConnection;
    WinHttpPtr mRequest;

    std::wstring mHeaders;
};
} // namespace shadercompile