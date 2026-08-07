# Change Log

## 2026-08-08 Phase 59

### 已完成

- 新增 `scripts/prepare_clean_environment_test.ps1`，生成独立干净环境验收套件。
- 新增 `scripts/run_clean_environment_acceptance.ps1`，在 Sandbox 或普通干净机器中执行自动检查。
- 新增 Windows Sandbox `.wsb` 配置：输入只读、结果单独可写、网络关闭、虚拟 GPU 关闭、剪贴板重定向关闭。
- 自动生成无密码 7z、密码 7z 和密码 zip 测试文件。
- 自动检查中文 Unicode 路径、错误密码退出码和主程序 smoke-test。
- 启动 GUI 后检查 5 个 VC++ Runtime DLL 的实际加载路径均位于程序目录。
- 新增 Phase59 人工验收清单，覆盖界面、表格、密码工作流、7-Zip 中文界面和右键菜单安装/卸载。
- 每次验收写入独立的时间戳会话目录，重新生成套件不会删除历史结果。

### 当前验证

- PowerShell 5.1 兼容语法：通过。
- 验收脚本保持纯 ASCII，避免干净系统代码页导致脚本解析失败。
- `.wsb` XML：通过。
- 验收套件生成：通过。
- 当前开发机模拟运行：14/14 通过。
- GUI 加载运行库：5/5 均来自应用目录。
- 当前机器 Windows Sandbox 状态：Disabled，未自动修改。

### 待完成

- 在 Windows Sandbox 或另一台干净 Windows 10/11 x64 机器执行人工验收。
- 当前开发机已有 VC++ Runtime 和开发工具，因此本机 14/14 结果只证明验收工具有效，不替代干净环境结论。

## 2026-08-08 Phase 58

### 已完成

- 修复轻量包从正在使用的便携目录复制文件导致运行数据进入 ZIP 的问题。
- 轻量包默认先生成最新完整包，再从完整包的干净 ZIP 构建。
- 完整包和轻量包压缩前强制检查 `data`、`config`、`logs`、`backup`，存在任何文件时停止打包。
- 新增 `scripts/verify_release_archive.ps1`，ZIP 生成后自动解包复验运行数据、发布清单和文件哈希。
- 修复打包中途失败时本地运行数据暂存副本可能被删除的问题；恢复失败时保留暂存副本。
- 校验 Release 构建产物、完整包和轻量包中的 `PasswordManager.exe` SHA256 一致。
- 新增 `scripts/deploy_vc_runtime.ps1`，扫描发布目录内全部 EXE/DLL 的 MSVC Runtime 依赖。
- 从 Visual Studio 官方 x64 Redistributable 目录部署 5 个应用本地运行库 DLL。
- 完整包继续保留 `vc_redist.x64.exe`；轻量包只保留应用本地运行库 DLL。
- 完整包和轻量包新增 `VC_RUNTIME_MANIFEST.txt`。

### 安全修复

- Phase57 本地轻量 ZIP 被发现包含本机数据库、配置和日志，旧 ZIP 已在本机重新生成并覆盖，不得继续使用旧副本。
- 新正式 ZIP 中运行数据文件检查结果：完整包 0，轻量包 0。
- 未读取或修改本机 SQLite 密码内容。

### 验证

- Debug 自动测试：14/14 通过。
- Release 构建：通过。
- 完整包与轻量包生成：通过。
- 完整包与轻量包 ZIP 完整性：通过。
- 完整包与轻量包 smoke-test：通过。
- VC++ Runtime 依赖闭包：5 个 DLL，全部为 x64。
- 三处 `PasswordManager.exe` SHA256：`921f8cb3385a05da918871e61fc7db7e5c7ab1e0d3ce455a585620e30b568546`。

### 新基线

- 完整包 ZIP：56.69 MiB，SHA256：`73fc915225d3507a9023d9114482a31d25e9eb39fc9651bfbee8601acd7faa04`。
- 轻量包 ZIP：30.93 MiB，SHA256：`9e97a891b7c8c8e4f77f3699c58b68eca1f38b820e8ddb435a772e491e9205bd`。
- 应用本地 VC++ Runtime 未压缩总大小约 1.00 MiB。
- 图形 fallback 文件保持不变。

## 2026-08-08 Phase 57

### 已完成

- 新增 `scripts/package_lite_release.ps1`。
- 新增可选轻量包输出：
  - `out/PasswordManager-lite-portable`
  - `out/PasswordManager-0.1.0-win-x64-lite.zip`
  - `out/PasswordManager-0.1.0-win-x64-lite.zip.sha256`
- 默认完整包不变，轻量包从完整包复制后再移除可选文件。
- 轻量包移除：
  - `vc_redist.x64.exe`。
  - 除 `qt_zh_CN.qm` 外的 Qt 翻译文件。
  - 除 `qsqlite.dll` 外的 SQL 驱动。
- 保留图形 fallback 文件，暂不移除 `opengl32sw.dll`、`dxcompiler.dll`、`d3dcompiler_47.dll`、`dxil.dll`。

### 验证

- 轻量包生成：通过。
- 轻量包 smoke-test：退出码 0。
- 轻量包 ZIP 完整性：通过。
- 轻量包体积基线生成：通过。
- QML/Quick 残留：0。

### 体积对比

- 完整包 zip：56.31 MiB。
- 轻量包 zip：30.58 MiB。
- zip 节省：约 25.73 MiB。
- 完整便携目录：101.32 MiB。
- 轻量便携目录：71.74 MiB。
- 目录节省：约 29.58 MiB。
- 完整包 SHA256：`3df40afd82f74b08d5586873e806172df9965e709da419881e48127443d33a24`。
- 轻量包 SHA256：`ae347a97432aeb61202844a3e179075ab31086a5f7a969f6bbdd241b370b8816`。

