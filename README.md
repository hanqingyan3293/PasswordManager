# PasswordManager

Windows 本地离线压缩包密码管理与智能解压辅助工具。

当前状态：右键菜单方案A、功能级设置、完整文件指纹、压缩包分类候选、分层智能匹配、分层日志、密码库测试、同目录说明文件候选、历史记录、分页表格、Widgets 基础现代化样式、纯 Widgets 首页概览、体积基线、内置 7-Zip、Release 便携包基础流程已实现。GPU / 多线程加速暂不进入当前 MVP，后续单独讨论。

## 当前锁定路线

- Qt Widgets + C++ + CMake + SQLite。
- Windows 本地离线运行，不联网、不上传、不做账号系统。
- 密码按用户确认明文保存到本地 SQLite。
- 7-Zip 固定内置在项目和发布目录的 `tools/7zip/`，不调用系统其他位置的 7-Zip。
- 右键菜单采用 `ExtendedSubCommandsKey` 引用菜单类的方案A。

## 开源协议

本项目使用 `GPL-3.0-only`。如果分发基于本项目的修改版或衍生版，需要按 GPL-3.0 公开对应源码。

## 关键文档

- `PasswordManager_Project_Spec.md`：项目规格书。
- `docs/00_Decision_Lock.md`：已锁定决策。
- `docs/01_Development_Flow.md`：开发流程。
- `docs/02_Environment_Setup_Windows.md`：未安装环境准备。
- `docs/03_Architecture_Draft.md`：架构草案。
- `docs/04_Acceptance_Checklist.md`：验收清单。
- `docs/05_Change_Log.md`：变更记录。
- `docs/46_User_Manual.md`：用户手册。
- `docs/74_Phase35_Plan.md`：当前右键菜单方案计划。
- `docs/75_Phase35_Report.md`：当前右键菜单方案结果。

## 构建

配置 Debug：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/configure_msvc.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
```

构建 Debug：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1
```

运行测试：

```powershell
& "C:\Program Files\CMake\bin\ctest.exe" --test-dir build --output-on-failure
```

不要直接双击 Debug 测试 exe。Debug 测试依赖 Qt Debug DLL，直接双击可能弹出缺少 `Qt6Cored.dll`。

