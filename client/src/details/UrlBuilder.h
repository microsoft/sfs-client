// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

// Forward declaration
struct Curl_URL;
typedef struct Curl_URL CURLU;

namespace SFS::details
{
class ReportingHandler;

enum class Scheme
{
    Https,
};

class UrlBuilder
{
  public:
    /**
     * @brief Construct a new Url Builder object with an empty URL
     */
    explicit UrlBuilder(const ReportingHandler& handler);

    /**
     * @brief Construct a new Url Builder object with an existing URL
     * @param url The URL to set
     */
    UrlBuilder(const std::string& url, const ReportingHandler& handler);

    ~UrlBuilder();

    UrlBuilder(const UrlBuilder&) = delete;
    UrlBuilder& operator=(const UrlBuilder&) = delete;

    std::string GetUrl() const;

    /**
     * @brief Set the scheme for the URL
     * @param scheme The scheme to set for the URL Ex: Https
     * @return The reference to the current object
     */
    UrlBuilder& SetScheme(Scheme scheme);

    /**
     * @brief Set the host for the URL
     * @param host The host to set for the URL. Ex: www.example.com
     * @throws SFSException if the string is invalid
     * @return The reference to the current object
     */
    UrlBuilder& SetHost(const std::string& host);

    /**
     * @brief Set a path to the URL
     * @param path The path to set for the URL. Ex: index.html
     * @param encode If true, the path will be URL encoded
     * @throws SFSException if the string is invalid
     * @return The reference to the current object
     */
    UrlBuilder& SetPath(const std::string& path, bool encode = false);

    /**
     * @brief Append a path to the URL
     * @param path The path to be appended for the URL. Ex: index.html
     * @param encode If true, the new path element will be URL encoded, including forward slashes
     * @throws SFSException if the string is invalid
     * @return The reference to the current object
     */
    UrlBuilder& AppendPath(const std::string& path, bool encode = false);

    /**
     * @brief Set a query to the URL
     * @param query The query to set to the URL. Ex: key=value
     * @throws SFSException if the string is invalid
     * @return The reference to the current object
     */
    UrlBuilder& SetQuery(const std::string& query);

    /**
     * @brief Append a query to the URL
     * @param query The query to append to the URL. Ex: key=value
     * @throws SFSException if the string is invalid
     * @return The reference to the current object
     */
    UrlBuilder& AppendQuery(const std::string& query);

    /**
     * @brief Set the URL through a string. Other methods can still be called later to modify the URl
     * @param url The string to set as URL. Ex: http://www.example.com/index.html
     * @throws SFSException if the string is invalid
     * @return The reference to the current object
     */
    UrlBuilder& SetUrl(const std::string& url);

  private:
    /**
     * @brief URL-escape a given string
     */
    std::string EscapeString(const std::string& str) const;

    const ReportingHandler& m_handler;

    CURLU* m_handle = nullptr;
};
} // namespace SFS::details