### 风险

- 轻量包要求目标系统已安装 Microsoft Visual C++ Runtime。
- 轻量包只保留 SQLite 驱动，不适合未来临时切换到其他 SQL 驱动。
- 轻量包只保留简体中文 Qt 翻译。
- 仍需在干净 Windows 机器或虚拟机做最终验收。

## 2026-08-04 Phase 56

### 已完成

- 新增 `scripts/measure_release.ps1`，用于生成发布包体积基线。
- 报告输出便携目录大小、zip 大小、exe 大小、文件数量、smoke-test 耗时、SHA256。
- 按类别统计 Application、Qt DLL、Graphics Runtime、MSVC Redistributable、7-Zip、Qt Plugins、Qt Translations、Runtime Data。
- 列出发布目录最大的文件，便于后续判断优化优先级。
- 检查 QML/Quick 依赖和目录残留。

### 当前基线

- 便携目录：101.32 MiB。
- zip：56.31 MiB。
- `PasswordManager.exe`：532.00 KiB。
- 文件数：111。
- smoke-test：35 ms。
- SHA256：`3df40afd82f74b08d5586873e806172df9965e709da419881e48127443d33a24`。
- QML/Quick 残留：0。

### 体积来源

- Graphics Runtime：39.30 MiB，38.8%。
- MSVC Redistributable：24.45 MiB，24.1%。
- Qt DLL：23.40 MiB，23.1%。
- Qt Translations：4.96 MiB，4.9%。
- Qt Plugins：4.51 MiB，4.5%。
- 7-Zip：4.02 MiB，4.0%。

## 2026-08-03 Phase 55

### 已完成

- 首页新增纯 Widgets 概览卡片，不引入 QML。
- 概览显示筛选压缩包数、当前页显示数、密码库记录数、待处理任务数。
- 概览随搜索、分页、刷新和任务状态变化同步更新。
- 保持纯 Widgets 发布依赖，不增加 Qt Quick/QML 运行库。

### 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- ZIP 完整性和 SHA256 校验：通过，`64ff3de888ef330d1c65947af1b6d476ff5c2273172b93e47cf3c415373bf25a`。
- 发布目录确认没有 `qml` 目录。
- 便携版 zip 约 57 MiB。

## 2026-08-02 Phase 54

### 已完成

- QML 混合架构试点已验证可行，但因发布包体积和运行时占用目标，决定回退。
- 移除 `QQuickWidget` 首页概览面板、`HomeOverviewViewModel`、QML resource 和 QML 文件。
- CMake 移除 `Qml`、`Quick`、`QuickWidgets` 依赖。
- 发布打包脚本移除 `windeployqt --qmldir`，恢复纯 Widgets 部署。
- 当前 UI 路线继续保持 Qt Widgets，后续如需概览面板优先用 Widgets 实现。

### 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- ZIP 完整性和 SHA256 校验：通过，`9702db22aa77025ebbeab4f1d9193e98f541afa39de309c485665f1362a2e73e`。
- 发布目录确认没有 `Qt6Qml`、`Qt6Quick`、`Qt6QuickWidgets`、`Qt6OpenGL` 相关 DLL。
- 发布目录确认没有 `qml` 和 `qmltooling` 目录。
- 便携版 zip 回到约 57 MiB。

## 2026-08-02 Phase 53

### 已完成

- 新增 `UiStyle::applyApplicationStyle`，统一 Widgets 按钮、输入框、下拉框、数字输入框、复选框、卡片、状态栏和菜单的基础样式。
- 主窗口左侧导航栏改为更克制的浅色选中态和 hover 态。
- 设置页运行目录从多张单行卡片合并为一张信息卡，目录操作按钮放入独立操作卡，减少纵向拥挤。
- 保留现有功能行为，不引入 QML。

### 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- ZIP SHA256 校验：通过，`0ef05ffe3c665d59da58434d5b3c7bf9ad425d93617d212e01ac78e8d517115f`。

## 2026-08-02 Phase 52

### 已完成

- 新增 `UiStyle`，集中管理 Widgets 基础字体和表格样式。
- 主程序固定全局字体为 `Microsoft YaHei UI`。
- 表格统一字体、行高、表头、网格线、交替行背景和选中颜色。
- 数字单元格使用 `Segoe UI` 并右对齐，修复数字显示像符号的字体 fallback 风险。
- 设置页智能匹配数字输入框使用 `Arial`。
- 表格列宽模式统一为 `Interactive`，保留列之间的双向箭头拖动调整。
- 不引入 QML，不改变现有页面结构和功能。

### 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 51

### 已完成

- `AppLogger` 新增 `archive.log`、`extract.log`、`database.log` 分层日志。
- 程序启动、退出、数据库打开、右键动作、扫描、任务状态和解压结果增加日志。
- 日志不记录明文密码。
- `archives` 表新增 `category` 字段，schema_version 升级到 7。
- 首页表格新增压缩包分类列，并支持对选中压缩包设置分类。
- 智能匹配新增同分类密码候选层。
- 设置页新增 `同分类密码候选` 开关。

### 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 50

### 已完成

- 设置页新增 `扫描时计算完整文件指纹`。
- 默认保持精确扫描；关闭后使用快速扫描，只计算 quickHash。
- 首页扫描和右键扫描反馈新增耗时与模式说明。
- 快速重复扫描已有记录时保留原 fullHash，避免被空值覆盖。
- 新增快速扫描和 fullHash 保留测试。

