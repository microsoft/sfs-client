## Logging Callback

To retrieve logging information from the API, set a logging callback in `ClientConfig::logCallbackFn` when constructing an SFSClient instance with `SFSClient::Make()`.

The logging callback function has the signature:

```cpp
void callback(const SFS::LogData&);
```

An example to log the data directly to the standard output using `std::cout`:

```cpp
void LoggingCallback(const SFS::LogData& logData)
{
    std::cout << "Log: [" << ToString(logData.severity) << "]" << " " << logData.file << ":"
              << logData.line << " " << logData.message << std::endl;
}
```

Notes:
- The callback itself is processed in the main thread. Do not use a blocking callback. If heavy processing has to be done, consider capturing the data and processing another thread.
- The LogData contents only exist within the callback call. If the processing will be done later, you should copy the data elsewhere.
- The callback should not do any re-entrant calls (e.g. call `SFSClient` methods).

## Class instances

It is recommended to only create a single `SFSClient` instance, even if multiple threads will be used.
Each `GetLatestDownloadInfo()` call will create its own connection and should not interfere with other calls.

### Thread safety

All API calls are thread-safe.

If a logging callback is set in a multi-threaded environment, and the same `SFSClient()` is reused across different threads, the same callback will be called by all usages of the class. So, make sure the callback itself is also thread-safe.

## Content types

A few data types are provided which abstract contents that can be sent by the SFS Service, such as `Content`, `ContentId`, `File`, `DeliveryOptimizationData`.
These data types provide `noexcept` methods to interact with member data.
They also provide comparison methods that allows one to check if an object is exactly the same as another one. The purpose of these comparisons is to ensure a recently-obtained piece of data is the same as another. As such, string comparisons of members are case-sensitive. If the server changes representation of internal data, the Content will no longer be considered equal.
