## SFSClient

The `SFSClient` class is the main entry point for interacting with the SFS (Software File Service) service. It provides methods to retrieve download information for products and applications.

To start using the SFSClient library, use `SFSClient::Make()` to create an `SFSClient` instance, which allows you to use the SFS APIs.
The first argument to the factory is a `ClientConfig` struct. Configuring this struct allows you to customize the behavior of the client.

### Creating an SFSClient Instance

```cpp
#include <sfsclient/SFSClient.h>

// Configure the client
SFS::ClientConfig config;
config.accountId = "your-account-id";  // Required
config.instanceId = "your-instance-id";  // Optional
config.nameSpace = "your-namespace";     // Optional

// Create the client
std::unique_ptr<SFS::SFSClient> client;
SFS::Result result = SFS::SFSClient::Make(config, client);
if (!result) {
    std::cerr << "Failed to create SFSClient: " << result.GetMsg() << std::endl;
    return;
}
```

### Key Methods

#### `SFSClient::Make()`
Static factory method to create an SFSClient instance.

**Parameters:**
- `config` - ClientConfig struct containing configuration options
- `out` - Reference to unique_ptr that will hold the created SFSClient instance

**Returns:** `Result` indicating success or failure

#### `GetLatestDownloadInfo()`
Retrieves combined metadata and download URLs from the latest version of specified products.

**Parameters:**
- `requestParams` - RequestParams containing the products to retrieve
- `contents` - Vector of Content objects that will be populated with results

**Returns:** `Result` indicating success or failure

**Example:**
```cpp
// Prepare request parameters
SFS::RequestParams params;
params.productRequests = {{"product-name-or-guid", {}}};

// Get download information
std::vector<SFS::Content> contents;
SFS::Result result = client->GetLatestDownloadInfo(params, contents);
if (result) {
    for (const auto& content : contents) {
        std::cout << "Content: " << content.GetContentId().GetName() 
                  << " v" << content.GetContentId().GetVersion() << std::endl;
        for (const auto& file : content.GetFiles()) {
            std::cout << "  File: " << file.GetFileId() 
                      << " (" << file.GetSizeInBytes() << " bytes)" << std::endl;
        }
    }
}
```

#### `GetLatestAppDownloadInfo()`
Retrieves combined metadata and download URLs from the latest version of specified applications, including prerequisites.

**Parameters:**
- `requestParams` - RequestParams containing the apps to retrieve
- `contents` - Vector of AppContent objects that will be populated with results

**Returns:** `Result` indicating success or failure

**Example:**
```cpp
// Prepare request parameters for an app
SFS::RequestParams params;
params.productRequests = {{"app-name-or-guid", {}}};

// Get app download information
std::vector<SFS::AppContent> appContents;
SFS::Result result = client->GetLatestAppDownloadInfo(params, appContents);
if (result) {
    for (const auto& appContent : appContents) {
        std::cout << "App: " << appContent.GetContentId().GetName() 
                  << " UpdateId: " << appContent.GetUpdateId() << std::endl;
        
        // Process prerequisites
        for (const auto& prereq : appContent.GetPrerequisites()) {
            std::cout << "  Prerequisite: " << prereq.GetContentId().GetName() << std::endl;
        }
    }
}
```

#### `GetVersion()`
Returns the version of the SFSClient library.

**Returns:** `const char*` containing the version string

**Example:**
```cpp
std::cout << "SFSClient version: " << SFS::SFSClient::GetVersion() << std::endl;
```

## Configuration Classes

### ClientConfig

The `ClientConfig` struct is used to configure an SFSClient instance during creation. It contains the connection parameters and optional settings.

**Key Members:**
- `accountId` (string, required) - The account ID of the SFS service used to identify the caller
- `instanceId` (optional<string>) - The instance ID of the SFS service (defaults if not provided)
- `nameSpace` (optional<string>) - The namespace of the SFS service (defaults if not provided)
- `logCallbackFn` (optional<LoggingCallbackFn>) - Callback function for receiving log messages