### 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 49

### 已完成

- 设置页新增 `补全文件指纹`。
- 新增旧记录 fullHash 补全服务。
- 只更新 `archives.full_hash` 和 `scanned_at`，不改密码库、不改历史记录、不删除记录。
- 文件不存在、计算失败、写库失败会分别计数。
- 新增 `ArchiveFingerprintServiceTests`。
- 修复空 fullHash 写库可能触发 SQLite NOT NULL 约束的问题。

### 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 48

### 已完成

- 压缩包记录新增完整 SHA256 文件指纹 `full_hash`。
- 扫描压缩包时计算并保存 fullHash。
- 旧数据库启动时自动迁移新增 `archives.full_hash` 字段。
- 同路径文件变化判断优先使用 fullHash，旧记录缺失时回退 quickHash。
- 智能匹配新增同 fullHash 历史候选层，用于不同路径同一压缩包复用成功密码。
- 首页表格显示 `文件指纹`。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 47

### 已完成

- 新增本地配置 `config/settings.ini`。
- 设置页新增智能匹配功能级开关和候选数量限制。
- 设置页新增右键菜单功能项开关。
- 首页和右键 `使用密码库测试` 按设置启用候选来源。
- 右键菜单安装/修复按设置写入菜单项。
- 右键菜单状态检测按当前设置判断，避免禁用项误报不完整。
- 发布包内新增 `LICENSE`。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 46

### 已完成

- 智能匹配候选升级为分层优先级。
- 当前压缩包成功历史优先于同目录历史、密码库和说明文件。
- 同目录其他压缩包成功历史作为资源组候选。
- 密码库排序继续按收藏、成功次数、失败次数、更新时间、ID。
- 说明文件候选排在密码库之后，不直接写入密码库。
- `ArchivePasswordRepository` 新增按 archiveId 精确读取历史记录。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 45

### 已完成

- 智能匹配测试新增本地说明文件密码候选。
- 支持读取压缩包同目录的 `.txt`、`.md`、`.nfo`、`.url`。
- 支持解析 `密码:`、`解压密码:`、`压缩包密码:`、`password:`、`pass:`、`pwd:` 等明确格式。
- 首页批量智能匹配按每个压缩包单独合并密码库候选和说明文件候选。
- 右键 `使用密码库测试` 同步支持说明文件候选。
- 说明文件候选只进入测试队列，不直接写入密码库。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 44

### 已完成

- 主程序启动时自动设置 `HKCU\Software\7-Zip\Lang = zh-cn`。
- 普通文件右键 `compress-archive` 启动内置 7zFM 前会再次确保中文语言设置。
- 新增 `AppPaths::sevenZipChineseLanguageFile()` 和 `AppPaths::ensureSevenZipChineseLanguage()`。
- 继续只使用项目内置 `tools/7zip/7zFM.exe`。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 43

### 已完成

- 将右键菜单状态读取从 `RegGetValueW()` 改为 `RegOpenKeyExW()` + `RegQueryValueExW()`。
- 状态读取继续保持只读，不再用 `QSettings` 读取 Windows 注册表。
- 不存在的 key 或值返回空字符串，避免创建空壳注册表项。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 42

### 已完成

- 修复 Windows 原生注册表读取的长度探测逻辑。
- `RegGetValueW()` 首次读取长度返回 `ERROR_MORE_DATA` 时视为可继续读取，避免把已安装菜单误判为未安装。
- 保留右键状态刷新只读化，避免卸载后重新创建空壳 key。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 41

### 已完成

- 修复设置页状态刷新可能重新创建 `PasswordManager.*Menu` 空壳注册表 key 的问题。
- Windows 下 `ShellIntegration::status()` 改用 `RegGetValueW()` 只读查询。
- Windows 下 `ShellIntegration::isInstalled()` 改用原生只读查询。
- 保留安装、修复、卸载的现有写入和删除流程。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 40

### 已完成

- 新增卸载注册表清单 `ShellIntegration::uninstallRegistryKeys()`，集中维护当前和历史右键菜单路径。
- 卸载完成后使用 Windows 原生 `RegOpenKeyExW()` 逐项反查残留。
- 如果残留仍存在，设置页会显示具体残留注册表路径，不再误提示卸载成功。
- `ShellIntegrationTests` 增加卸载清单覆盖。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 39

### 已完成

- 普通文件右键 `打包压缩包` 改为走 `PasswordManager.exe --shell-action compress-archive "%1"`。
- `compress-archive` 对普通文件打开父目录，避免 txt、py、json 被 `7zFM.exe` 当作压缩包打开。
- 文件夹和压缩包右键打包行为保持不变。
- 注册表删除增加 Windows 原生 `RegDeleteTreeW()` 兜底，降低卸载后残留 `PasswordManager` 入口的风险。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 38

### 已完成

- 普通文件右键新增 `压缩包密码管理器 > 打包压缩包`。
- 文件夹空白处右键新增 `压缩包密码管理器 > 打包压缩包 / 打开主程序`。
- 右键菜单卸载时清理历史残留注册表项，避免留下失效的 `PasswordManager` 入口。
- 内置 7-Zip GUI 补入 `tools/7zip/Lang/zh-cn.txt`，并加入构建、发布和 SHA256 清单。
- `ShellIntegrationTests` 增加普通文件、文件夹空白处和 `%V` 命令校验。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 37

### 已完成

