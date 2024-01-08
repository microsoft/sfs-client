// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/Content.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[ContentTests] " __VA_ARGS__)
#define TEST_SCENARIO(...) TEST_CASE("[ContentTests] Scenario: " __VA_ARGS__)

using namespace SFS;

TEST("Testing ContentId::Make()")
{
    std::unique_ptr<ContentId> contentId;

    const std::string nameSpace{"myNameSpace"};
    const std::string name{"myName"};
    const std::string version{"myVersion"};

    REQUIRE(ContentId::Make(nameSpace, name, version, contentId) == Result::S_Ok);
    REQUIRE(contentId != nullptr);

    CHECK(nameSpace == contentId->GetNameSpace());
    CHECK(name == contentId->GetName());
    CHECK(version == contentId->GetVersion());

    SECTION("Testing ContentId equality operators")
    {
        auto CompareContentId = [&contentId](const std::string& nameSpace,
                                             const std::string& name,
                                             const std::string& version,
                                             bool isEqual) {
            std::unique_ptr<ContentId> otherContentId;
            REQUIRE(ContentId::Make(nameSpace, name, version, otherContentId) == Result::S_Ok);
            REQUIRE(otherContentId != nullptr);

            if (isEqual)
            {
                REQUIRE(*contentId == *otherContentId);
                REQUIRE_FALSE(*contentId != *otherContentId);
            }
            else
            {
                REQUIRE(*contentId != *otherContentId);
                REQUIRE_FALSE(*contentId == *otherContentId);
            }
        };

        CompareContentId(nameSpace, name, version, true /*isEqual*/);
        CompareContentId("", name, version, false /*isEqual*/);
        CompareContentId(nameSpace, "", version, false /*isEqual*/);
        CompareContentId(nameSpace, name, "", false /*isEqual*/);
        CompareContentId("", "", "", false /*isEqual*/);
    }
}

TEST("Testing File::Make()")
{
    std::unique_ptr<File> file;

    const std::string fileId{"myFileId"};
    const std::string url{"myUrl"};
    const uint64_t sizeInBytes{1234};
    const std::unordered_map<HashType, std::string> hashes{{HashType::Sha1, "mySha1"}, {HashType::Sha256, "mySha256"}};

    REQUIRE(File::Make(fileId, url, sizeInBytes, hashes, file) == Result::S_Ok);
    REQUIRE(file != nullptr);

    CHECK(fileId == file->GetFileId());
    CHECK(url == file->GetUrl());
    CHECK(sizeInBytes == file->GetSizeInBytes());
    CHECK(hashes == file->GetHashes());

    SECTION("Testing File equality operators")
    {
        auto CompareFile = [&file](const std::string& fileId,
                                   const std::string& url,
                                   uint64_t sizeInBytes,
                                   const std::unordered_map<HashType, std::string>& hashes,
                                   bool isEqual) {
            std::unique_ptr<File> otherFile;
            REQUIRE(File::Make(fileId, url, sizeInBytes, hashes, otherFile) == Result::S_Ok);
            REQUIRE(otherFile != nullptr);

            if (isEqual)
            {
                REQUIRE(*file == *otherFile);
                REQUIRE_FALSE(*file != *otherFile);
            }
            else
            {
                REQUIRE(*file != *otherFile);
                REQUIRE_FALSE(*file == *otherFile);
            }
        };

        CompareFile(fileId, url, sizeInBytes, hashes, true /*isEqual*/);
        CompareFile("", url, sizeInBytes, hashes, false /*isEqual*/);
        CompareFile(fileId, "", sizeInBytes, hashes, false /*isEqual*/);
        CompareFile(fileId, url, 0, hashes, false /*isEqual*/);
        CompareFile(fileId, url, sizeInBytes, {}, false /*isEqual*/);
        CompareFile("", "", 0, {}, false /*isEqual*/);
    }
}

