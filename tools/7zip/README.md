# Bundled 7-Zip

PasswordManager only calls the 7-Zip executable bundled in this directory.

Expected runtime files:

```text
tools/7zip/7z.exe
tools/7zip/7z.dll
tools/7zip/License.txt
tools/7zip/VERSION.txt
tools/7zip/SHA256SUMS.txt
```

Do not call `7z.exe` from the system `PATH`.