- 修复同路径同名压缩包重建后，旧密码历史仍显示在新压缩包上的问题。
- `ArchiveRepository::upsert()` 在同路径 `quick_hash` 变化时清理旧的 `archive_passwords` 关联。
- 保持 `archives.path` 唯一键不变，首页仍只显示当前路径的当前文件状态。
- 修复右键结果弹窗最近成功时间格式。
- 增加同路径不同 hash 的历史清理测试。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：通过。
- benchmark：通过。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-02 Phase 36

### 已完成

- `打包压缩包` 改为调用内置 `7zFM.exe` 打开 7-Zip 图形界面，不再直接生成 `.7z`。
- 补入 `tools/7zip/7zFM.exe`，并加入构建、发布和 SHA256 清单。
- 修复无密码压缩包误判：不会把密码库里的任意密码写成成功密码。
- 首页和右键 `使用密码库测试` 都会跳过无需密码的压缩包。
- `自动查找密码` / `查看结果` 对无密码压缩包提示无需密码。
- 密码结果弹窗最多显示前 5 条，避免密码库太多导致弹窗过大。
- 右键 `解压` 改为先测试密码再解压，测试通过后才执行真实解压。
- 增加单实例转发，已有主窗口时复用旧窗口。

### 验证

- Debug 构建：通过。
- Debug CTest：13/13 通过。
- Release 打包：通过。
- Release CTest：13/13 通过。
- smoke-test：通过。
- benchmark：通过。
- ZIP 完整性和 SHA256 校验：通过。

## 2026-08-01 Phase 35

### 已完成

- 右键菜单改为方案A：根菜单写入 `ExtendedSubCommandsKey = PasswordManager.ArchiveMenu` / `PasswordManager.DirectoryMenu`，二级命令写入独立菜单类。
- 压缩包右键移除 `添加到测试队列`，保留 `使用密码库测试`。
- 压缩包右键新增 `解压`，采用受控解压方案2：先无密码，再已知密码，再手动输入；非空密码成功后写入密码库和历史记录。
- 压缩包和文件夹右键新增 `打包压缩包`，直接调用内置 `tools/7zip/7zG.exe`。
- `查看结果` 和 `自动查找密码` 改为只弹窗反馈，不打开主界面。
- 主窗口最小尺寸下调到 `560 x 360`。
- 补入 `7zG.exe`，并加入构建复制、发布复制和 SHA256 记录。
- 修复右键相关源码中的乱码字符串和损坏引号。
- 更新 README、用户手册、Phase35 计划和报告。

### 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-08-01 Phase 34

### 已完成

- 将右键菜单注册表结构从 `SubCommands="" + shell` 改为 `ExtendedSubCommandsKey\Shell`。
- 压缩包右键动作写入：
  - `SystemFileAssociations\.zip\shell\PasswordManager\ExtendedSubCommandsKey\Shell`
  - `SystemFileAssociations\.rar\shell\PasswordManager\ExtendedSubCommandsKey\Shell`
  - `SystemFileAssociations\.7z\shell\PasswordManager\ExtendedSubCommandsKey\Shell`
- 文件夹右键动作写入：
  - `Directory\shell\PasswordManager\ExtendedSubCommandsKey\Shell`
- 新安装不再写空 `SubCommands`。
- 右键命令路径改为 Windows 原生反斜杠。
- `ShellIntegrationTests` 增加 Windows 原生命令路径格式覆盖。

### 手动操作

- 更新发布包后，进入 `设置` 点击 `重新安装/修复`，清理旧注册表树并写入新的 `ExtendedSubCommandsKey` 结构。

### 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 33

### 已完成

- 右键菜单从三级结构改为二级结构，所有已实现动作直接放在 `压缩包密码管理器` 下。
- 压缩包右键二级动作：`自动查找密码`、`使用密码库测试`、`添加到测试队列`、`查看结果`、`打开主程序`。
- 文件夹右键二级动作：`扫描文件夹`、`打开主程序`。
- 安装右键菜单时会先清理旧的 `PasswordManager` 菜单树，再写入新结构。
- 右键动作改为主窗口显示后执行，避免页面切换或弹窗被初始化顺序吞掉。
- 右键动作增加弹窗反馈，避免点击后没有任何可见结果。
- 右键动作增加 Windows 前台激活处理：`showNormal()`、`raise()`、`activateWindow()` 和原生 `SetForegroundWindow()`。
- 右键动作弹窗设置为置顶。
- 日志增加 shell-action 请求、执行、完成记录，便于判断 Explorer 是否真正启动了程序。
- 主窗口默认尺寸从 1120 x 720 降到 960 x 640。
- 主窗口最小尺寸设为 680 x 420，左侧导航不再固定 180 px。
- 设置页新增纵向滚动区域，避免小窗口下内容被挤乱。
- 设置页目录操作按钮从单行改为两行网格。
- 设置页右键菜单和备份按钮改为多行布局。
- 设置页长路径和状态文本允许换行，不再撑大页面宽度。

### 手动操作

- 更新发布包后，进入 `设置` 点击 `重新安装/修复`，清理旧三级菜单并写入新二级菜单。

### 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-28

### 已锁定

- 第一阶段采用 Qt Widgets MVP。
- 按未安装开发环境规划。
- 7-Zip 从项目开始就作为内置依赖准备，放在项目目录，不调用系统其他位置的 7-Zip。
- GPU / 多线程加速登记为后期专题，不进入 MVP。

### 文档新增

- `docs/00_Decision_Lock.md`
- `docs/01_Development_Flow.md`
- `docs/02_Environment_Setup_Windows.md`
- `docs/03_Architecture_Draft.md`
- `docs/04_Acceptance_Checklist.md`
- `docs/05_Change_Log.md`

