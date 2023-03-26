#include "http_request.h"

#include "utility.h"

#include <Windows.h>
#include <winhttp.h>

namespace shadercompile
{
HttpRequest::HttpRequest(std::string_view server)
    : mSession(nullptr, &WinHttpCloseHandle)
    , mConnection(nullptr, &WinHttpCloseHandle)
    , mRequest(nullptr, &WinHttpCloseHandle)
{
    mServer = utf8Decode(server);

    mSession = WinHttpPtr(WinHttpOpen(L"shadercompile/1.0",
                                      WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                      WINHTTP_NO_PROXY_NAME,
                                      WINHTTP_NO_PROXY_BYPASS,
                                      0),
                          &WinHttpCloseHandle);

    if(!mSession)
    {
        DWORD error = GetLastError();

        switch(error)
        {
        case ERROR_WINHTTP_INTERNAL_ERROR: // An internal error has occurred.
        case ERROR_NOT_ENOUGH_MEMORY: // Not enough memory was available to complete the requested operation. (Windows
                                      // error code)
        default: break;
        }
    }
}

std::optional<HttpRequestError> HttpRequest::openConnection()
{
    if(!mSession) { return HttpRequestError::FailedToOpenSession; }

    if(mConnection) { return std::nullopt; }

    mConnection = WinHttpPtr(WinHttpConnect(mSession.get(), mServer.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0),
                             &WinHttpCloseHandle);

    if(!mConnection)
    {
        DWORD error = GetLastError();

        switch(error)
        {
        case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: // The type of handle supplied is incorrect for this operation.
            return HttpRequestError::PlatformError;
        case ERROR_WINHTTP_INTERNAL_ERROR: // An internal error has occurred.
            return HttpRequestError::PlatformError;
        case ERROR_WINHTTP_INVALID_URL: // The URL is invalid.
            return HttpRequestError::InvalidUrl;
        case ERROR_WINHTTP_OPERATION_CANCELLED: // The operation was canceled, usually because the handle on which the
                                                // request was operating was closed before the operation completed.
            return HttpRequestError::PlatformError;
        case ERROR_WINHTTP_UNRECOGNIZED_SCHEME: // The URL scheme could not be recognized, or is not supported.
            return HttpRequestError::UnrecognizedScheme;
        case ERROR_WINHTTP_SHUTDOWN: // The WinHTTP function support is being shut down or unloaded.
            return HttpRequestError::PlatformError;
        case ERROR_NOT_ENOUGH_MEMORY: // Not enough memory was available to complete the requested operation. (Windows
                                      // error code)
            return HttpRequestError::PlatformError;
        default: return HttpRequestError::PlatformError;
        }
    }

    return std::nullopt;
}

bool HttpRequest::closeConnection()
{
    mConnection.reset();
    return true;
}

void HttpRequest::setHeaders(std::span<const HttpRequestHeader> headers)
{
    mHeaders.clear();

    size_t approxHeaderStringSize = 0;
    for(const HttpRequestHeader& header : headers)
    {
        // header: value\r\n
        approxHeaderStringSize += header.name.size() + 2 + header.value.size() + 2;
    }

    mHeaders.reserve(approxHeaderStringSize);

    for(const HttpRequestHeader& header : headers)
    {
        utf8Decode(header.name, mHeaders);
        mHeaders.append(L": ");
        utf8Decode(header.value, mHeaders);
        mHeaders.append(L"\r\n");
    }
}

tl::expected<std::vector<std::byte>, HttpRequestError> HttpRequest::get(std::string_view location)
{
    if(!mSession) { return tl::make_unexpected(HttpRequestError::FailedToOpenSession); }

    if(!mConnection)
    {
        if(auto error = openConnection(); error.has_value()) { return tl::make_unexpected(error.value()); }
    }

    std::wstring wideLocation = utf8Decode(location);

    WinHttpPtr request(WinHttpOpenRequest(mConnection.get(),
                                          L"GET",
                                          wideLocation.c_str(),
                                          nullptr,
                                          WINHTTP_NO_REFERER,
                                          WINHTTP_DEFAULT_ACCEPT_TYPES,
                                          WINHTTP_FLAG_SECURE),
                       &WinHttpCloseHandle);

    if(!request)
    {
        DWORD error = GetLastError();

        switch(error)
        {
        case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: // The type of handle supplied is incorrect for this operation.
            return tl::make_unexpected(HttpRequestError::PlatformError);
        case ERROR_WINHTTP_INTERNAL_ERROR: // An internal error has occurred.
            return tl::make_unexpected(HttpRequestError::PlatformError);
        case ERROR_WINHTTP_INVALID_URL: // The URL is invalid.
            return tl::make_unexpected(HttpRequestError::InvalidUrl);
        case ERROR_WINHTTP_OPERATION_CANCELLED: // The operation was canceled, usually because the handle on which the
                                                // request was operating was closed before the operation completed.
            return tl::make_unexpected(HttpRequestError::PlatformError);
        case ERROR_WINHTTP_UNRECOGNIZED_SCHEME: // The URL specified a scheme other than "http:" or "https:".
            return tl::make_unexpected(HttpRequestError::UnrecognizedScheme);
        case ERROR_NOT_ENOUGH_MEMORY: // Not enough memory was available to complete the requested operation. (Windows
                                      // error code)
            return tl::make_unexpected(HttpRequestError::PlatformError);
        default: return tl::make_unexpected(HttpRequestError::PlatformError);
        }
    }

    {
        BOOL success = WinHttpAddRequestHeaders(request.get(),
                                                mHeaders.c_str(),
                                                (DWORD)mHeaders.size(),
                                                WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);

        if(!success)
        {
            DWORD error = GetLastError();

            switch(error)
            {
            case ERROR_WINHTTP_INCORRECT_HANDLE_STATE: return tl::make_unexpected(HttpRequestError::PlatformError);
            case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: return tl::make_unexpected(HttpRequestError::PlatformError);
            case ERROR_WINHTTP_INTERNAL_ERROR: return tl::make_unexpected(HttpRequestError::PlatformError);
            case ERROR_NOT_ENOUGH_MEMORY: return tl::make_unexpected(HttpRequestError::PlatformError);
            case ERROR_INVALID_HANDLE: return tl::make_unexpected(HttpRequestError::PlatformError);
            case ERROR_INVALID_PARAMETER: return tl::make_unexpected(HttpRequestError::PlatformError);
            default: return tl::make_unexpected(HttpRequestError::PlatformError);
            }
        }
    }

    BOOL sendResult =
        WinHttpSendRequest(request.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if(!sendResult)
    {
        DWORD error = GetLastError();

        switch(error)
        {
        case ERROR_WINHTTP_CANNOT_CONNECT: // Returned if connection to the server failed.
            break;
        case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED: // The secure HTTP server requires a client certificate. The
                                                    // application retrieves the list of certificate issuers by calling
                                                    // WinHttpQueryOption with the
                                                    // WINHTTP_OPTION_CLIENT_CERT_ISSUER_LIST option. If the server
                                                    // requests the client certificate, but does not require it, the
                                                    // application can alternately call WinHttpSetOption with the
                                                    // WINHTTP_OPTION_CLIENT_CERT_CONTEXT option. In this case, the
                                                    // application specifies the WINHTTP_NO_CLIENT_CERT_CONTEXT macro in
                                                    // the lpBuffer parameter of WinHttpSetOption. For more information,
                                                    // see the WINHTTP_OPTION_CLIENT_CERT_CONTEXT option.Windows Server
                                                    // 2003 with SP1, Windows XP with SP2 and Windows 2000:  This error
                                                    // is not supported. //case ERROR_WINHTTP_CONNECTION_ERROR: //The
                                                    // connection with the server has been reset or terminated, or an
                                                    // incompatible SSL protocol was encountered. For example, WinHTTP
                                                    // version 5.1 does not support SSL2 unless the client specifically
                                                    // enables it.
            break;
        case ERROR_WINHTTP_INCORRECT_HANDLE_STATE: // The requested operation cannot be carried out because the handle
                                                   // supplied is not in the correct state.
            break;
        case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: // The type of handle supplied is incorrect for this operation.
            break;
        case ERROR_WINHTTP_INTERNAL_ERROR: // An internal error has occurred.
            break;
        case ERROR_WINHTTP_INVALID_URL: // The URL is invalid.
            break;
        case ERROR_WINHTTP_LOGIN_FAILURE: // The login attempt failed. When this error is encountered, the request
                                          // handle should be closed with WinHttpCloseHandle. A new request handle must
                                          // be created before retrying the function that originally produced this
                                          // error.
            break;
        case ERROR_WINHTTP_NAME_NOT_RESOLVED: // The server name cannot be resolved.
            break;
        case ERROR_WINHTTP_OPERATION_CANCELLED: // The operation was canceled, usually because the handle on which the
                                                // request was operating was closed before the operation completed.
            break;
        case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW: // Returned when an incoming response exceeds an internal WinHTTP
                                                    // size limit.
            break;
        case ERROR_WINHTTP_SECURE_FAILURE: // One or more errors were found in the Secure Sockets Layer (SSL)
                                           // certificate sent by the server. To determine what type of error was
                                           // encountered, verify through a WINHTTP_CALLBACK_STATUS_SECURE_FAILURE
                                           // notification in a status callback function. For more information, see
                                           // WINHTTP_STATUS_CALLBACK.
            break;
        case ERROR_WINHTTP_SHUTDOWN: // The WinHTTP function support is shut down or unloaded.
            break;
        case ERROR_WINHTTP_TIMEOUT: // The request timed out.
            break;
        case ERROR_WINHTTP_UNRECOGNIZED_SCHEME: // The URL specified a scheme other than "http:" or "https:".
            break;
        case ERROR_NOT_ENOUGH_MEMORY: // Not enough memory was available to complete the requested operation. (Windows
                                      // error code) Windows Server 2003, Windows XP and Windows 2000:  The TCP
                                      // reservation range set with the WINHTTP_OPTION_PORT_RESERVATION option is not
                                      // large enough to send this request.
            break;
        case ERROR_INVALID_PARAMETER: // The content length specified in the dwTotalLength parameter does not match the
                                      // length specified in the Content-Length header. The lpOptional parameter must be
                                      // NULL and the dwOptionalLength parameter must be zero when the Transfer-Encoding
                                      // header is present. The Content-Length header cannot be present when the
                                      // Transfer-Encoding header is present.
            break;
        case ERROR_WINHTTP_RESEND_REQUEST: // The application must call WinHttpSendRequest again due to a redirect or
                                           // authentication challenge. Windows Server 2003 with SP1, Windows XP with
                                           // SP2 and Windows 2000:  This error is not supported.
            break;
        default: break;
        }

        return tl::make_unexpected(HttpRequestError::PlatformError);
    }

    BOOL receivedResponse = WinHttpReceiveResponse(request.get(), nullptr);

    if(!receivedResponse)
    {
        DWORD error = GetLastError();

        switch(error)
        {
        case ERROR_WINHTTP_CANNOT_CONNECT: // Returned if connection to the server failed.
            break;
        case ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW: // Returned when an overflow condition is encountered
                                                                  // in the course of parsing chunked encoding.
            break;
        case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED: // Returned when the server requests client authentication.
            break;
        case ERROR_WINHTTP_CONNECTION_ERROR: // The connection with the server has been reset or terminated, or an
                                             // incompatible SSL protocol was encountered. For example, WinHTTP
                                             // version 5.1 does not support SSL2 unless the client specifically enables
                                             // it.
            break;
        case ERROR_WINHTTP_HEADER_COUNT_EXCEEDED: // Returned when a larger number of headers were present in a response
                                                  // than WinHTTP could receive.
            break;
        case ERROR_WINHTTP_HEADER_SIZE_OVERFLOW: // Returned by WinHttpReceiveResponse when the size of headers received
                                                 // exceeds the limit for the request handle.
            break;
        case ERROR_WINHTTP_INCORRECT_HANDLE_STATE: // The requested operation cannot be carried out because the handle
                                                   // supplied is not in the correct state.
            break;
        case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: // The type of handle supplied is incorrect for this operation.
            break;
        case ERROR_WINHTTP_INTERNAL_ERROR: // An internal error has occurred.
            break;
        case ERROR_WINHTTP_INVALID_SERVER_RESPONSE: // The server response could not be parsed.
            break;
        case ERROR_WINHTTP_INVALID_URL: // The URL is invalid.
            break;
        case ERROR_WINHTTP_LOGIN_FAILURE: // The login attempt failed. When this error is encountered, the request
                                          // handle should be closed with WinHttpCloseHandle. A new request handle must
                                          // be created before retrying the function that originally produced this
                                          // error.
            break;
        case ERROR_WINHTTP_NAME_NOT_RESOLVED: // The server name could not be resolved.
            break;
        case ERROR_WINHTTP_OPERATION_CANCELLED: // The operation was canceled, usually because the handle on which the
                                                // request was operating was closed before the operation completed.
            break;
        case ERROR_WINHTTP_REDIRECT_FAILED: // The redirection failed because either the scheme changed or all attempts
                                            // made to redirect failed (default is five attempts).
            break;
        case ERROR_WINHTTP_RESEND_REQUEST: // The WinHTTP function failed. The desired function can be retried on the
                                           // same request handle.
            break;
        case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW: // Returned when an incoming response exceeds an internal WinHTTP
                                                    // size limit.
            break;
        case ERROR_WINHTTP_SECURE_FAILURE: // One or more errors were found in the Secure Sockets Layer (SSL)
                                           // certificate sent by the server. To determine what type of error was
                                           // encountered, check for a WINHTTP_CALLBACK_STATUS_SECURE_FAILURE
                                           // notification in a status callback function. For more information, see
                                           // WINHTTP_STATUS_CALLBACK.
            break;
        case ERROR_WINHTTP_TIMEOUT: // The request has timed out.
            break;
        case ERROR_WINHTTP_UNRECOGNIZED_SCHEME: // The URL specified a scheme other than "http:" or "https:".
            break;
        case ERROR_NOT_ENOUGH_MEMORY: // Not enough memory was available to complete the requested operation. (Windows
                                      // error code)
            break;
        default: break;
        }

        return tl::make_unexpected(HttpRequestError::PlatformError);
    }

    std::vector<std::byte> buffer;
    std::vector<std::byte> fullResponse;
    DWORD responseSize;

    do
    {
        responseSize = 0;
        if(!WinHttpQueryDataAvailable(request.get(), &responseSize))
        {
            DWORD error = GetLastError();

            switch(error)
            {
            case ERROR_WINHTTP_CONNECTION_ERROR: // The connection with the server has been reset or terminated, or an
                                                 // incompatible SSL protocol was encountered. For example, WinHTTP
                                                 // version 5.1 does not support SSL2 unless the client specifically
                                                 // enables it.
                break;
            case ERROR_WINHTTP_INCORRECT_HANDLE_STATE: // The requested operation cannot complete because the handle
                                                       // supplied is not in the correct state.
                break;
            case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: // The type of handle supplied is incorrect for this operation.
                break;
            case ERROR_WINHTTP_INTERNAL_ERROR: // An internal error has occurred.
                break;
            case ERROR_WINHTTP_OPERATION_CANCELLED: // The operation was canceled, usually because the handle on which
                                                    // the request was operating was closed before the operation
                                                    // completed.
                break;
            case ERROR_WINHTTP_TIMEOUT: // The request has timed out.
                break;
            case ERROR_NOT_ENOUGH_MEMORY: // Not enough memory was available to complete the requested operation.
                                          // (Windows error code)
                break;
            default: break;
            }

            return tl::make_unexpected(HttpRequestError::PlatformError);
        }

        buffer.resize(responseSize);

        DWORD bytesRead = 0;
        if(!WinHttpReadData(request.get(), static_cast<LPVOID>(buffer.data()), responseSize, &bytesRead))
        {
            DWORD error = GetLastError();

            switch(error)
            {
            case ERROR_WINHTTP_CONNECTION_ERROR: // The connection with the server has been reset or terminated, or an
                                                 // incompatible SSL protocol was encountered. For example, WinHTTP 5.1
                                                 // does not support SSL2 unless the client specifically enables it.
                break;
            case ERROR_WINHTTP_INCORRECT_HANDLE_STATE: // The requested operation cannot be carried out because the
                                                       // handle supplied is not in the correct state.
                break;
            case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: // The type of handle supplied is incorrect for this operation.
                break;
            case ERROR_WINHTTP_INTERNAL_ERROR: // An internal error has occurred.
                break;
            case ERROR_WINHTTP_OPERATION_CANCELLED: // The operation was canceled, usually because the handle on which
                                                    // the request was operating was closed before the operation
                                                    // completed.
                break;
            case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW: // Returned when an incoming response exceeds an internal
                                                        // WinHTTP size limit.
                break;
            case ERROR_WINHTTP_TIMEOUT: // The request has timed out.
                break;
            case ERROR_NOT_ENOUGH_MEMORY: // Not enough memory was available to complete the requested operation.
                                          // (Windows error code)
                break;
            default: break;
            }

            return tl::make_unexpected(HttpRequestError::PlatformError);
        }

        fullResponse.insert(fullResponse.end(), buffer.cbegin(), buffer.cbegin() + bytesRead);
    } while(responseSize > 0);

    return fullResponse;
}
} // namespace shadercompile