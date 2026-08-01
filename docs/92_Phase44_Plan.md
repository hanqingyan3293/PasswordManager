# Phase 44 Plan - 7-Zip GUI Chinese Language

## 目标

- 内置 `7zFM.exe` 打开时优先显示中文界面。
- 继续使用项目内置 `tools/7zip/7zFM.exe`，不调用系统其他位置的 7-Zip。

## 根因

发布包已经包含 `tools/7zip/Lang/zh-cn.txt`，但 7-Zip File Manager 不会因为语言文件存在就自动切换中文。它需要读取用户级 7-Zip 语言配置。

## 实现方案

- 新增 `AppPaths::sevenZipChineseLanguageFile()`。
- 新增 `AppPaths::ensureSevenZipChineseLanguage()`。
- 主程序启动时写入：

```text
HKCU\Software\7-Zip
Lang = zh-cn
```

- 普通文件 `compress-archive` 启动 7zFM 前再次确保语言配置。

## 验收

- 打开主程序后，再从右键菜单启动内置 7zFM，应优先显示中文界面。
- 如果 `tools/7zip/Lang/zh-cn.txt` 缺失，程序记录错误或弹窗提示。