## 2026-07-28 Phase 0

### 已完成

- 初始化 Git 仓库。
- 创建 CMake + Qt Widgets 最小项目骨架。
- 准备内置 7-Zip 26.02 x64。
- 安装 CMake 4.4.0。
- 安装 Ninja 1.13.2。
- 安装 Qt 6.8.3 `msvc2022_64`。
- 完成 CMake 配置、编译和最小启动验证。

### 新增脚本

- `scripts/check_environment.ps1`
- `scripts/configure_msvc.ps1`
- `scripts/build_msvc.ps1`

## 2026-07-28 Phase 1

### 已完成

- 实现基础 Qt Widgets 主窗口框架。
- 实现设置页。
- 实现程序运行路径显示。
- 实现内置 7-Zip 存在性和版本检测。
- 实现基础样式和状态栏。
- 增加构建后复制内置 7-Zip 运行文件到 exe 相对目录的规则。
- 完成构建、启动、日志和缺失 7-Zip 边界验证。

### 新增模块

- `SevenZipProbe`
- `SettingsPage`

## 2026-07-28 Phase 2

### 已锁定

- 密码存储策略为明文 SQLite，不加密。

### 已完成

- 实现 SQLite 数据库服务。
- 实现 `database_info` 和 `passwords` 表。
- 实现密码仓库列表、搜索、新增、编辑、删除。
- 实现密码库页面。
- 完成构建、启动、数据库创建和持久化验证。

### 新增模块

- `DatabaseService`
- `PasswordRepository`
- `PasswordRecord`
- `PasswordDialog`
- `PasswordsPage`

## 2026-07-28 Phase 3

### 已完成

- 实现压缩包扫描器。
- 支持 ZIP/RAR/7Z 后缀识别。
- 支持单文件、多文件、文件夹递归扫描。
- 实现快速 Hash。
- 新增 `archives` 表。
- 数据库 schema 升级到 2。
- 实现首页扫描页面。
- 增加 `PasswordManagerTests`，覆盖扫描器核心逻辑。

### 新增模块

- `ArchiveRecord`
- `ArchiveScanner`
- `ArchiveRepository`
- `HomePage`
- `ArchiveScannerTests`

## 2026-07-28 Phase 4

### 已完成

- 实现内置 7-Zip 密码测试。
- 首页支持对选中压缩包输入密码并测试。
- 新增真实测试压缩包。
- 新增 `SevenZipRunnerTests`。
- 完成正确密码、错误密码、缺失 7-Zip 自动化验证。

### 测试资源

- `testdata/archives/pm_zip_password_pm-zip-123.zip`
- `testdata/archives/pm_7z_password_pm-7z-456.7z`
- `testdata/tmp/`

### 新增模块

- `SevenZipRunner`
- `SevenZipRunnerTests`

## 2026-07-28 Phase 5

### 已完成

- 实现内存任务系统。
- 首页密码测试改为加入后台任务。
- 解压队列页面替换为任务队列页面。
- 支持 WAITING、RUNNING、COMPLETED、FAILED、CANCELLED、TIMEOUT。
- 支持取消任务。
- 新增 `PasswordTestTaskManagerTests`。
- 测试压缩包扩充到 20 个。

### 测试资源

- `testdata/archives/`
- `testdata/tmp/`

### 新增模块

- `PasswordTestTaskManager`
- `TaskQueuePage`
- `PasswordTestTaskManagerTests`

## 2026-07-28 Phase 6

### 已完成

- 实现密码候选匹配器。
- 首页新增智能匹配测试入口。
- 支持从密码库生成候选密码并批量加入测试任务。
- 新增 `PasswordMatcherTests`。

### 新增模块

- `PasswordMatcher`
- `PasswordMatcherTests`

## 2026-07-28 Phase 7

### 已完成

- 实现 `archive_passwords` 历史关联表。
- 数据库 schema 升级到 3。
- 任务完成后自动回写密码成功/失败次数。
- 正确密码任务完成后保存压缩包与密码关联。
- 历史记录页面接入真实数据。
- 新增 `ArchivePasswordRepositoryTests`。

### 新增模块

- `ArchivePasswordRepository`
- `ArchivePasswordRecord`
- `HistoryPage`
- `ArchivePasswordRepositoryTests`

## 2026-07-29 Phase 8

### 已完成

- 实现基础解压服务。
- 历史记录页新增单个压缩包解压入口。
- 解压前要求用户选择输出目录并确认。
- 新增 `extract_logs` 表。
- 数据库 schema 升级到 4。
- 新增真实解压自动化测试。

### 新增模块

- `ExtractService`
- `ExtractLogRepository`
- `ExtractServiceTests`

## 2026-07-29 Phase 9

### 已完成

- 实现 Windows 当前用户级右键菜单集成。
- 设置页新增右键菜单安装、卸载、刷新入口。
- 注册表操作前二次确认。
- 新增 `--shell-action` 命令行入口。
- 新增 `ShellIntegrationTests`。

### 新增模块

- `ShellIntegration`
- `ShellIntegrationTests`

## 2026-07-29 Phase 10

### 已完成
- 新增发布脚本 `scripts/package_release.ps1`。
- `scripts/configure_msvc.ps1` 新增 `-BuildType` 参数。
- 发布包改用独立 `build-release/` 目录和 Release 构建，避免 Debug Qt DLL 弹窗。
- 发布目录固定为 `out/PasswordManager-portable/`。
- 发布目录内包含 Qt 运行库、MSVC redistributable、运行目录和 `tools/7zip/`。
- 新增 `PasswordManager.exe --smoke-test` 发布验收入口。
- 修正 `SevenZipProbe`，避免 7-Zip 探测输出阻塞。
- 补充 `docs/24_Phase10_Plan.md` 和 `docs/25_Phase10_Report.md`。