**Example:**
```cpp
SFS::ClientConfig config;
config.accountId = "msedge";
config.instanceId = "PROD";
config.nameSpace = "microsoft.com";
config.logCallbackFn = [](const SFS::LogData& logData) {
    std::cout << "[" << SFS::ToString(logData.severity) << "] " 
              << logData.message << std::endl;
};
```

### RequestParams

The `RequestParams` struct defines the parameters for requests to the SFS service.

**Key Members:**
- `productRequests` (vector<ProductRequest>, required) - List of products to retrieve
- `baseCV` (optional<string>) - Base CorrelationVector for telemetry stitching
- `proxy` (optional<string>) - Proxy settings for connections
- `retryOnError` (bool) - Whether to retry failed requests (default: true)

**Example:**
```cpp
SFS::RequestParams params;
params.productRequests = {
    {"msedge-stable-win-x64", {{"Architecture", "x64"}, {"Channel", "Stable"}}}
};
params.retryOnError = true;
```

### ProductRequest

The `ProductRequest` struct specifies a single product to retrieve from the service.

**Key Members:**
- `product` (string, required) - The name or GUID that uniquely represents the product
- `attributes` (TargetingAttributes) - Key-value pairs to filter the data retrieved

**Example:**
```cpp
SFS::ProductRequest request;
request.product = "msedge-stable-win-x64";
request.attributes = {
    {"Architecture", "x64"},
    {"Channel", "Stable"},
    {"Language", "en-US"}
};
```

To retrieve logging information from the API, set a logging callback in `ClientConfig::logCallbackFn` when constructing an SFSClient instance with `SFSClient::Make()`.

The logging callback function has the signature:

```cpp
void callback(const SFS::LogData&);
```

### LogData Structure

The `LogData` struct contains information about log messages from the SFSClient:
- `severity` - Log severity level (Error, Warning, Info, Debug, etc.)
- `file` - Source file where the log was generated
- `line` - Line number in the source file
- `message` - The log message text
- `time` - Timestamp when the log was generated

### Logging Examples

**Basic logging to console:**
```cpp
void LoggingCallback(const SFS::LogData& logData)
{
    std::cout << "Log: [" << SFS::ToString(logData.severity) << "]" 
              << " " << logData.file << ":" << logData.line 
              << " " << logData.message << std::endl;
}
```

**Advanced logging with timestamp:**
```cpp
void AdvancedLoggingCallback(const SFS::LogData& logData)
{
    auto now = std::chrono::system_clock::to_time_t(logData.time);
    std::cout << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
              << " [" << SFS::ToString(logData.severity) << "] "
              << std::filesystem::path(logData.file).filename().string()
              << ":" << logData.line << " " << logData.message << std::endl;
}
```

**Setting up the logging callback:**
```cpp
SFS::ClientConfig config;
config.accountId = "your-account-id";
config.logCallbackFn = LoggingCallback;  // or AdvancedLoggingCallback

std::unique_ptr<SFS::SFSClient> client;
SFS::Result result = SFS::SFSClient::Make(config, client);
```

### Important Notes:
- The callback itself is processed in the main thread. Do not use a blocking callback. If heavy processing has to be done, consider capturing the data and processing another thread.
- The LogData contents only exist within the callback call. If the processing will be done later, you should copy the data elsewhere.
- The callback should not do any reentrant calls (e.g. call `SFSClient` methods).

## Class instances

It is recommended to only create a single `SFSClient` instance, even if multiple threads will be used.
Each `GetLatestDownloadInfo()` call will create its own connection and should not interfere with other calls.

### Thread safety

All API calls are thread-safe.

If a logging callback is set in a multi-threaded environment, and the same `SFSClient()` is reused across different threads, the same callback will be called by all usages of the class. So, make sure the callback itself is also thread-safe.

## Content Classes

### Content

The `Content` class represents downloadable content with associated files. Each Content object contains metadata about the content and a list of files that can be downloaded.

