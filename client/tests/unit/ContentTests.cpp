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
}

TEST("Testing FileId::Make()")
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
}

TEST_SCENARIO("Testing Content::Make()")
{
    GIVEN("Elements that make up a content")
    {
        std::string contentNameSpace{"myNameSpace"};
        std::string contentName{"myName"};
        std::string contentVersion{"myVersion"};
        std::string correlationVector{"myCorrelationVector"};

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

        WHEN("A Content is created by copying the arguments")
        {
            std::unique_ptr<Content> copiedContent;
            REQUIRE(
                Content::Make(contentNameSpace, contentName, contentVersion, correlationVector, files, copiedContent) ==
                Result::S_Ok);
            REQUIRE(copiedContent != nullptr);

            THEN("The content elements are copies")
            {
                CHECK(contentNameSpace == copiedContent->GetContentId().GetNameSpace());
                CHECK(contentName == copiedContent->GetContentId().GetName());
                CHECK(contentVersion == copiedContent->GetContentId().GetVersion());
                CHECK(correlationVector == copiedContent->GetCorrelationVector());

                // Files were cloned, so the pointers are different, but the contents should be similar
                REQUIRE(files.size() == copiedContent->GetFiles().size());
                REQUIRE(filePointers.size() == copiedContent->GetFiles().size());
                for (size_t i = 0; i < files.size(); ++i)
                {
                    // Checking ptrs
                    REQUIRE(filePointers[i] != copiedContent->GetFiles()[i].get());
                    REQUIRE(files[i] != copiedContent->GetFiles()[i]);

                    // Checking contents
                    CHECK(files[i]->GetFileId() == copiedContent->GetFiles()[i]->GetFileId());
                    CHECK(files[i]->GetUrl() == copiedContent->GetFiles()[i]->GetUrl());
                    CHECK(files[i]->GetSizeInBytes() == copiedContent->GetFiles()[i]->GetSizeInBytes());
                    CHECK(files[i]->GetHashes() == copiedContent->GetFiles()[i]->GetHashes());
                }
            }

            AND_THEN("Using the Make that moves arguments really moves the arguments")
            {
                std::unique_ptr<Content> movedContent;
                REQUIRE(Content::Make(std::move(contentNameSpace),
                                      std::move(contentName),
                                      std::move(contentVersion),
                                      std::move(correlationVector),
                                      std::move(files),
                                      movedContent) == Result::S_Ok);
                REQUIRE(movedContent != nullptr);

                CHECK(copiedContent->GetContentId().GetNameSpace() == movedContent->GetContentId().GetNameSpace());
                CHECK(copiedContent->GetContentId().GetName() == movedContent->GetContentId().GetName());
                CHECK(copiedContent->GetContentId().GetVersion() == movedContent->GetContentId().GetVersion());
                CHECK(copiedContent->GetCorrelationVector() == movedContent->GetCorrelationVector());

                REQUIRE(filePointers.size() == movedContent->GetFiles().size());
                REQUIRE(copiedContent->GetFiles().size() == movedContent->GetFiles().size());
                for (size_t i = 0; i < filePointers.size(); ++i)
                {
                    // Checking ptrs
                    REQUIRE(copiedContent->GetFiles()[i] != movedContent->GetFiles()[i]);
                    REQUIRE(filePointers[i] == movedContent->GetFiles()[i].get());

                    // Checking contents
                    CHECK(copiedContent->GetFiles()[i]->GetFileId() == movedContent->GetFiles()[i]->GetFileId());
                    CHECK(copiedContent->GetFiles()[i]->GetFileId() == movedContent->GetFiles()[i]->GetFileId());
                    CHECK(copiedContent->GetFiles()[i]->GetUrl() == movedContent->GetFiles()[i]->GetUrl());
                    CHECK(copiedContent->GetFiles()[i]->GetSizeInBytes() ==
                          movedContent->GetFiles()[i]->GetSizeInBytes());
                    CHECK(copiedContent->GetFiles()[i]->GetHashes() == movedContent->GetFiles()[i]->GetHashes());
                }
            }
        }
    }
}
