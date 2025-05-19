# SFSClient Tool

The SFSClient Tool is a command-line utility that demonstrates how to interact with the SFS service through the SFS Client library. It retrieves content metadata and download URLs from the SFS Service, which can be displayed or saved to a file.

## Prerequisites

To build and run the SFSClient Tool, you'll need:
- CMake (3.15 or higher)
- C++ compiler with C++17 support
- nlohmann_json library (installed via vcpkg)
- The SFS Client library

## Building

The SFSClient Tool is built as part of the samples when building the SFS Client library. To build the samples:

Windows:
```powershell
.\scripts\Setup.ps1
build
```

Linux:
```bash
source ./scripts/setup.sh
build
```

Alternatively, you can use CMake directly:

```bash
cmake -B build -S .
cmake --build build
```

The built executable will be located in the `build/samples/tool` directory.

## Usage

The SFSClient Tool requires at minimum a product identifier and account ID to retrieve content information.

Basic usage:
```
SFSClientTool --product <identifier> --accountId <id> [options]
```

### Example

```
SFSClientTool --product msedge-stable-win-x64 --accountId msedge
```

This will retrieve the latest version information for the Microsoft Edge stable channel for Windows x64 and display it in the console.

### Saving Output to a File

To save the JSON output to a file:

```
SFSClientTool --product msedge-stable-win-x64 --accountId msedge -o result.json
```

## Command-line Arguments

### Required Arguments

| Argument | Description |
|----------|-------------|
| `--product <identifier>` | Name or GUID of the product to be retrieved |
| `--accountId <id>` | Account ID of the SFS service, used to identify the caller |

### Optional Arguments

| Argument | Description |
|----------|-------------|
| `-h, --help` | Display the help message |
| `-v, --version` | Display the library version |
| `-o, --outputFile <path>` | Save the JSON output to the specified file |
| `--isApp` | Indicates the specific product is an App |
| `--instanceId <id>` | A custom SFS instance ID |
| `--namespace <ns>` | A custom SFS namespace |
| `--customUrl <url>` | A custom URL for the SFS service (library must have been built with SFS_ENABLE_OVERRIDES) |

## Output Format

The tool outputs JSON-formatted data containing content information, including:
- Content ID (namespace, name, version)
- File details (ID, URL, size, hashes)
- For apps, additional metadata like architecture and prerequisite information

Example output:
```json
[
  {
    "ContentId": {
      "Name": "product-name",
      "Namespace": "namespace",
      "Version": "1.2.3.4"
    },
    "Files": [
      {
        "FileId": "file-id",
        "Hashes": {
          "Sha256": "hash-value"
        },
        "SizeInBytes": 1234567,
        "Url": "https://example.com/download/file.ext"
      }
    ]
  }
]
```