**Key Methods:**
- `GetContentId()` - Returns the unique ContentId for this content
- `GetFiles()` - Returns a vector of File objects belonging to this content

**Example:**
```cpp
// After getting content from GetLatestDownloadInfo()
for (const auto& content : contents) {
    const SFS::ContentId& contentId = content.GetContentId();
    std::cout << "Content: " << contentId.GetName() 
              << " v" << contentId.GetVersion() 
              << " (namespace: " << contentId.GetNameSpace() << ")" << std::endl;
    
    const auto& files = content.GetFiles();
    std::cout << "  Contains " << files.size() << " files:" << std::endl;
    for (const auto& file : files) {
        std::cout << "    " << file.GetFileId() 
                  << " - " << file.GetSizeInBytes() << " bytes" << std::endl;
    }
}
```

### AppContent

The `AppContent` class represents application-specific downloadable content, which includes prerequisites and additional metadata.

**Key Methods:**
- `GetContentId()` - Returns the unique ContentId for this app content
- `GetUpdateId()` - Returns the unique update identifier
- `GetFiles()` - Returns a vector of AppFile objects belonging to this app
- `GetPrerequisites()` - Returns a vector of AppPrerequisiteContent objects

**Example:**
```cpp
// After getting app content from GetLatestAppDownloadInfo()
for (const auto& appContent : appContents) {
    std::cout << "App: " << appContent.GetContentId().GetName() 
              << " UpdateId: " << appContent.GetUpdateId() << std::endl;
    
    // Process main app files
    const auto& files = appContent.GetFiles();
    std::cout << "  Main files: " << files.size() << std::endl;
    for (const auto& file : files) {
        std::cout << "    " << file.GetFileMoniker() 
                  << " (" << file.GetFileId() << ")" << std::endl;
    }
    
    // Process prerequisites
    const auto& prerequisites = appContent.GetPrerequisites();
    std::cout << "  Prerequisites: " << prerequisites.size() << std::endl;
    for (const auto& prereq : prerequisites) {
        std::cout << "    " << prereq.GetContentId().GetName() 
                  << " v" << prereq.GetContentId().GetVersion() << std::endl;
    }
}
```

### ContentId

The `ContentId` class provides a unique identifier for content, consisting of namespace, name, and version.

**Key Methods:**
- `GetNameSpace()` - Returns the content namespace
- `GetName()` - Returns the content name
- `GetVersion()` - Returns the 4-part integer version string

**Example:**
```cpp
const SFS::ContentId& contentId = content.GetContentId();
std::cout << "Content Identifier:" << std::endl;
std::cout << "  Namespace: " << contentId.GetNameSpace() << std::endl;
std::cout << "  Name: " << contentId.GetName() << std::endl;
std::cout << "  Version: " << contentId.GetVersion() << std::endl;
```

## File Classes

### File

The `File` class represents a downloadable file with metadata including URL, size, and integrity hashes.

**Key Methods:**
- `GetFileId()` - Returns the unique file identifier within a content version
- `GetUrl()` - Returns the download URL for the file
- `GetSizeInBytes()` - Returns the file size in bytes
- `GetHashes()` - Returns a map of hash algorithms to base64-encoded hash values

**Example:**
```cpp
for (const auto& file : content.GetFiles()) {
    std::cout << "File: " << file.GetFileId() << std::endl;
    std::cout << "  URL: " << file.GetUrl() << std::endl;
    std::cout << "  Size: " << file.GetSizeInBytes() << " bytes" << std::endl;
    
    const auto& hashes = file.GetHashes();
    std::cout << "  Hashes:" << std::endl;
    for (const auto& hash : hashes) {
        std::string hashType = (hash.first == SFS::HashType::Sha1) ? "SHA1" : "SHA256";
        std::cout << "    " << hashType << ": " << hash.second << std::endl;
    }
}
```

### AppFile

The `AppFile` class extends File with additional applicability information for application files.

