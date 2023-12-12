# SFS Client

## Introduction

This repository holds the Simple File Solution (SFS) Client, a C++ library that simplifies the interface with the SFS service.
Read below to get started on developing and using the library.

## Getting Started

### Prerequisites

#### Setup script

There are a few dependencies required to work on this project.
To set them up, use the Setup script:

```powershell
.\scripts\Setup.ps1
```

The script can be run multiple times as it does not replace what has been installed, and updates dependencies.
It also sets up useful command-line aliases that can be used while developing.

## Formatting

This project is currently using the clang-format tool to format its source code according to predefined rules.
It is also using cmake-format to format the CMakeLists.txt files.
Both are installed automatically with the Setup script.

### Automatic usage

The project is configured to automatically run formatting upon committing so nobody introduces
unformatted changes to the codebase. This is done through a pre-commit hook.
If you must avoid the hook, you can use `git commit -n` to bypass it.

### Running on command line

Use -h to see the binary help:
```
clang-format -h
cmake-format -h
```

Use -i to edit a file inplace:
```
clang-format -i .\interface.cpp
cmake-format -i .\CMakeLists.txt
```

Wildcards are accepted in clang-format:
```
clang-format -i .\*.h
```

## Building

Setup the CMake build a single time:

```
cmake -S <repo_root> -B <build_dir>
```

For example, on PowerShell:

```powershell
Set-Location (git rev-parse --show-toplevel)
cmake -S . -B build
```

Then to build:

```
cmake --build <build_dir>
```

## VSCode

[Visual Studio Code](https://code.visualstudio.com) is the recommended editor to work with this project.
But you're free to use other editors and command-line tools.

### Configuring VSCode includes with CMake

VSCode has a great integration with CMake through the CMake Tools extension (ms-vscode.cmake-tools).
It allows you to configure and build the project through the UI.

Open the command pane on VSCode and search for "C/C++: Edit Configurations (JSON)" to create a c_cpp_properties.json file under a .vscode folder in repo root.
The folder is ignored in git by default.
Add the following line to the "configurations" element to make VSCode start using the includes defined in the CMakeLists.txt files.

```json
"configurations": [
  {
    "configurationProvider": "ms-vscode.cmake-tools"
  }
]
```

### Formatting C++ Sources with VSCode

Install the extension https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools and Shift+Alt+F inside a file.
You can also access "Format" options by right clicking on an open file or over a line selection.

If you want it, you can also make VSCode format on each Save operation by adding this to your User JSON settings:
`"editor.formatOnSave": true`

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