### 验证
- `ctest --test-dir build --output-on-failure`：7/7 通过。
- `scripts/package_release.ps1`：通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe i`：通过。
- `ctest --test-dir build-release --output-on-failure`：7/7 通过。

## 2026-07-29 Phase 11

### 已完成
- 发布脚本新增版本参数，默认版本为 `0.1.0`。
- 发布脚本生成版本化 ZIP：
  - `out/PasswordManager-0.1.0-win-x64-portable.zip`
- 发布脚本生成 ZIP 校验文件：
  - `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256`
- 发布目录新增：
  - `README_RELEASE.txt`
  - `ACCEPTANCE_CHECKLIST.txt`
  - `RELEASE_MANIFEST.txt`
- `RELEASE_MANIFEST.txt` 记录发布目录内文件的 SHA256。
- 更新 `README.md`、`docs/26_Phase11_Plan.md`、`docs/27_Phase11_Report.md`。

### 验证
- SHA256 文件与 ZIP 实际哈希一致。
- `7z t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `ctest --test-dir build-release --output-on-failure`：7/7 通过。
- `ctest --test-dir build --output-on-failure`：7/7 通过。

## 2026-07-29 Phase 12

### 已完成
- 新增 `ShellActionService`，封装右键菜单动作的业务流程。
- `--shell-action add-test-queue <archive>` 现在会真实执行：
  - 校验右键传入文件。
  - 扫描并登记压缩包。
  - 从本地明文 SQLite 密码库读取密码。
  - 复用智能匹配生成候选密码。
  - 将候选密码加入测试队列。
- `ArchiveRepository` 新增 `findByPath()`，用于保存后回读 archive ID。
- 主窗口右键动作成功后切换到任务队列页。
- 新增 `ShellActionServiceTests`。
- 自动化测试数量从 7 个增加到 8 个。

### 验证
- `ctest --test-dir build --output-on-failure`：8/8 通过。
- `scripts/package_release.ps1`：通过。
- `ctest --test-dir build-release --output-on-failure`：8/8 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- 发布 ZIP 完整性测试：通过。

## 2026-07-29 Phase 13

### 已完成
- 历史记录页新增 `focusArchivePath()`。
- `--shell-action view-results <archive>` 现在会：
  - 打开历史记录页。
  - 将搜索框设置为右键传入压缩包的绝对路径。
  - 重新加载过滤后的历史记录。
  - 有匹配记录时自动选中第一行。
- `ArchivePasswordRepositoryTests` 新增按压缩包路径过滤历史记录的测试。
- 发布包已重新生成。

### 验证
- `ctest --test-dir build --output-on-failure`：8/8 通过。
- `scripts/package_release.ps1`：通过。
- `ctest --test-dir build-release --output-on-failure`：8/8 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- 发布 ZIP 完整性测试：通过。

## 2026-07-29 Phase 14

### 已完成
- 新增 `password_test_tasks` 表。
- 数据库 schema 版本升级到 `5`。
- 新增 `PasswordTestTaskRepository`。
- `PasswordTestTaskManager` 支持可选持久化仓库。
- 主程序已将任务管理器接入 `PasswordTestTaskRepository`。
- 任务入队、开始、完成、失败、超时、取消都会写入 SQLite。
- 程序启动时加载已持久化任务历史。
- 上次遗留的 `WAITING` / `RUNNING` 任务会在启动时标记为 `CANCELLED`，避免误自动继续执行。
- 新增 `PasswordTestTaskRepositoryTests`。
- `PasswordTestTaskManagerTests` 增加持久化覆盖。
- 自动化测试数量从 8 个增加到 9 个。

### 验证
- `ctest --test-dir build --output-on-failure`：9/9 通过。
- `scripts/package_release.ps1`：通过。
- `ctest --test-dir build-release --output-on-failure`：9/9 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 32

### 已完成

- 右键菜单改为单入口 `压缩包密码管理器`，压缩包文件下按 `密码测试` 和 `任务` 分组。
- 压缩包右键新增 `自动查找密码`、`使用密码库测试`、`添加到测试队列`、`查看结果`、`打开主程序`。
- 文件夹右键新增 `扫描文件夹` 和 `打开主程序`。
- `自动查找密码` 只查询既有成功密码历史，并弹窗提示结果。
- `扫描文件夹` 会递归扫描 ZIP/RAR/7Z 并写入压缩包记录。
- 修复密码库为空时右键加入测试队列误报成功的问题。
- `DatabaseService` 改为每个实例使用独立 SQLite 连接名，提升测试隔离性。
- `ShellActionServiceTests` 新增已知密码查找和文件夹扫描覆盖。

### 暂缓

- `智能匹配测试并解压` 暂不暴露到右键菜单，需要先单独定义确认、输出目录、覆盖、失败处理和日志规则。
- Explorer 多选批量命令和设置页功能级开关暂未完成。

### 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 31

### 已完成
- 新增右键菜单注册表按扩展名状态诊断。
- 设置页显示 `.zip`、`.rar`、`.7z` 右键菜单是否正常、未安装、不完整或路径不一致。
- 新增命令路径匹配校验，确认注册表命令是否指向当前程序。
- 设置页新增 `重新安装/修复`，用户确认后会清理并重新安装当前用户级右键菜单。
- 保留原有安装、卸载、刷新入口。
- `ShellIntegrationTests` 新增命令匹配覆盖，不真实写注册表。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。
- `scripts/package_release.ps1`：通过。
- `ctest --test-dir build-release --output-on-failure`：9/9 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- 发布 ZIP 完整性测试：通过。