**Key Methods (inherits from File):**
- `GetFileMoniker()` - Returns a human-readable file identifier
- `GetApplicabilityDetails()` - Returns applicability information (architectures, platforms)

**Example:**
```cpp
for (const auto& appFile : appContent.GetFiles()) {
    std::cout << "App File: " << appFile.GetFileMoniker() << std::endl;
    std::cout << "  File ID: " << appFile.GetFileId() << std::endl;
    std::cout << "  Size: " << appFile.GetSizeInBytes() << " bytes" << std::endl;
    
    const auto& applicability = appFile.GetApplicabilityDetails();
    const auto& architectures = applicability.GetArchitectures();
    if (!architectures.empty()) {
        std::cout << "  Architectures: ";
        for (const auto& arch : architectures) {
            std::cout << ToString(arch) << " ";
        }
        std::cout << std::endl;
    }
}
```

## Error Handling

### Result

The `Result` class provides a comprehensive error handling mechanism for all SFS API operations.

**Key Methods:**
- `GetCode()` - Returns the result code
- `GetMsg()` - Returns the error message (if any)
- `IsSuccess()` - Returns true if the operation succeeded
- `IsFailure()` - Returns true if the operation failed
- `operator bool()` - Implicit conversion to bool (true for success)

**Common Result Codes:**
- `Success` (0x00000000) - Operation completed successfully
- `InvalidArg` (0x80000001) - Invalid argument provided
- `ConnectionSetupFailed` (0x80001000) - Failed to establish connection
- `HttpTimeout` (0x80002000) - HTTP request timed out
- `HttpNotFound` (0x80002404) - HTTP 404 error
- `ServiceInvalidResponse` (0x80003000) - Invalid response from service

**Error Handling Examples:**

**Basic error checking:**
```cpp
SFS::Result result = client->GetLatestDownloadInfo(params, contents);
if (!result) {
    std::cerr << "Error: " << result.GetMsg() << std::endl;
    return -1;
}
```

**Detailed error handling:**
```cpp
SFS::Result result = client->GetLatestDownloadInfo(params, contents);
if (result.IsFailure()) {
    std::cerr << "Operation failed with code: " << std::hex << result.GetCode() 
              << std::dec << std::endl;
    std::cerr << "Message: " << result.GetMsg() << std::endl;
    
    if (result == SFS::Result::HttpTimeout) {
        std::cerr << "Request timed out, please try again" << std::endl;
    } else if (result == SFS::Result::HttpNotFound) {
        std::cerr << "Product not found" << std::endl;
    }
    return -1;
}
```

**Complete example with error handling:**
```cpp
std::unique_ptr<SFS::SFSClient> client;
SFS::Result result = SFS::SFSClient::Make(config, client);
if (!result) {
    std::cerr << "Failed to create client: " << result.GetMsg() << std::endl;
    return -1;
}

SFS::RequestParams params;
params.productRequests = {{"your-product-id", {}}};

std::vector<SFS::Content> contents;
result = client->GetLatestDownloadInfo(params, contents);
if (result) {
    std::cout << "Successfully retrieved " << contents.size() << " content items" << std::endl;
} else {
    std::cerr << "Failed to get download info: " << result.GetMsg() << std::endl;
    return -1;
}
```

## Complete Usage Examples

### Basic Content Download Information Retrieval

