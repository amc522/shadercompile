#pragma once

#include <tl/expected.hpp>

#include <charconv>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace shadercompile
{
struct DxcVersion
{
    int major = -1;
    int minor = -1;
    int micro = -1;
    int patch = 0;

    [[nodiscard]] auto operator<=>(const DxcVersion& right) const noexcept = default;

    [[nodiscard]] bool isValid() const noexcept { return major != -1 && minor != -1 && micro != -1 && patch >= 0; }

    template<class OutputItr>
        requires std::output_iterator<OutputItr, char>
    void toString(OutputItr outputItr) const
    {
        auto convertToChars = [&](int value)
        {
            char buffer[21];
            std::to_chars_result result = std::to_chars(&buffer[0], &buffer[0] + 21, value);
            if(result.ec == std::errc::value_too_large) { return; }

            for(char* ptr = &buffer[0]; ptr != result.ptr; ++ptr)
            {
                (*outputItr) = *ptr;
            }
        };

        convertToChars(major);
        outputItr = '.';
        convertToChars(minor);
        outputItr = '.';
        convertToChars(micro);
        if(patch > 0)
        {
            outputItr = '.';
            convertToChars(patch);
        }
    }

    [[nodiscard]] std::string toString() const
    {
        std::string str;
        toString(std::back_inserter(str));
        return str;
    }
};

struct DxcRelease
{
    DxcVersion version;
    std::string zipAssetDownloadUrlPath;
    std::string zipAssetFileName;
    std::filesystem::path directory;
    bool downloaded = false;
};

class HttpRequest;

class DxcReleaseManager
{
public:
    DxcReleaseManager(const std::filesystem::path& dxcDownloadDirectory);
    ~DxcReleaseManager();

    bool queryAvailableReleases();
    [[nodiscard]] tl::expected<DxcVersion, std::errc> queryLatestRelease();
    bool queryRelease(DxcVersion version);

    bool downloadRelease(DxcVersion version);
    bool downloadLatestRelease();

    std::span<const DxcVersion> availableReleases() const { return mAvailableVersions; }

    [[nodiscard]] std::optional<DxcVersion> getLastestReleaseVersion() const
    {
        if(mAvailableVersions.empty()) { return std::nullopt; }

        return mAvailableVersions.front();
    }

    [[nodiscard]] bool isReleaseAvailable(DxcVersion version) const;
    [[nodiscard]] const std::filesystem::path& getReleaseDirectory(DxcVersion version) const;

private:
    void finishCreatingDxcRelease(DxcRelease& release);

    std::filesystem::path mDownloadDirectory;
    std::vector<DxcVersion> mAvailableVersions;
    std::vector<DxcRelease> mReleases;
    std::unique_ptr<HttpRequest> mGitHubRequest;
};
} // namespace shadercompile