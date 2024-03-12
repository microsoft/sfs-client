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
    //
    // Expected extra elements for an app version entity:
    // {
    //   ...
    //   "ApplicabilityDetails": {
    //     "Architectures": [
    //       "<arch>"
    //     ],
    //     "PlatformApplicabilityForPackage": [
    //       "<app>"
    //     ]
    //   },
    //   "FileMoniker": "<moniker>",
    // }

    THROW_INVALID_RESPONSE_IF_NOT(file.is_object(), "File is not a JSON object", handler);

    std::unique_ptr<FileEntity> tmp;
    const bool isAppEntity = file.contains("FileMoniker");
    if (isAppEntity)
    {
        tmp = std::make_unique<AppFileEntity>();
    }
    else
    {
        tmp = std::make_unique<GenericFileEntity>();
    }

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

    if (isAppEntity)
    {
        auto appEntity = dynamic_cast<AppFileEntity*>(tmp.get());

        THROW_INVALID_RESPONSE_IF_NOT(file["FileMoniker"].is_string(), "File.FileMoniker is not a string", handler);
        appEntity->fileMoniker = file["FileMoniker"];

        THROW_INVALID_RESPONSE_IF_NOT(file.contains("ApplicabilityDetails"),
                                      "Missing File.ApplicabilityDetails in response",
                                      handler);

        const auto& details = file["ApplicabilityDetails"];
        THROW_INVALID_RESPONSE_IF_NOT(details.is_object(), "File.ApplicabilityDetails is not an object", handler);

        THROW_INVALID_RESPONSE_IF_NOT(details.contains("Architectures"),
                                      "Missing File.ApplicabilityDetails.Architectures in response",
                                      handler);
        THROW_INVALID_RESPONSE_IF_NOT(details["Architectures"].is_array(),
                                      "File.ApplicabilityDetails.Architectures is not an array",
                                      handler);
        for (const auto& arch : details["Architectures"])
        {
            THROW_INVALID_RESPONSE_IF_NOT(arch.is_string(),
                                          "File.ApplicabilityDetails.Architectures array value is not a string",
                                          handler);
        }
        appEntity->applicabilityDetails.architectures = details["Architectures"];

        THROW_INVALID_RESPONSE_IF_NOT(details.contains("PlatformApplicabilityForPackage"),
                                      "Missing File.ApplicabilityDetails.PlatformApplicabilityForPackage in response",
                                      handler);
        THROW_INVALID_RESPONSE_IF_NOT(details["PlatformApplicabilityForPackage"].is_array(),
                                      "File.ApplicabilityDetails.PlatformApplicabilityForPackage is not an array",
                                      handler);
        for (const auto& app : details["PlatformApplicabilityForPackage"])
        {
            THROW_INVALID_RESPONSE_IF_NOT(
                app.is_string(),
                "File.ApplicabilityDetails.PlatformApplicabilityForPackage array value is not a string",
                handler);
        }
        appEntity->applicabilityDetails.platformApplicabilityForPackage = details["PlatformApplicabilityForPackage"];
    }

    return tmp;
}

ContentType GenericFileEntity::GetContentType() const
{
    return ContentType::Generic;
}

ContentType AppFileEntity::GetContentType() const
{
    return ContentType::App;
}
