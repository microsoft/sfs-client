// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/Content.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace SFS;

TEST(ContentIdTests, Make)
{
    std::unique_ptr<ContentId> contentId;

    const std::string nameSpace{"myNameSpace"};
    const std::string name{"myName"};
    const std::string version{"myVersion"};

    ASSERT_EQ(ContentId::Make(nameSpace, name, version, contentId).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, contentId);

    EXPECT_EQ(nameSpace, contentId->GetNameSpace());
    EXPECT_EQ(name, contentId->GetName());
    EXPECT_EQ(version, contentId->GetVersion());
}

TEST(FileIdTests, Make)
{
    std::unique_ptr<File> file;

    const std::string fileId{"myFileId"};
    const std::string url{"myUrl"};
    const uint64_t sizeInBytes{1234};
    const std::unordered_map<HashType, std::string> hashes{{HashType::Sha1, "mySha1"}, {HashType::Sha256, "mySha256"}};

    ASSERT_EQ(File::Make(fileId, url, sizeInBytes, hashes, file).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, file);

    EXPECT_EQ(fileId, file->GetFileId());
    EXPECT_EQ(url, file->GetUrl());
    EXPECT_EQ(sizeInBytes, file->GetSizeInBytes());
    EXPECT_EQ(hashes, file->GetHashes());
}

TEST(ContentTests, Make)
{
    // 1. Testing Make that copies arguments

    const std::string contentNameSpace{"myNameSpace"};
    const std::string contentName{"myName"};
    const std::string contentVersion{"myVersion"};
    const std::string correlationVector{"myCorrelationVector"};

    std::unique_ptr<File> file1;
    ASSERT_EQ(File::Make("fileId1", "url1", 1 /*sizeInBytes*/, {{HashType::Sha1, "sha1"}}, file1).GetCode(),
              Result::S_Ok);

    std::unique_ptr<File> file2;
    ASSERT_EQ(File::Make("fileId2", "url2", 1 /*sizeInBytes*/, {{HashType::Sha256, "sha256"}}, file2).GetCode(),
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

    std::unique_ptr<Content> copiedContent;
    ASSERT_EQ(
        Content::Make(contentNameSpace, contentName, contentVersion, correlationVector, files, copiedContent).GetCode(),
        Result::S_Ok);
    ASSERT_NE(nullptr, copiedContent);

    EXPECT_EQ(contentNameSpace, copiedContent->GetContentId().GetNameSpace());
    EXPECT_EQ(contentName, copiedContent->GetContentId().GetName());
    EXPECT_EQ(contentVersion, copiedContent->GetContentId().GetVersion());
    EXPECT_EQ(correlationVector, copiedContent->GetCorrelationVector());

    // Files were cloned, so the pointers are different, but the contents should be similar
    ASSERT_EQ(files.size(), copiedContent->GetFiles().size());
    ASSERT_EQ(filePointers.size(), copiedContent->GetFiles().size());
    for (size_t i = 0; i < files.size(); ++i)
    {
        // Checking ptrs
        EXPECT_NE(filePointers[i], copiedContent->GetFiles()[i].get());
        EXPECT_NE(files[i], copiedContent->GetFiles()[i]);

        // Checking contents
        EXPECT_EQ(files[i]->GetFileId(), copiedContent->GetFiles()[i]->GetFileId());
        EXPECT_EQ(files[i]->GetUrl(), copiedContent->GetFiles()[i]->GetUrl());
        EXPECT_EQ(files[i]->GetSizeInBytes(), copiedContent->GetFiles()[i]->GetSizeInBytes());
        EXPECT_EQ(files[i]->GetHashes(), copiedContent->GetFiles()[i]->GetHashes());
    }

    // 2. Testing Make that moves arguments

    std::unique_ptr<Content> movedContent;
    ASSERT_EQ(Content::Make(std::move(contentNameSpace),
                            std::move(contentName),
                            std::move(contentVersion),
                            std::move(correlationVector),
                            std::move(files),
                            movedContent)
                  .GetCode(),
              Result::S_Ok);
    ASSERT_NE(nullptr, movedContent);

    EXPECT_EQ(copiedContent->GetContentId().GetNameSpace(), movedContent->GetContentId().GetNameSpace());
    EXPECT_EQ(copiedContent->GetContentId().GetName(), movedContent->GetContentId().GetName());
    EXPECT_EQ(copiedContent->GetContentId().GetVersion(), movedContent->GetContentId().GetVersion());
    EXPECT_EQ(copiedContent->GetCorrelationVector(), movedContent->GetCorrelationVector());

    ASSERT_EQ(filePointers.size(), movedContent->GetFiles().size());
    ASSERT_EQ(copiedContent->GetFiles().size(), movedContent->GetFiles().size());
    for (size_t i = 0; i < filePointers.size(); ++i)
    {
        // Checking ptrs
        EXPECT_NE(copiedContent->GetFiles()[i], movedContent->GetFiles()[i]);
        EXPECT_EQ(filePointers[i], movedContent->GetFiles()[i].get());

        // Checking contents
        EXPECT_EQ(copiedContent->GetFiles()[i]->GetFileId(), movedContent->GetFiles()[i]->GetFileId());
        EXPECT_EQ(copiedContent->GetFiles()[i]->GetFileId(), movedContent->GetFiles()[i]->GetFileId());
        EXPECT_EQ(copiedContent->GetFiles()[i]->GetUrl(), movedContent->GetFiles()[i]->GetUrl());
        EXPECT_EQ(copiedContent->GetFiles()[i]->GetSizeInBytes(), movedContent->GetFiles()[i]->GetSizeInBytes());
        EXPECT_EQ(copiedContent->GetFiles()[i]->GetHashes(), movedContent->GetFiles()[i]->GetHashes());
    }
}
