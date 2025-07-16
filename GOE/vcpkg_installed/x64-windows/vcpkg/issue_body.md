Package: pugixml:x64-windows@1.15

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.44.35211.0
-    vcpkg-tool version: 2025-06-20-ef7c0d541124bbdd334a03467e7edb6c3364d199
    vcpkg-scripts version: b509a07261 2025-07-15 (2 hours ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
Downloading https://github.com/zeux/pugixml/archive/v1.15.tar.gz -> zeux-pugixml-v1.15.tar.gz
Successfully downloaded zeux-pugixml-v1.15.tar.gz
-- Extracting source C:/dev/vcpkg/downloads/zeux-pugixml-v1.15.tar.gz
CMake Error at scripts/cmake/vcpkg_execute_required_process.cmake:127 (message):
    Command failed: "C:/Program Files/CMake/bin/cmake.exe" -E tar xjf C:/dev/vcpkg/downloads/zeux-pugixml-v1.15.tar.gz
    Working Directory: C:/dev/vcpkg/buildtrees/pugixml/src/v1.15-8a9beec4a5.clean.tmp
    Error code: 1
    See logs for more information:
      C:\dev\vcpkg\buildtrees\pugixml\extract-err.log

Call Stack (most recent call first):
  scripts/cmake/vcpkg_extract_source_archive.cmake:120 (vcpkg_execute_required_process)
  scripts/cmake/vcpkg_extract_source_archive_ex.cmake:8 (vcpkg_extract_source_archive)
  scripts/cmake/vcpkg_from_github.cmake:127 (vcpkg_extract_source_archive_ex)
  C:/Users/sunri/AppData/Local/vcpkg/registries/git-trees/a3181de4dee35567b45611a938eb0f44f5ee016d/portfile.cmake:1 (vcpkg_from_github)
  scripts/ports.cmake:206 (include)



```

<details><summary>C:\dev\vcpkg\buildtrees\pugixml\extract-err.log</summary>

```
CMake Error: Problem with archive_write_header(): Invalid empty pathname
CMake Error: Current file: 
CMake Error: Problem extracting tar: C:/dev/vcpkg/downloads/zeux-pugixml-v1.15.tar.gz
```
</details>

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "dependencies": [
    "assimp"
  ]
}

```
</details>
