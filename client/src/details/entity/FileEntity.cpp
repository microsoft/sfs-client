// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "FileEntity.h"

#include "../ErrorHandling.h"
#include "../ReportingHandler.h"

#include <nlohmann/json.hpp>

#define THROW_INVALID_RESPONSE_IF_NOT(condition, message, handler)                                                     \
    THROW_CODE_IF_NOT_LOG(ServiceInvalidResponse, condition, handler, message)

using namespace SFS::details;
using json = nlohmann::json;

std::unique_ptr<FileEntity> FileEntity::FromJson(const nlohmann::json& file, const ReportingHandler& handler)
{
    // Expected format for a generic file entity:
    // {
    //   "FileId": <fileid>,
    //   "Url": <url>,
    //   "SizeInBytes": <size>,
    //   "Hashes": {
    //     "Sha1": <sha1>,
    //     "Sha256": <sha2>
    //   },
    //   "DeliveryOptimization": {} // ignored, not used by the client.
    // }

    // TODO #50: Use a different entity once the service supports app content.

    std::unique_ptr<FileEntity> tmp = std::make_unique<GenericFileEntity>();

    THROW_INVALID_RESPONSE_IF_NOT(file.is_object(), "File is not a JSON object", handler);

    THROW_INVALID_RESPONSE_IF_NOT(file.contains("FileId"), "Missing File.FileId in response", handler);
    THROW_INVALID_RESPONSE_IF_NOT(file["FileId"].is_string(), "File.FileId is not a string", handler);
    tmp->fileId = file["FileId"];

    THROW_INVALID_RESPONSE_IF_NOT(file.contains("Url"), "Missing File.Url in response", handler);
    THROW_INVALID_RESPONSE_IF_NOT(file["Url"].is_string(), "File.Url is not a string", handler);
    tmp->url = file["Url"];

    THROW_INVALID_RESPONSE_IF_NOT(file.contains("SizeInBytes"), "Missing File.SizeInBytes in response", handler);
    THROW_INVALID_RESPONSE_IF_NOT(file["SizeInBytes"].is_number_unsigned(),
                                  "File.SizeInBytes is not an unsigned number",
                                  handler);
    tmp->sizeInBytes = file["SizeInBytes"];

    THROW_INVALID_RESPONSE_IF_NOT(file.contains("Hashes"), "Missing File.Hashes in response", handler);
    THROW_INVALID_RESPONSE_IF_NOT(file["Hashes"].is_object(), "File.Hashes is not an object", handler);

    for (const auto& [hashType, hashValue] : file["Hashes"].items())
    {
        THROW_INVALID_RESPONSE_IF_NOT(hashValue.is_string(), "File.Hashes object value is not a string", handler);
        tmp->hashes[hashType] = hashValue;
    }

    return tmp;
}

ContentType GenericFileEntity::GetContentType() const
{
    return ContentType::Generic;
}