```cpp
#include <sfsclient/SFSClient.h>
#include <iostream>
#include <memory>

int main() {
    // Configure the client
    SFS::ClientConfig config;
    config.accountId = "msedge";
    config.instanceId = "PROD";
    config.nameSpace = "microsoft.com";
    
    // Optional: Set up logging
    config.logCallbackFn = [](const SFS::LogData& logData) {
        std::cout << "[" << SFS::ToString(logData.severity) << "] " 
                  << logData.message << std::endl;
    };
    
    // Create the client
    std::unique_ptr<SFS::SFSClient> client;
    SFS::Result result = SFS::SFSClient::Make(config, client);
    if (!result) {
        std::cerr << "Failed to create SFSClient: " << result.GetMsg() << std::endl;
        return -1;
    }
    
    // Prepare request
    SFS::RequestParams params;
    params.productRequests = {
        {"msedge-stable-win-x64", {{"Architecture", "x64"}, {"Channel", "Stable"}}}
    };
    
    // Get download information
    std::vector<SFS::Content> contents;
    result = client->GetLatestDownloadInfo(params, contents);
    if (!result) {
        std::cerr << "Failed to get download info: " << result.GetMsg() << std::endl;
        return -1;
    }
    
    // Process results
    for (const auto& content : contents) {
        const auto& contentId = content.GetContentId();
        std::cout << "Content: " << contentId.GetName() 
                  << " v" << contentId.GetVersion() << std::endl;
        
        for (const auto& file : content.GetFiles()) {
            std::cout << "  File: " << file.GetFileId() << std::endl;
            std::cout << "    URL: " << file.GetUrl() << std::endl;
            std::cout << "    Size: " << file.GetSizeInBytes() << " bytes" << std::endl;
        }
    }
    
    return 0;
}
```

### Application Content with Prerequisites

```cpp
#include <sfsclient/SFSClient.h>
#include <iostream>

void ProcessAppContent() {
    // Client setup (same as above)
    SFS::ClientConfig config;
    config.accountId = "your-account-id";
    
    std::unique_ptr<SFS::SFSClient> client;
    if (!SFS::SFSClient::Make(config, client)) {
        return;
    }
    
    // Request app content
    SFS::RequestParams params;
    params.productRequests = {{"your-app-id", {}}};
    
    std::vector<SFS::AppContent> appContents;
    SFS::Result result = client->GetLatestAppDownloadInfo(params, appContents);
    if (!result) {
        std::cerr << "Failed to get app info: " << result.GetMsg() << std::endl;
        return;
    }
    
    // Process app content
    for (const auto& appContent : appContents) {
        std::cout << "App: " << appContent.GetContentId().GetName() 
                  << " (Update: " << appContent.GetUpdateId() << ")" << std::endl;
        
        // Main app files
        std::cout << "Main Files:" << std::endl;
        for (const auto& file : appContent.GetFiles()) {
            std::cout << "  " << file.GetFileMoniker() 
                      << " - " << file.GetSizeInBytes() << " bytes" << std::endl;
            
            // Show applicability
            const auto& applicability = file.GetApplicabilityDetails();
            const auto& architectures = applicability.GetArchitectures();
            if (!architectures.empty()) {
                std::cout << "    Architectures: ";
                for (const auto& arch : architectures) {
                    std::cout << ToString(arch) << " ";
                }
                std::cout << std::endl;
            }
        }
        
        // Prerequisites
        if (!appContent.GetPrerequisites().empty()) {
            std::cout << "Prerequisites:" << std::endl;
            for (const auto& prereq : appContent.GetPrerequisites()) {
                std::cout << "  " << prereq.GetContentId().GetName() 
                          << " v" << prereq.GetContentId().GetVersion() << std::endl;
                for (const auto& file : prereq.GetFiles()) {
                    std::cout << "    " << file.GetFileMoniker() 
                              << " - " << file.GetSizeInBytes() << " bytes" << std::endl;
                }
            }
        }
    }
}
```

### Advanced Configuration with Proxy and Custom Settings

