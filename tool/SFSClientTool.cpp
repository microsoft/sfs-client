// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/SFSClient.h"

#include <iostream>
#include <vector>

using namespace SFS;

namespace
{
void DisplayUsage()
{
    std::cout
        << "Usage: SFSClientTool --productName name [options]" << std::endl
        << "Options:" << std::endl
        << "  -h, --help\t\tDisplay this help message" << std::endl
        << "  --accountId\t\tThe account ID of the SFS service is used to identify the caller" << std::endl
        << "  --instanceId\t\tThe instance ID is usually \"Default\" (optional)" << std::endl
        << "  --nameSpace\t\tThe namespace is usually \"Default\" (optional)" << std::endl
        << "  --productName\t\tThe name of the product to retrieve" << std::endl
        << "Example:" << std::endl
        << "  SFSClientTool --accountId test --productName \"Microsoft.WindowsStore_12011.1001.1.0_x64__8wekyb3d8bbwe\""
        << std::endl;
}

void DisplayHelp()
{
    std::cout << "SFSClient Tool" << std::endl
              << "Copyright (c) Microsoft Corporation. All rights reserved." << std::endl
              << std::endl
              << "Use to interact with the SFS service through the SFS Client library." << std::endl
              << std::endl;
    DisplayUsage();
}

std::string redStart = "\033[1;31m";
std::string redEnd = "\033[0m";

void PrintError(std::string_view message)
{
    std::cout << redStart << message << redEnd << std::endl;
}

struct Settings
{
    bool displayHelp{true};
    std::string accountId;
    std::string instanceId;
    std::string nameSpace;
    std::string productName;
};

int ParseArguments(const std::vector<std::string_view>& args, Settings& settings)
{
    settings = {};
    settings.displayHelp = args.size() == 1;

    for (int i = 1; i < args.size(); ++i)
    {
        if (args[i].compare("-h") == 0 || args[i].compare("--help") == 0)
        {
            settings.displayHelp = true;
        }
        else if (args[i].compare("--accountId") == 0)
        {
            if (args.size() <= i + 1)
            {
                PrintError("Missing accountId argument.");
                return 1;
            }
            settings.accountId = args[++i];
        }
        else if (args[i].compare("--instanceId") == 0)
        {
            if (args.size() <= i + 1)
            {
                PrintError("Missing instanceId argument.");
                return 1;
            }
            settings.instanceId = args[++i];
        }
        else if (args[i].compare("--nameSpace") == 0)
        {
            if (args.size() <= i + 1)
            {
                PrintError("Missing nameSpace argument.");
                return 1;
            }
            settings.nameSpace = args[++i];
        }
        else if (args[i].compare("--productName") == 0)
        {
            if (args.size() <= i + 1)
            {
                PrintError("Missing productName argument.");
                return 1;
            }
            settings.productName = args[++i];
        }
        else
        {
            PrintError("Unknown option " + std::string(args[i]) + ".\n");
            return 1;
        }
    }
    return 0;
}

constexpr std::string_view ToString(HashType type)
{
    switch (type)
    {
    case HashType::Sha1:
        return "Sha1";
    case HashType::Sha256:
        return "Sha256";
    }
    return "";
}

void DisplayResults(const Contents& responseContents)
{
    if (responseContents.empty())
    {
        std::cout << "No results found." << std::endl;
        return;
    }

    auto comma = [](size_t count, size_t size) -> std::string { return count == size - 1 ? "" : ","; };

    std::cout << "Contents found: " << responseContents.size() << std::endl;
    size_t contentCount = 0;
    std::cout << "{" << std::endl;
    for (const auto& content : responseContents)
    {
        std::cout << R"(  "Content )" << contentCount << R"(": {)" << std::endl;
        std::cout << R"(    "NameSpace": ")" << content->GetContentId().GetNameSpace() << R"(",)" << std::endl;
        std::cout << R"(    "Name": ")" << content->GetContentId().GetName() << R"(",)" << std::endl;
        std::cout << R"(    "Version": ")" << content->GetContentId().GetVersion() << R"(",)" << std::endl;
        std::cout << R"(    "CorrelationVector": ")" << content->GetCorrelationVector() << R"(",)" << std::endl;

        if (content->GetFiles().size() == 0)
        {
            std::cout << R"(    "Files": [])" << std::endl;
        }
        else
        {
            std::cout << R"(    "Files": [)" << std::endl;

            size_t fileCount = 0;
            for (const auto& file : content->GetFiles())
            {
                std::cout << R"(      {)" << std::endl;
                std::cout << R"(        "FileId": ")" << file->GetFileId() << R"(",)" << std::endl;
                std::cout << R"(        "Url": ")" << file->GetUrl() << R"(",)" << std::endl;
                std::cout << R"(        "SizeInBytes": ")" << file->GetSizeInBytes() << R"(",)" << std::endl;
                std::cout << R"(        "Hashes": {)" << std::endl;
                size_t hashCount = 0;
                for (const auto& hash : file->GetHashes())
                {
                    std::cout << "          \"" << ToString(hash.first) << R"(": ")" << hash.second << '"'
                              << comma(hashCount++, file->GetHashes().size()) << std::endl;
                }
                std::cout << R"(        })" << std::endl;
                std::cout << R"(      })" << comma(fileCount++, content->GetFiles().size()) << std::endl;
            }
            std::cout << R"(    ])" << std::endl;
        }
        std::cout << R"(  })" << comma(contentCount++, responseContents.size()) << std::endl;
    }
    std::cout << R"(})" << std::endl;
}

void LogResult(const SFS::Result& result)
{
    std::cout << "Result code: " << ToString(result.GetCode());
    if (!result.GetMessage().empty())
    {
        std::cout << " Message: " << result.GetMessage();
    }
    std::cout << std::endl;
}

bool TestMake()
{
    std::cout << std::endl << "Testing Make methods." << std::endl;

    std::unique_ptr<File> file1;
    if (File::Make("fileId1", "url1", 1 /*sizeInBytes*/, {{HashType::Sha1, "sha1"}}, file1) != Result::S_Ok)
    {
        std::cout << "Failed to create file1." << std::endl;
        return false;
    }

    std::unique_ptr<File> file2;
    if (File::Make("fileId2", "url2", 1 /*sizeInBytes*/, {{HashType::Sha256, "sha256"}}, file2) != Result::S_Ok)
    {
        std::cout << "Failed to create file2." << std::endl;
        return false;
    }

    std::vector<std::unique_ptr<File>> files1;
    files1.push_back(std::move(file1));
    files1.push_back(std::move(file2));

    std::unique_ptr<Content> content1;
    if (!Content::Make("contentNameSpace", "contentName", "contentVersion", "correlationVector", files1, content1))
    {
        return false;
    }

    std::unique_ptr<File> file3;
    if (File::Make("fileId3", "url3", 3 /*sizeInBytes*/, {{HashType::Sha1, "sha1"}}, file1) != Result::S_Ok)
    {
        std::cout << "Failed to create file3." << std::endl;
        return false;
    }

    std::vector<std::unique_ptr<File>> files2;
    files1.push_back(std::move(file3));

    std::unique_ptr<Content> content2;
    if (!Content::Make("contentNameSpace2", "contentName2", "contentVersion2", "correlationVector2", files2, content2))
    {
        return false;
    }

    Contents contents;
    contents.push_back(std::move(content1));
    contents.push_back(std::move(content2));

    DisplayResults(contents);

    return true;
}
} // namespace