## 2026-07-29 Phase 15

### 已完成
- 启动恢复策略从“取消所有未完成任务”调整为“保留等待任务、失败化运行中任务”。
- `PasswordTestTaskRepository` 新增 `prepareTasksForStartup()`，只处理上次遗留的 `RUNNING` 任务。
- `PasswordTestTaskManager` 启动加载持久化任务后会自动继续执行 `WAITING` 队列。
- 持久化任务列表按 `id ASC` 读取，恢复执行顺序与入队顺序一致。
- 更新 `PasswordTestTaskRepositoryTests` 和 `PasswordTestTaskManagerTests`，覆盖启动恢复行为。

### 验证
- `ctest --test-dir build --output-on-failure`：9/9 通过。

## 2026-07-29 Phase 16

### 已完成
- 任务队列页新增按任务状态筛选。
- 任务队列页新增按测试结果/失败原因筛选。
- `PasswordTestTaskManager` 新增 `retryTask(int id)`。
- 手动重试会基于原任务新增一条任务记录，保留原历史。
- 等待中和运行中的任务不允许重试。
- 任务队列表格中的状态和测试结果改为中文显示。
- 任务队列表格列宽改为可拖动调整。
- 任务队列页新增复制单元格和复制整行能力，支持右键菜单复制。
- 任务队列表格改为单元格选中，并用更明显的选中颜色区分当前格。
- 密码库表格新增列宽拖动、复制单元格、复制整行、当前行和当前格差异高亮。
- 密码库新增/编辑窗口的成功次数和失败次数改为普通数字输入框，并限制为整数。
- 手动测试数据的密码库备注改为 UTF-8 中文写入，避免显示为问号。
- `PasswordTestTaskManagerTests` 新增重试行为覆盖。

### 验证
- `ctest --test-dir build --output-on-failure`：9/9 通过。
- `scripts/package_release.ps1`：通过。
- `ctest --test-dir build-release --output-on-failure`：9/9 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 17

### 已完成
- `PasswordTestTaskRepository` 新增单条删除已结束任务记录能力。
- `PasswordTestTaskRepository` 新增批量清理已结束任务记录能力。
- `PasswordTestTaskManager` 新增 `removeFinishedTask(int id)` 和 `clearFinishedTasks()`。
- 删除范围限制为 `COMPLETED`、`FAILED`、`CANCELLED`、`TIMEOUT`。
- `WAITING` 和 `RUNNING` 任务记录不会被删除。
- 任务队列页新增“删除记录”和“清理已结束”按钮。
- 任务队列右键菜单新增“删除记录”。
- 删除确认窗口明确说明不会删除密码库和成功历史。
- `PasswordTestTaskRepositoryTests` 新增清理行为覆盖。
- `PasswordTestTaskManagerTests` 新增删除已结束任务覆盖。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：9/9 通过。
- `scripts/package_release.ps1`：通过。
- `ctest --test-dir build-release --output-on-failure`：9/9 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 18

### 已完成
- 新增 `DatabaseBackupService`。
- 数据库备份使用 SQLite `VACUUM INTO` 生成一致性 `.sqlite3` 文件。
- 备份文件写入运行目录下的 `backup/`。
- 恢复前校验所选文件必须是 PasswordManager SQLite 数据库。
- 恢复前自动保存当前数据库为安全备份。
- 恢复会替换整个当前 SQLite 数据库，完成后退出程序，要求用户重新打开。
- 设置页新增“立即备份”和“从备份恢复”入口。
- 新增 `DatabaseBackupServiceTests`。
- 自动化测试数量从 9 个增加到 10 个。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：10/10 通过。
- `scripts/package_release.ps1`：通过。
- `ctest --test-dir build-release --output-on-failure`：10/10 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 19

### 已完成
- 新增 `PasswordLibraryTransferService`。
- 密码库支持 UTF-8 CSV 明文导出。
- 密码库支持 UTF-8 CSV 明文导入。
- CSV 格式为 `password,category,note,favorite,success_count,failure_count`。
- 导出会写入表头。
- 导入会识别表头，并跳过空密码行。
- CSV 支持逗号和引号转义。
- 密码库页新增“导入 CSV”和“导出 CSV”按钮。
- 导入导出确认窗口明确提示 CSV 包含明文密码。
- 新增 `PasswordLibraryTransferServiceTests`。
- 自动化测试数量从 10 个增加到 11 个。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：11/11 通过。
- `scripts/package_release.ps1`：通过。
- `ctest --test-dir build-release --output-on-failure`：11/11 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 20

### 已完成
- 新增 `docs/44_Phase20_Plan.md`。
- 新增 `docs/45_Current_Feature_Overview.md`。
- 新增 `docs/46_User_Manual.md`。
- 新增 `docs/47_Phase20_Report.md`。
- README 增加当前功能总览和用户手册入口。
- 保留发布目录数据，避免影响真实环境手动测试。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- Release 打包、Release CTest、smoke-test、benchmark、ZIP 完整性、SHA256 校验：通过。

## 2026-07-29 Phase 23

### 已完成
- 首页、密码库、历史记录、任务队列表格新增空状态中文提示。
- 首页、密码库、历史记录、任务队列表格支持点击表头排序。
- 排序后选择记录改为通过表格 ID 回查，避免编辑、删除、解压、测试指向错误记录。
- 设置页新增“打开数据目录”“打开备份目录”“打开日志目录”按钮。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- Release 打包、Release CTest、smoke-test、benchmark、ZIP 完整性、SHA256 校验：通过。