TEST_SCENARIO("Testing Content::Make()")
{
    GIVEN("Elements that make up a content")
    {
        const std::string contentNameSpace{"myNameSpace"};
        const std::string contentName{"myName"};
        const std::string contentVersion{"myVersion"};

        std::unique_ptr<File> file1;
        REQUIRE(File::Make("fileId1", "url1", 1 /*sizeInBytes*/, {{HashType::Sha1, "sha1"}}, file1) == Result::S_Ok);

        std::unique_ptr<File> file2;
        REQUIRE(File::Make("fileId2", "url2", 1 /*sizeInBytes*/, {{HashType::Sha256, "sha256"}}, file2) ==
                Result::S_Ok);

        std::vector<std::unique_ptr<File>> files;
        files.push_back(std::move(file1));
        files.push_back(std::move(file2));

        // Getting raw pointers to check they don't match after copy but match after move
        std::vector<File*> filePointers;
        for (const auto& file : files)
        {
            filePointers.push_back(file.get());
        }

        WHEN("A Content is created by copying the parameters")
        {
            std::unique_ptr<Content> copiedContent;
            REQUIRE(Content::Make(contentNameSpace, contentName, contentVersion, files, copiedContent) == Result::S_Ok);
            REQUIRE(copiedContent != nullptr);

            THEN("The content elements are copies")
            {
                CHECK(contentNameSpace == copiedContent->GetContentId().GetNameSpace());
                CHECK(contentName == copiedContent->GetContentId().GetName());
                CHECK(contentVersion == copiedContent->GetContentId().GetVersion());

                // Files were cloned, so the pointers are different, but the contents should be similar
                REQUIRE(files.size() == copiedContent->GetFiles().size());
                REQUIRE(filePointers.size() == copiedContent->GetFiles().size());
                for (size_t i = 0; i < files.size(); ++i)
                {
                    // Checking ptrs
                    REQUIRE(filePointers[i] != copiedContent->GetFiles()[i].get());
                    REQUIRE(files[i] != copiedContent->GetFiles()[i]);

                    // Checking contents
                    CHECK(*files[i] == *copiedContent->GetFiles()[i]);
                }
            }

            AND_THEN("Using the Make() that moves the file parameter really moves the parameter")
            {
                std::unique_ptr<Content> movedContent;
                REQUIRE(Content::Make(contentNameSpace, contentName, contentVersion, std::move(files), movedContent) ==
                        Result::S_Ok);
                REQUIRE(movedContent != nullptr);

                // Checking contents
                CHECK(*copiedContent == *movedContent);

                // Checking underlying pointers are the same since they were moved
                REQUIRE(filePointers.size() == movedContent->GetFiles().size());
                REQUIRE(copiedContent->GetFiles().size() == movedContent->GetFiles().size());
                for (size_t i = 0; i < filePointers.size(); ++i)
                {
                    REQUIRE(copiedContent->GetFiles()[i] != movedContent->GetFiles()[i]);
                    REQUIRE(filePointers[i] == movedContent->GetFiles()[i].get());
                }
            }
        }
    }
}

TEST("Testing Content equality operators")
{
    const std::string contentNameSpace{"myNameSpace"};
    const std::string contentName{"myName"};
    const std::string contentVersion{"myVersion"};
    const std::string correlationVector{"myCorrelationVector"};

    std::unique_ptr<File> file;
    REQUIRE(File::Make("fileId", "url", 1 /*sizeInBytes*/, {{HashType::Sha1, "sha1"}}, file) == Result::S_Ok);

    std::unique_ptr<File> clonedFile;
    REQUIRE(file->Clone(clonedFile) == Result::S_Ok);

    std::vector<std::unique_ptr<File>> files;
    files.push_back(std::move(file));

    std::vector<std::unique_ptr<File>> clonedFiles;
    clonedFiles.push_back(std::move(clonedFile));

    std::unique_ptr<Content> content;
    REQUIRE(Content::Make(contentNameSpace, contentName, contentVersion, files, content) == Result::S_Ok);

    auto CompareContent = [&content](const std::string& contentNameSpace,
                                     const std::string& contentName,
                                     const std::string& contentVersion,
                                     const std::vector<std::unique_ptr<File>>& files,
                                     bool isEqual) {
        std::unique_ptr<Content> otherContent;
        REQUIRE(Content::Make(contentNameSpace, contentName, contentVersion, files, otherContent) == Result::S_Ok);
        REQUIRE(otherContent != nullptr);

        if (isEqual)
        {
            REQUIRE(*content == *otherContent);
            REQUIRE_FALSE(*content != *otherContent);
        }
        else
        {
            REQUIRE(*content != *otherContent);
            REQUIRE_FALSE(*content == *otherContent);
        }
    };

    CompareContent(contentNameSpace, contentName, contentVersion, files, true /*isEqual*/);
    CompareContent(contentNameSpace, contentName, contentVersion, clonedFiles, true /*isEqual*/);
    CompareContent("", contentName, contentVersion, files, false /*isEqual*/);
    CompareContent(contentNameSpace, "", contentVersion, files, false /*isEqual*/);
    CompareContent(contentNameSpace, contentName, "", files, false /*isEqual*/);
    CompareContent(contentNameSpace, contentName, contentVersion, {}, false /*isEqual*/);
    CompareContent("", "", "", {}, false /*isEqual*/);
}