```cpp
void AdvancedClientSetup() {
    SFS::ClientConfig config;
    config.accountId = "your-account-id";
    config.instanceId = "CUSTOM-INSTANCE";
    config.nameSpace = "custom.namespace";
    
    // Advanced logging with filtering
    config.logCallbackFn = [](const SFS::LogData& logData) {
        // Only log warnings and errors
        if (logData.severity >= SFS::LogSeverity::Warning) {
            auto filename = std::filesystem::path(logData.file).filename().string();
            std::cout << "[" << SFS::ToString(logData.severity) << "] "
                      << filename << ":" << logData.line 
                      << " " << logData.message << std::endl;
        }
    };
    
    std::unique_ptr<SFS::SFSClient> client;
    SFS::Result result = SFS::SFSClient::Make(config, client);
    if (!result) {
        std::cerr << "Client creation failed: " << result.GetMsg() << std::endl;
        return;
    }
    
    // Request with proxy and custom correlation vector
    SFS::RequestParams params;
    params.productRequests = {{"product-id", {{"OS", "Windows"}, {"Arch", "x64"}}}};
    params.proxy = "http://proxy.company.com:8080";
    params.baseCV = "custom-correlation-vector-base";
    params.retryOnError = true;
    
    std::vector<SFS::Content> contents;
    result = client->GetLatestDownloadInfo(params, contents);
    
    // Handle specific error cases
    if (result.IsFailure()) {
        switch (result.GetCode()) {
            case SFS::Result::HttpTimeout:
                std::cerr << "Request timed out, check network connectivity" << std::endl;
                break;
            case SFS::Result::HttpNotFound:
                std::cerr << "Product not found, check product ID" << std::endl;
                break;
            case SFS::Result::ConnectionSetupFailed:
                std::cerr << "Connection failed, check proxy settings" << std::endl;
                break;
            default:
                std::cerr << "Unexpected error: " << result.GetMsg() << std::endl;
                break;
        }
        return;
    }
    
    std::cout << "Successfully retrieved " << contents.size() << " content items" << std::endl;
}
```

## Retry Behavior

The SFS Client API follows a set of rules to automatically retry requests upon encountering specific HTTP Status Codes. This behavior is configurable through the `retryOnError` member of `RequestParams`.

### Default Retry Settings

By default, the client will retry up to **3 times** when encountering the following HTTP Status Codes:
- **429**: Too Many Requests
- **500**: Internal Server Error
- **502**: Bad Gateway
- **503**: Service Unavailable (Server Busy)
- **504**: Gateway Timeout

### Retry Intervals

Between each retry attempt, the client will wait for an interval determined by:
1. **Retry-After header**: If the server response includes a `Retry-After` header, the client respects this value
2. **Exponential backoff**: If no `Retry-After` header is present, the client uses exponential backoff with:
   - Factor of 2
   - Starting from 15 seconds
   - Sequence: 15s, 30s, 60s for subsequent retries

### Configuring Retry Behavior

You can control retry behavior through the `RequestParams` struct:

```cpp
SFS::RequestParams params;
params.productRequests = {{"your-product-id", {}}};

// Enable retries (default: true)
params.retryOnError = true;

// Disable retries for immediate failure on errors
params.retryOnError = false;
```

### Example: Handling Retry Scenarios

```cpp
void HandleRetryableRequests() {
    SFS::RequestParams params;
    params.productRequests = {{"busy-service-product", {}}};
    params.retryOnError = true;  // Enable automatic retries
    
    std::vector<SFS::Content> contents;
    SFS::Result result = client->GetLatestDownloadInfo(params, contents);
    
    if (!result) {
        // Check if this was a retryable error that ultimately failed
        switch (result.GetCode()) {
            case SFS::Result::HttpTooManyRequests:
                std::cerr << "Service busy - all retry attempts exhausted" << std::endl;
                break;
            case SFS::Result::HttpTimeout:
                std::cerr << "Request timed out after retries" << std::endl;
                break;
            case SFS::Result::HttpServiceNotAvailable:
                std::cerr << "Service unavailable after retries" << std::endl;
                break;
            default:
                std::cerr << "Request failed: " << result.GetMsg() << std::endl;
                break;
        }
    } else {
        std::cout << "Request succeeded (possibly after retries)" << std::endl;
    }
}
```

**Note**: The retry mechanism is transparent to the caller. When retries are enabled, the API will automatically handle retryable errors and only return failure if all retry attempts are exhausted.