## 2026-07-29 Phase 24

### 已完成
- 新增 `DiagnosticService`。
- 设置页新增“导出诊断包”按钮。
- 诊断导出会在 `logs/diagnostic-*` 目录生成 `diagnostic.txt`。
- 诊断导出会复制已有 `.log` 文件。
- 诊断文本包含应用版本、Qt 版本、OS、运行路径、7-Zip 状态、数据库表记录数量。
- 新增 `DiagnosticServiceTests`。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- Release 打包、Release CTest、smoke-test、benchmark、ZIP 完整性、SHA256 校验：通过。

## 2026-07-29 Phase 22

### 已完成
- CSV 导入结果新增重复行和无效行统计。
- CSV 导入会跳过本地密码库中已存在的相同明文密码。
- CSV 导入会跳过同一文件内重复的相同明文密码。
- CSV 导入会将空密码行计为无效并跳过。
- 密码库导入完成提示显示新增、跳过、重复、无效数量。
- 更新 `PasswordLibraryTransferServiceTests`。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- Release 打包、Release CTest、smoke-test、benchmark、ZIP 完整性、SHA256 校验：通过。

## 2026-07-29 Phase 25

### 已完成
- 新增 `PerformanceBenchmarkService`。
- 设置页新增“运行性能基准”按钮。
- 命令行新增 `--benchmark [folder]`。
- 基准报告写入 `logs/benchmark-*.txt`。
- 报告包含核心表读取耗时、密码候选生成耗时、可选目录扫描耗时。
- 报告明确标记为单线程基线，不涉及 GPU 或多线程。
- 新增 `PerformanceBenchmarkServiceTests`。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- Release 打包、Release CTest、smoke-test、benchmark、ZIP 完整性、SHA256 校验：通过。

## 2026-07-29 Phase 26

### 已完成
- 首页密码输入框提示改为“输入密码测试选中压缩包；留空可测试无密码压缩包”。
- 手动测试空密码时弹窗确认，确认后按无密码压缩包测试。
- 智能匹配测试在加入候选任务前弹窗确认，避免误点直接开始自动测试。
- 密码库无候选时提示用户可通过空密码手动测试无密码压缩包。
- 新增无密码测试压缩包 `fixture_21_no_password.zip`。
- `SevenZipRunnerTests` 新增无密码 ZIP 测试。
- `PasswordTestTaskManagerTests` 新增无密码队列任务测试。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 29

### 已完成
- 首页压缩包表格支持多选。
- 首页 `智能匹配测试` 支持对多个选中压缩包批量加入候选密码任务。
- 批量智能匹配确认窗口显示压缩包数量、候选数量和总任务数量。
- 历史记录页新增复制密码、复制整行、查看密码库、删除历史。
- 查看密码库会跳转到密码库页并定位对应密码记录。
- 删除历史只删除成功历史关联，不删除密码库记录。
- 设置页新增打开程序目录、打开用户手册、打开测试数据入口。
- 发布包新增 `USER_MANUAL.md`。
- 使用内置 7-Zip 完成 20 个加密 ZIP/7Z fixture 和 1 个无密码 ZIP fixture 兼容性基线测试。
- `ArchivePasswordRepositoryTests` 新增删除历史但保留密码库记录覆盖。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- 使用内置 `tools/7zip/7z.exe` 测试 fixture 兼容性：21/21 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布包 `USER_MANUAL.md`：存在。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 30

### 已完成
- 首页、密码库、历史记录、任务队列表格新增分页控件。
- 每页显示支持 10、20、50、100、200、500、全部。
- 支持上一页、下一页和页码输入跳转。
- 搜索或筛选条件变化时自动回到第 1 页。
- ID 列改为数字排序，避免 `1,10,11,2` 这类文本排序问题。
- 密码库成功/失败次数和历史成功次数也改为数字排序。
- 数字单元格改为普通文本显示，并通过自定义数字比较项排序，修复 ID/次数显示异常符号的问题。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：关闭旧便携程序后通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。
- 数字显示补丁重新构建和发布验证：通过。

## 2026-07-29 Phase 27

### 已完成
- 首页、密码库、历史记录、任务队列表格在无数据时保留标题行和列宽拖动能力。
- 空状态提示改为显示在表格下方，不再替代表格本体。
- `PasswordRepository` 新增按明文密码精确查找能力。
- 手动输入的非空密码测试成功后会自动加入明文密码库。
- 已存在于密码库的成功密码不会重复新增，会更新成功次数。
- 无密码压缩包测试成功后不新增空密码库记录，只保留成功历史关联。
- 发布脚本生成干净 ZIP 后，会恢复本地便携目录的 `data`、`config`、`logs`、`backup`，避免手动测试记录被打包过程清空。
- `PasswordLibraryTransferServiceTests` 新增明文密码查找覆盖。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：旧便携目录数据库文件句柄释放后通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。

## 2026-07-29 Phase 28

### 已完成
- 首页、密码库、历史记录、任务队列的 `reload()` 改为主窗口可调用。
- 主窗口保存密码库页和任务队列页指针，便于切页时刷新。
- 切换进入首页、密码库、历史记录、任务队列时自动刷新当前页。
- 程序启动后自动刷新一次当前可见页。
- 手动刷新按钮保留。

### 验证
- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- 发布 ZIP 完整性测试：通过。
- 发布 ZIP SHA256 校验：通过。