## 发布包

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
```

生成：

```text
out/PasswordManager-portable/
out/PasswordManager-0.1.0-win-x64-portable.zip
out/PasswordManager-0.1.0-win-x64-portable.zip.sha256
```

发布验收：

```powershell
.\out\PasswordManager-portable\PasswordManager.exe --smoke-test
.\out\PasswordManager-portable\PasswordManager.exe --benchmark
.\out\PasswordManager-portable\tools\7zip\7z.exe t .\out\PasswordManager-0.1.0-win-x64-portable.zip
& "C:\Program Files\CMake\bin\ctest.exe" --test-dir build-release --output-on-failure
```

## 右键菜单

压缩包：

```text
压缩包密码管理器 >
├── 自动查找密码
├── 使用密码库测试
├── 查看结果
├── 解压
├── 打包压缩包
└── 打开主程序
```

文件夹：

```text
压缩包密码管理器 >
├── 扫描文件夹
├── 打包压缩包
└── 打开主程序
```

行为：

- `自动查找密码`：只弹窗提示是否有已知成功密码；无密码压缩包提示无需密码。
- `使用密码库测试`：打开或调出主程序并加入测试队列，不解压；候选来源为密码库和压缩包同目录说明文件；无密码压缩包会跳过，不记录任意密码。
- `查看结果`：只弹窗显示成功密码记录，不打开主界面，最多显示前 5 条。
- `解压`：默认解压到同目录同名文件夹；先测试无密码，再测试已知密码，最后要求手动输入。测试通过后才真正解压，非空密码成功后写入密码库和历史关联。
- `打包压缩包`：打开内置 `tools/7zip/7zFM.exe`，由用户在 7-Zip 图形界面里选择压缩参数。
- `扫描文件夹`：打开主程序并扫描文件夹。
- `打开主程序`：打开主窗口。

同一路径同名压缩包被重新创建时，如果 quickHash 变化，旧密码历史会被清理，避免旧文件密码显示到新文件上。

右键菜单安装或修复后，如果根菜单没有 `>`，说明旧注册表结构未清理或 Explorer 未刷新，需要在设置页执行 `重新安装/修复`。

## 命令行入口

```powershell
PasswordManager.exe --shell-action lookup-password "C:\path\archive.zip"
PasswordManager.exe --shell-action use-password-library-test "C:\path\archive.zip"
PasswordManager.exe --shell-action view-results "C:\path\archive.zip"
PasswordManager.exe --shell-action extract-archive "C:\path\archive.zip"
PasswordManager.exe --shell-action scan-folder "C:\path\folder"
PasswordManager.exe --shell-action open-main "C:\path\archive.zip"
```
## Phase 38 右键菜单补充

普通文件右键：

```text
压缩包密码管理器 >
└── 打包压缩包
```

文件夹空白处右键：

```text
压缩包密码管理器 >
├── 打包压缩包
└── 打开主程序
```

`打包压缩包` 调用发布目录内置 `tools/7zip/7zFM.exe`，发布包携带 `tools/7zip/Lang/zh-cn.txt`。卸载右键菜单会清理早期版本残留的 `PasswordManager` 注册表入口。

## Phase 39 普通文件打包补充

普通文件右键的 `打包压缩包` 现在走 `PasswordManager.exe --shell-action compress-archive "%1"`，程序会打开该文件所在目录的内置 7-Zip File Manager，避免 txt、py、json 被当作压缩包打开。卸载右键菜单时同时使用 Windows 原生注册表删除兜底清理残留入口。

## Phase 40 右键卸载验证补充

卸载右键菜单后，程序会用原生注册表查询逐项确认当前和历史 `PasswordManager` 菜单路径是否仍存在。如果仍有残留，设置页会显示具体注册表 key，便于继续定位。

## Phase 41 状态刷新补充

右键菜单状态刷新已改为 Windows 原生只读注册表查询，避免使用 `QSettings` 读取不存在路径时重新创建 `PasswordManager.*Menu` 空壳 key。

## Phase 42 状态识别补充

修复 `RegGetValueW()` 长度探测返回 `ERROR_MORE_DATA` 时的误判，避免右键菜单安装成功但设置页仍显示未安装或不完整。

## Phase 43 状态读取补充

右键菜单状态读取改为 `RegOpenKeyExW()` + `RegQueryValueExW()`，继续保持只读，并降低安装成功后状态误判的风险。

## Phase 44 7-Zip GUI 中文补充

主程序启动时会设置 `HKCU\Software\7-Zip\Lang = zh-cn`，配合发布包内置 `tools/7zip/Lang/zh-cn.txt`，让内置 7-Zip File Manager 优先显示中文界面。

## Phase 45 说明文件候选补充

首页 `智能匹配测试` 和右键 `使用密码库测试` 会读取压缩包同目录说明文件中的明确密码格式。支持 `.txt`、`.md`、`.nfo`、`.url`，示例：`解压密码：123456`、`password: 123456`、`pwd=123456`。说明文件候选排在密码库候选之后，且不会直接写入密码库。

## Phase 46 候选优先级补充

智能匹配候选顺序已固化为：当前压缩包成功历史、同目录其他压缩包成功历史、密码库排序、同目录说明文件候选。密码库内部继续按收藏、成功次数、失败次数、更新时间和 ID 排序。分类候选暂未实现，因为压缩包记录当前没有分类字段。

## Phase 47 功能级设置补充

设置页新增智能匹配开关和右键菜单功能项开关。智能匹配设置保存后立即生效；右键菜单功能项保存后，需要执行 `重新安装/修复` 才会重写资源管理器菜单。

## Phase 48 文件指纹补充

扫描压缩包时新增完整 SHA256 `fullHash`，用于可靠识别不同路径下的同一压缩包。智能匹配候选顺序调整为：当前压缩包成功历史、同 fullHash 其他路径成功历史、同目录历史、密码库、说明文件。

## Phase 49 文件指纹补全补充

设置页新增 `补全文件指纹`，用于为旧数据库中 `full_hash` 为空的压缩包记录计算完整 SHA256。该功能只更新文件指纹，不修改密码库或历史记录，也不会删除文件不存在的记录。

## Phase 50 扫描模式补充

设置页新增 `扫描时计算完整文件指纹`。默认开启时为精确模式，会在扫描时计算完整 SHA256；关闭后为快速模式，只计算 quickHash，新记录的 `文件指纹` 可能为空。快速模式不会清空已有 fullHash，可之后用 `补全文件指纹` 补齐。

## Phase 51 日志与分类候选补充

日志目录会生成 `app.log`、`error.log`、`archive.log`、`extract.log`、`database.log`，诊断包会一并导出。日志不记录明文密码。

首页压缩包表格新增 `分类` 列，可对选中压缩包设置分类。智能匹配候选顺序调整为：当前压缩包历史、同 fullHash 历史、同目录历史、同分类密码、密码库全局排序、同目录说明文件。

## Phase 52 UI 基础修复补充

程序固定全局字体为 `Microsoft YaHei UI`，表格数字单元格使用 `Segoe UI`，设置页智能匹配数字输入框使用 `Arial`，用于修复数字显示像符号的问题。首页、密码库、历史记录、解压队列表格统一了行高、表头、网格线、交替行背景和选中颜色，并保持列宽拖动。

## Phase 53 Widgets 现代化外观补充

程序新增全局 Widgets 样式，统一按钮、输入框、下拉框、数字输入框、复选框、卡片、状态栏和菜单。设置页运行目录合并为一张信息卡，目录操作按钮放入独立操作卡；不引入 QML，不改变业务功能。

## Phase 54 轻量 UI 路线补充

QML 混合架构试点已验证可行，但为降低发布包体积和运行时占用，当前阶段回退 QML，继续采用纯 Qt Widgets。后续如果需要首页概览面板，优先用 Widgets 实现。

## Phase 55 纯 Widgets 首页概览补充

首页新增纯 Widgets 概览卡片，显示筛选压缩包数、当前页显示数、密码库记录数和待处理任务数。发布包继续保持纯 Widgets 依赖。

## Phase 56 体积/占用基线补充

新增 `scripts/measure_release.ps1`，可生成 `out/release-size-baseline.md`。当前基线：便携目录约 101 MiB，zip 约 56 MiB，QML/Quick 残留为 0。最大体积来源是 Graphics Runtime、MSVC Redistributable 和 Qt DLL。
