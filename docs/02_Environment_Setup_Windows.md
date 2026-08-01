# Windows Environment Setup

版本：v0.1
状态：按未安装环境规划

## 1. 需要安装的软件

### 1.1 Visual Studio 2022 Community

用途：
- 提供 MSVC C++ 编译器。
- 提供 Windows SDK。
- 提供 CMake 集成所需的编译工具链。

安装选项：
- 勾选 “Desktop development with C++”。
- 勾选 Windows 10/11 SDK。
- 勾选 MSVC v143 工具集。

验收命令：

```powershell
cl
```

如果在普通 PowerShell 中找不到 `cl`，可以使用 “Developer PowerShell for VS 2022”。

### 1.2 CMake

用途：
- 管理 C++/Qt 构建。
- 生成 Visual Studio 或 Ninja 构建文件。

当前项目环境：
- CMake 4.4.0 已安装。

验收命令：

```powershell
cmake --version
```

### 1.3 Ninja

用途：
- 快速构建。
- 比 Visual Studio 生成器更适合命令行自动化。

验收命令：

```powershell
ninja --version
```

当前项目环境：
- Ninja 1.13.2 已安装。

### 1.4 Qt 6

用途：
- 桌面 UI。
- SQLite 接入。
- 文件、进程、线程等基础能力。

建议安装：
- Qt 6 LTS。
- MSVC 2022 64-bit 组件。
- Qt Creator 可选。

当前项目环境：
- Qt 6.8.3 已安装到 `C:\Qt\6.8.3\msvc2022_64`。

验收命令：

```powershell
qmake --version
```

或确认 Qt CMake 路径存在，例如：

```powershell
$env:CMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2022_64"
```

### 1.5 Git

当前已安装。

验收命令：

```powershell
git --version
```

## 2. 7-Zip 内置策略

本项目不调用系统 PATH 里的 `7z.exe`，只调用项目目录内的 7-Zip。

开发目录建议：

```text
tools/
  7zip/
    7z.exe
    7z.dll
    License.txt
    VERSION.txt
    SHA256SUMS.txt
```

发布目录建议：

```text
PasswordManager/
  PasswordManager.exe
  tools/
    7zip/
      7z.exe
      7z.dll
      License.txt
```

程序定位规则：

1. 获取 `PasswordManager.exe` 所在目录。
2. 拼接 `tools/7zip/7z.exe`。
3. 检查文件是否存在。
4. 检查进程是否可启动。
5. 只使用这个路径，不回退到系统 PATH。

失败处理：
- 如果 `7z.exe` 不存在，设置页显示依赖缺失。
- 密码测试按钮禁用或提示无法执行。
- 日志写入 `logs/error.log`。

## 3. 推荐首次环境检查命令

```powershell
git --version
cmake --version
ninja --version
cl
qmake --version
```

## 4. 当前已验证命令

```powershell
powershell -ExecutionPolicy Bypass -File scripts/check_environment.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure_msvc.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1
```

## 5. 第一阶段不要求安装的软件

暂不要求：
- GPU SDK
- CUDA
- OpenCL SDK
- 数据库管理工具
- 安装包制作工具
- 自动化 CI/CD 服务

这些都不进入 MVP。