int main(int argc, char* argv[])
{
    Settings settings;
    if (ParseArguments(std::vector<std::string_view>(argv, argv + argc), settings) != 0)
    {
        DisplayUsage();
        return 1;
    }

    if (settings.displayHelp || settings.productName.empty())
    {
        DisplayHelp();
        return 0;
    }

    // Initialize SFSClient
    std::cout << "Initializing SFSClient with accountId: " << settings.accountId
              << ", instanceId: " << settings.instanceId << ", nameSpace: " << settings.nameSpace << std::endl
              << std::endl;
    std::unique_ptr<SFSClient> sfsClient;
    auto result = SFSClient::Make(settings.accountId, settings.instanceId, settings.nameSpace, sfsClient);
    if (!result)
    {
        std::cout << "Failed to initialize SFSClient.";
        LogResult(result);
        return result.GetCode();
    }

    // Perform operations using SFSClient
    std::cout << "Getting latest download info for product: " << settings.productName << std::endl;
    Contents responseContents;
    result = sfsClient->GetLatestDownloadInfo(settings.productName, responseContents);
    if (!result)
    {
        std::cout << "Failed to get latest download info." << std::endl;
        LogResult(result);
        // return result.GetCode();
    }

    // Display results
    DisplayResults(responseContents);

    // Testing - move to unit tests later
    if (TestMake())
    {
        return 1;
    }

    return 0;
}
