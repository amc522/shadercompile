#include "shadercompile/dxc_release_manager.h"

#include "http_request.h"
#include "utility.h"

#include <libzippp/libzippp.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <string_view>

using namespace nlohmann;
using namespace std::string_view_literals;

namespace shadercompile
{
constexpr std::string_view kGitHubApiDomainName = "api.github.com"sv;
constexpr std::string_view kGitHubApiUrl = "https://api.github.com"sv;

DxcReleaseManager::DxcReleaseManager(const std::filesystem::path& dxcDownloadDirectory)
    : mDownloadDirectory(dxcDownloadDirectory)
{
    mGitHubRequest = std::make_unique<HttpRequest>(kGitHubApiDomainName);
}

DxcReleaseManager::~DxcReleaseManager() = default;

tl::expected<DxcVersion, std::errc> parseDxcVersionFromTagName(std::string_view tagName)
{
    DxcVersion version;

    if(tagName.empty()) { return tl::make_unexpected(std::errc::invalid_argument); }

    std::string_view versionString;
    if(tagName[0] == 'v') { versionString = tagName.substr(1); }
    else { versionString = tagName; }

    // major
    const size_t majorVersionEndIndex = versionString.find('.');
    if(majorVersionEndIndex == std::string_view::npos) { return tl::make_unexpected(std::errc::invalid_argument); }

    const std::from_chars_result majorVersionParseResult =
        std::from_chars(versionString.data(), versionString.data() + majorVersionEndIndex, version.major);

    if(majorVersionParseResult.ptr == versionString.data()) { return tl::make_unexpected(std::errc::invalid_argument); }

    // minor
    const size_t minorVersionEndIndex = versionString.find('.', majorVersionEndIndex + 1);
    if(minorVersionEndIndex == std::string_view::npos) { return tl::make_unexpected(std::errc::invalid_argument); }

    const std::from_chars_result minorVersionParseResult =
        std::from_chars(versionString.data() + majorVersionEndIndex + 1,
                        versionString.data() + minorVersionEndIndex,
                        version.minor);

    if(minorVersionParseResult.ptr == versionString.data()) { return tl::make_unexpected(std::errc::invalid_argument); }

    // micro
    size_t microVersionEndIndex = versionString.find('.', minorVersionEndIndex + 1);

    bool hasPatchNumber = true;
    if(microVersionEndIndex == std::string_view::npos)
    {
        hasPatchNumber = false;
        microVersionEndIndex = versionString.size();
    }

    std::from_chars_result microVersionParseResult = std::from_chars(versionString.data() + minorVersionEndIndex + 1,
                                                                     versionString.data() + microVersionEndIndex,
                                                                     version.micro);

    if(microVersionParseResult.ptr == versionString.data()) { return tl::make_unexpected(std::errc::invalid_argument); }

    if(!hasPatchNumber) { return version; }

    // patch
    size_t patchVersionEndIndex = versionString.size();
    std::from_chars_result patchVersionParseResult = std::from_chars(versionString.data() + microVersionEndIndex + 1,
                                                                     versionString.data() + patchVersionEndIndex,
                                                                     version.patch);

    if(patchVersionParseResult.ptr == versionString.data()) { return tl::make_unexpected(std::errc::invalid_argument); }

    return version;
}

tl::expected<DxcRelease, std::errc> parseDxcReleaseFromJson(const json& releaseJson)
{
    DxcRelease dxcRelease;

    const auto tagNameJsonItr = releaseJson.find("tag_name");
    if(tagNameJsonItr == releaseJson.end()) { return tl::make_unexpected(std::errc::invalid_argument); }

    if(!tagNameJsonItr->is_string()) { return tl::make_unexpected(std::errc::invalid_argument); }

    std::string_view tagName = tagNameJsonItr->get<std::string_view>();
    tl::expected<DxcVersion, std::errc> tagNameParseResult = parseDxcVersionFromTagName(tagName);

    if(!tagNameParseResult) { return tl::make_unexpected(std::errc::invalid_argument); }

    dxcRelease.version = tagNameParseResult.value();

    const auto assestsJsonItr = releaseJson.find("assets");
    if(assestsJsonItr == releaseJson.end()) { return tl::make_unexpected(std::errc::invalid_argument); }

    if(!assestsJsonItr->is_array()) { return tl::make_unexpected(std::errc::invalid_argument); }

    for(const json& assetJson : *assestsJsonItr)
    {
        if(!assetJson.is_object()) { continue; }

        const auto nameJsonItr = assetJson.find("name");
        if(nameJsonItr == assetJson.end()) { continue; }

        if(!nameJsonItr->is_string()) { continue; }

        const std::string_view name = nameJsonItr->get<std::string_view>();

        if(!(name.starts_with("dxc") && name.ends_with(".zip"))) { continue; }

        const auto urlJsonItr = assetJson.find("url");
        if(urlJsonItr == assetJson.end()) { continue; }

        if(!urlJsonItr->is_string()) { continue; }

        dxcRelease.zipAssetFileName = name;
        dxcRelease.zipAssetDownloadUrlPath = urlJsonItr->get<std::string_view>().substr(kGitHubApiUrl.size() + 1);
        break;
    }

    if(dxcRelease.zipAssetFileName.empty()) { return tl::make_unexpected(std::errc::invalid_argument); }

    return dxcRelease;
}

bool DxcReleaseManager::queryAvailableReleases()
{
    mGitHubRequest->openConnection();

    constexpr std::array requestHeaders = {
        HttpRequestHeader{.name = "Accept",               .value = "application/vnd.github+json"},
        HttpRequestHeader{.name = "X-GitHub-Api-Version", .value = "2022-11-28"                 },
    };

    mGitHubRequest->setHeaders(requestHeaders);
    auto releasesResponse = mGitHubRequest->get("repos/microsoft/DirectXShaderCompiler/releases");

    if(!releasesResponse) { return false; }

    const json releasesJsonArray = json::parse(releasesResponse.value());

    if(!releasesJsonArray.is_array()) { return false; }

    const auto releaseCount = releasesJsonArray.size();

    mAvailableVersions.clear();
    mReleases.clear();

    mAvailableVersions.reserve(releaseCount);
    mReleases.reserve(releaseCount);

    for(const json& releaseJson : releasesJsonArray)
    {
        tl::expected<DxcRelease, std::errc> dxcReleaseParseResult = parseDxcReleaseFromJson(releaseJson);

        if(dxcReleaseParseResult)
        {
            finishCreatingDxcRelease(dxcReleaseParseResult.value());
            mAvailableVersions.push_back(dxcReleaseParseResult->version);
            mReleases.push_back(std::move(dxcReleaseParseResult.value()));
        }
    }

    std::sort(mReleases.begin(),
              mReleases.end(),
              [](const auto& left, const auto& right) { return left.version > right.version; });

    std::sort(mAvailableVersions.begin(), mAvailableVersions.end(), std::greater());

    return !mReleases.empty();
}

tl::expected<DxcVersion, std::errc> DxcReleaseManager::queryLatestRelease()
{
    mGitHubRequest->openConnection();

    constexpr std::array requestHeaders = {
        HttpRequestHeader{.name = "Accept",               .value = "application/vnd.github+json"},
        HttpRequestHeader{.name = "X-GitHub-Api-Version", .value = "2022-11-28"                 },
    };

    mGitHubRequest->setHeaders(requestHeaders);
    auto latestReleaseResponse = mGitHubRequest->get("repos/microsoft/DirectXShaderCompiler/releases/latest");

    if(!latestReleaseResponse) { return tl::make_unexpected(std::errc::io_error); }

    const json releaseJson = json::parse(latestReleaseResponse.value());

    if(!releaseJson.is_object()) { return tl::make_unexpected(std::errc::invalid_argument); }

    tl::expected<DxcRelease, std::errc> parseLatestReleaseResult = parseDxcReleaseFromJson(releaseJson);

    if(!parseLatestReleaseResult) { return tl::make_unexpected(std::errc::invalid_argument); }

    finishCreatingDxcRelease(parseLatestReleaseResult.value());

    if(mAvailableVersions.empty())
    {
        mAvailableVersions.push_back(parseLatestReleaseResult->version);
        mReleases.push_back(std::move(parseLatestReleaseResult.value()));
    }
    else if(parseLatestReleaseResult->version > mAvailableVersions.front())
    {
        mAvailableVersions.insert(mAvailableVersions.begin(), parseLatestReleaseResult->version);
        mReleases.insert(mReleases.begin(), std::move(parseLatestReleaseResult.value()));
    }

    return mAvailableVersions.front();
}

bool DxcReleaseManager::queryRelease(DxcVersion version)
{
    {
        auto findItr = std::find(mAvailableVersions.begin(), mAvailableVersions.end(), version);

        if(findItr != mAvailableVersions.end()) { return true; }
    }

    mGitHubRequest->openConnection();

    constexpr std::array requestHeaders = {
        HttpRequestHeader{.name = "Accept",               .value = "application/vnd.github+json"},
        HttpRequestHeader{.name = "X-GitHub-Api-Version", .value = "2022-11-28"                 },
    };

    mGitHubRequest->setHeaders(requestHeaders);

    std::string path = "repos/microsoft/DirectXShaderCompiler/releases/tags/v";
    path.reserve(path.size() + 17);
    version.toString(std::back_inserter(path));

    auto releaseResponse = mGitHubRequest->get(path);

    if(!releaseResponse) { return false; }

    const json releaseJson = json::parse(releaseResponse.value());

    if(!releaseJson.is_object()) { return false; }

    tl::expected<DxcRelease, std::errc> parseReleaseResult = parseDxcReleaseFromJson(releaseJson);

    if(!parseReleaseResult) { return false; }

    finishCreatingDxcRelease(parseReleaseResult.value());

    mAvailableVersions.push_back(parseReleaseResult->version);
    mReleases.push_back(std::move(parseReleaseResult.value()));

    std::sort(mAvailableVersions.begin(), mAvailableVersions.end());
    std::sort(mReleases.begin(),
              mReleases.end(),
              [](const auto& left, const auto& right) { return left.version > right.version; });

    return true;
}

bool DxcReleaseManager::downloadRelease(DxcVersion version)
{
    auto findItr = std::find(mAvailableVersions.cbegin(), mAvailableVersions.cend(), version);

    if(findItr == mAvailableVersions.cend())
    {
        const bool queryReleaseSuccess = queryRelease(version);

        if(!queryReleaseSuccess) { return false; }

        findItr = std::find(mAvailableVersions.cbegin(), mAvailableVersions.cend(), version);
    }

    DxcRelease& release = *(mReleases.begin() + std::distance(mAvailableVersions.cbegin(), findItr));

    if(std::filesystem::exists(release.directory) && std::filesystem::exists(release.directory / "bin/x64/dxc.exe") &&
       std::filesystem::exists(release.directory / "bin/x64/dxcompiler.dll") &&
       std::filesystem::exists(release.directory / "bin/x64/dxil.dll") &&
       std::filesystem::exists(release.directory / "lib/x64/dxcompiler.lib") &&
       std::filesystem::exists(release.directory / "inc"))
    {
        return true;
    }

    mGitHubRequest->openConnection();

    constexpr std::array requestHeaders = {
        HttpRequestHeader{.name = "Accept",               .value = "application/octet-stream"},
        HttpRequestHeader{.name = "X-GitHub-Api-Version", .value = "2022-11-28"              },
    };

    mGitHubRequest->setHeaders(requestHeaders);
    tl::expected<std::vector<std::byte>, HttpRequestError> downloadResult =
        mGitHubRequest->get(release.zipAssetDownloadUrlPath);

    if(!downloadResult) { return false; }

    libzippp::ZipArchive* zipArchive =
        libzippp::ZipArchive::fromBuffer(downloadResult->data(),
                                         static_cast<libzippp_uint32>(downloadResult->size()),
                                         true);

    if(!zipArchive) { return false; }

    const auto cleanup = finally([zipArchive]() { libzippp::ZipArchive::free(zipArchive); });

    if(!zipArchive->isOpen()) { return false; }

    std::filesystem::create_directory(release.directory);

    for(const libzippp::ZipEntry& entry : zipArchive->getEntries())
    {
        if(entry.isNull()) { continue; }

        if(entry.isDirectory())
        {
            std::filesystem::path directoryPath = release.directory / entry.getName();
            if(!std::filesystem::exists(directoryPath)) { std::filesystem::create_directory(directoryPath); }

            continue;
        }

        if(!entry.isFile()) { continue; }

        std::ofstream fileStream(release.directory / entry.getName(), std::ios_base::out | std::ios_base::binary);
        if(!fileStream.is_open()) { return false; }

        int result = entry.readContent(fileStream);

        if(result != LIBZIPPP_OK) { return false; }
    }

    release.downloaded = true;

    return true;
}

bool DxcReleaseManager::downloadLatestRelease()
{
    tl::expected<DxcVersion, std::errc> queryResult = queryLatestRelease();

    if(!queryResult) { return false; }

    return downloadRelease(queryResult.value());
}

bool DxcReleaseManager::isReleaseAvailable(DxcVersion version) const
{
    return std::find(mAvailableVersions.cbegin(), mAvailableVersions.cend(), version) != mAvailableVersions.cend();
}

void DxcReleaseManager::finishCreatingDxcRelease(DxcRelease& release)
{
    std::string folderName;
    // dxc-##.##.####.## = 17 chars
    folderName.reserve(17);
    folderName.append("dxc-");
    release.version.toString(std::back_inserter(folderName));
    release.directory = mDownloadDirectory / folderName;
    release.downloaded =
        std::filesystem::exists(release.directory) && std::filesystem::exists(release.directory / "dxc.exe");
}

const std::filesystem::path& DxcReleaseManager::getReleaseDirectory(DxcVersion version) const
{
    auto findItr = std::find(mAvailableVersions.cbegin(), mAvailableVersions.cend(), version);

    if(findItr == mAvailableVersions.cend())
    {
        static const std::filesystem::path emptyPath;
        return emptyPath;
    }

    return (mReleases.begin() + std::distance(mAvailableVersions.cbegin(), findItr))->directory;
}
} // namespace shadercompile