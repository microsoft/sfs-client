// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "FileIdHelpers.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <set>

#define TEST(...) TEST_CASE("[FileIdHelperTests] " __VA_ARGS__)

using SFS::sample::FileIdToFileName;
using SFS::sample::IsValidFileId;

//
// IsValidFileId
//

TEST("IsValidFileId accepts normal identifiers")
{
    CHECK(IsValidFileId("testProduct.json"));
    CHECK(IsValidFileId("testProduct.bin"));
    CHECK(IsValidFileId("file-id"));
    CHECK(IsValidFileId("a_b"));
    CHECK(IsValidFileId("A9"));
    CHECK(IsValidFileId("a.b.c"));
}

TEST("IsValidFileId accepts dot-containing identifiers that are valid on the backend")
{
    CHECK(IsValidFileId("."));
    CHECK(IsValidFileId(".."));
    CHECK(IsValidFileId("..."));
    CHECK(IsValidFileId(".hidden"));
    CHECK(IsValidFileId("a..b"));
    CHECK(IsValidFileId("file."));
}

TEST("IsValidFileId rejects empty identifiers")
{
    CHECK_FALSE(IsValidFileId(""));
}

TEST("IsValidFileId rejects path traversal attempts")
{
    CHECK_FALSE(IsValidFileId("../x"));
    CHECK_FALSE(IsValidFileId("..\\x"));
    CHECK_FALSE(IsValidFileId("../.env"));
    CHECK_FALSE(IsValidFileId("..\\foo\\Desktop Alert\\uuid"));
}

TEST("IsValidFileId rejects absolute and drive-qualified paths")
{
    CHECK_FALSE(IsValidFileId("/etc/passwd"));
    CHECK_FALSE(IsValidFileId("/tmp/file"));
    CHECK_FALSE(IsValidFileId("C:\\file"));
    CHECK_FALSE(IsValidFileId("C:file"));
    CHECK_FALSE(IsValidFileId("\\\\server\\share\\file"));
}

TEST("IsValidFileId rejects whitespace, control characters and non-ASCII")
{
    CHECK_FALSE(IsValidFileId("a b"));
    CHECK_FALSE(IsValidFileId("a\tb"));
    CHECK_FALSE(IsValidFileId("file\n"));
    CHECK_FALSE(IsValidFileId("\xc3\xa9"));
    CHECK_FALSE(IsValidFileId(std::string("a\0b", 3)));
}

TEST("IsValidFileId rejects other special characters")
{
    CHECK_FALSE(IsValidFileId("a*b"));
    CHECK_FALSE(IsValidFileId("a?b"));
    CHECK_FALSE(IsValidFileId("a<b"));
    CHECK_FALSE(IsValidFileId("a>b"));
    CHECK_FALSE(IsValidFileId("a|b"));
    CHECK_FALSE(IsValidFileId("a\"b"));
    CHECK_FALSE(IsValidFileId("file:stream"));
}

//
// FileIdToFileName
//

TEST("FileIdToFileName encodes vulnerability-like identifiers safely")
{
    for (const auto* input :
         {".", "..", "...", ".hidden", "file.", "a..b", "../x", "..\\x", "CON", "NUL.txt", "/etc/passwd"})
    {
        CAPTURE(input);
        const auto name = FileIdToFileName(input);
        const std::string Prefix = "file-";
        const auto PrefixLength = Prefix.size();

        // Starts with the fixed prefix
        REQUIRE(name.substr(0, PrefixLength) == Prefix);

        // Remainder is only lowercase hex digits
        CHECK(std::all_of(name.begin() + PrefixLength, name.end(), [](char ch) {
            return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
        }));

        CHECK(name.size() == PrefixLength + 2 * std::string(input).size());

        const std::filesystem::path asPath(name);
        CHECK_FALSE(asPath.has_root_path());
        CHECK_FALSE(asPath.has_parent_path());
        CHECK(asPath.filename() == asPath);
    }
}

TEST("FileIdToFileName produces known encodings")
{
    CHECK(FileIdToFileName(".") == "file-2e");
    CHECK(FileIdToFileName("..") == "file-2e2e");
    CHECK(FileIdToFileName("a..b") == "file-612e2e62");
    CHECK(FileIdToFileName("abc") == "file-616263");
    CHECK(FileIdToFileName("") == "file-");
}

TEST("FileIdToFileName preserves case distinctions")
{
    CHECK(FileIdToFileName("ABC") != FileIdToFileName("abc"));
    CHECK(FileIdToFileName("File") != FileIdToFileName("file"));
}

TEST("FileIdToFileName produces unique names for all single-byte values")
{
    std::set<std::string> names;
    for (unsigned int byte = 0; byte < 256; ++byte)
    {
        const std::string input(1, static_cast<char>(byte));
        const auto name = FileIdToFileName(input);
        CHECK(name.size() == 7);
        CHECK(names.insert(name).second);
    }
}
