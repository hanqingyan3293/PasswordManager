cmake_minimum_required(VERSION 3.24)

project(PasswordManager VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Test Network)

qt_standard_project_setup()

qt_add_executable(PasswordManager
    src/main.cpp
    src/app/AppConfig.cpp
    src/app/MainWindow.cpp
    src/app/AppPaths.cpp
    src/app/AppLogger.cpp
    src/app/ArchiveFingerprintService.cpp
    src/app/ArchiveScanner.cpp
    src/app/DiagnosticService.cpp
    src/app/ExtractService.cpp
    src/app/PerformanceBenchmarkService.cpp
    src/app/PasswordMatcher.cpp
    src/app/PasswordTestTaskManager.cpp
    src/app/SevenZipProbe.cpp
    src/app/SevenZipRunner.cpp
    src/app/ShellActionService.cpp
    src/app/ShellIntegration.cpp
    src/data/ArchivePasswordRepository.cpp
    src/data/ArchiveRepository.cpp
    src/data/DatabaseBackupService.cpp
    src/data/DatabaseService.cpp
    src/data/ExtractLogRepository.cpp
    src/data/PasswordLibraryTransferService.cpp
    src/data/PasswordRepository.cpp
    src/data/PasswordTestTaskRepository.cpp
    src/domain/ArchiveRecord.cpp
    src/domain/ArchivePasswordRecord.cpp
    src/domain/PasswordRecord.cpp
    src/ui/HistoryPage.cpp
    src/ui/HomePage.cpp
    src/ui/PasswordDialog.cpp
    src/ui/PasswordsPage.cpp
    src/ui/SettingsPage.cpp
    src/ui/TaskQueuePage.cpp
    include/PasswordManager/app/MainWindow.h
    include/PasswordManager/app/AppConfig.h
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/app/ArchiveFingerprintService.h
    include/PasswordManager/app/AppLogger.h
    include/PasswordManager/app/DiagnosticService.h
    include/PasswordManager/app/PasswordMatcher.h
    include/PasswordManager/app/PerformanceBenchmarkService.h
    include/PasswordManager/app/PasswordTestTaskManager.h
    include/PasswordManager/app/SevenZipProbe.h
    include/PasswordManager/app/SevenZipRunner.h
    include/PasswordManager/app/ShellActionService.h
    include/PasswordManager/app/ShellIntegration.h
    include/PasswordManager/app/ArchiveScanner.h
    include/PasswordManager/app/ExtractService.h
    include/PasswordManager/data/ArchivePasswordRepository.h
    include/PasswordManager/data/ArchiveRepository.h
    include/PasswordManager/data/DatabaseBackupService.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/data/ExtractLogRepository.h
    include/PasswordManager/data/PasswordLibraryTransferService.h
    include/PasswordManager/data/PasswordRepository.h
    include/PasswordManager/data/PasswordTestTaskRepository.h
    include/PasswordManager/domain/ArchiveRecord.h
    include/PasswordManager/domain/ArchivePasswordRecord.h
    include/PasswordManager/domain/PasswordRecord.h
    include/PasswordManager/ui/HistoryPage.h
    include/PasswordManager/ui/HomePage.h
    include/PasswordManager/ui/PasswordDialog.h
    include/PasswordManager/ui/PasswordsPage.h
    include/PasswordManager/ui/SettingsPage.h
    include/PasswordManager/ui/TaskQueuePage.h
)

target_include_directories(PasswordManager
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PasswordManager
    PRIVATE
        Qt6::Widgets
        Qt6::Sql
        Qt6::Network
)

set_target_properties(PasswordManager PROPERTIES
    WIN32_EXECUTABLE TRUE
)

add_custom_command(TARGET PasswordManager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:PasswordManager>/tools/7zip
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:PasswordManager>/tools/7zip/Lang
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/7zip/7z.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/7zip/7zG.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/7zip/7zFM.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/7zip/7z.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/7zip/License.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/7zip/VERSION.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/7zip/SHA256SUMS.txt
        $<TARGET_FILE_DIR:PasswordManager>/tools/7zip
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/7zip/Lang/zh-cn.txt
        $<TARGET_FILE_DIR:PasswordManager>/tools/7zip/Lang
)

enable_testing()

qt_add_executable(PasswordManagerTests
    tests/ArchiveScannerTests.cpp
    src/app/ArchiveScanner.cpp
    src/domain/ArchiveRecord.cpp
    include/PasswordManager/app/ArchiveScanner.h
    include/PasswordManager/domain/ArchiveRecord.h
)

target_include_directories(PasswordManagerTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PasswordManagerTests
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_test(
    NAME PasswordManagerTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:PasswordManagerTests>
)

qt_add_executable(SevenZipRunnerTests
    tests/SevenZipRunnerTests.cpp
    src/app/SevenZipRunner.cpp
    include/PasswordManager/app/SevenZipRunner.h
)

target_include_directories(SevenZipRunnerTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(SevenZipRunnerTests
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_test(
    NAME SevenZipRunnerTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:SevenZipRunnerTests>
)

qt_add_executable(PasswordTestTaskManagerTests
    tests/PasswordTestTaskManagerTests.cpp
    src/app/AppLogger.cpp
    src/app/AppPaths.cpp
    src/app/PasswordTestTaskManager.cpp
    src/app/SevenZipRunner.cpp
    src/data/DatabaseService.cpp
    src/data/PasswordTestTaskRepository.cpp
    include/PasswordManager/app/AppLogger.h
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/app/PasswordTestTaskManager.h
    include/PasswordManager/app/SevenZipRunner.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/data/PasswordTestTaskRepository.h
)

qt_add_executable(PasswordMatcherTests
    tests/PasswordMatcherTests.cpp
    src/app/PasswordMatcher.cpp
    include/PasswordManager/app/PasswordMatcher.h
    include/PasswordManager/domain/PasswordRecord.h
)

target_include_directories(PasswordMatcherTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PasswordMatcherTests
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_test(
    NAME PasswordMatcherTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:PasswordMatcherTests>
)

qt_add_executable(ArchivePasswordRepositoryTests
    tests/ArchivePasswordRepositoryTests.cpp
    src/app/AppPaths.cpp
    src/data/ArchivePasswordRepository.cpp
    src/data/ArchiveRepository.cpp
    src/data/DatabaseService.cpp
    src/data/PasswordRepository.cpp
    src/domain/ArchivePasswordRecord.cpp
    src/domain/ArchiveRecord.cpp
    src/domain/PasswordRecord.cpp
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/data/ArchivePasswordRepository.h
    include/PasswordManager/data/ArchiveRepository.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/data/PasswordRepository.h
    include/PasswordManager/domain/ArchivePasswordRecord.h
    include/PasswordManager/domain/ArchiveRecord.h
    include/PasswordManager/domain/PasswordRecord.h
)

qt_add_executable(ExtractServiceTests
    tests/ExtractServiceTests.cpp
    src/app/ExtractService.cpp
    include/PasswordManager/app/ExtractService.h
)

qt_add_executable(PasswordTestTaskRepositoryTests
    tests/PasswordTestTaskRepositoryTests.cpp
    src/app/AppLogger.cpp
    src/app/AppPaths.cpp
    src/app/PasswordTestTaskManager.cpp
    src/app/SevenZipRunner.cpp
    src/data/DatabaseService.cpp
    src/data/PasswordTestTaskRepository.cpp
    include/PasswordManager/app/AppLogger.h
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/app/PasswordTestTaskManager.h
    include/PasswordManager/app/SevenZipRunner.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/data/PasswordTestTaskRepository.h
)

qt_add_executable(DatabaseBackupServiceTests
    tests/DatabaseBackupServiceTests.cpp
    src/app/AppPaths.cpp
    src/data/DatabaseBackupService.cpp
    src/data/DatabaseService.cpp
    src/data/PasswordRepository.cpp
    src/domain/PasswordRecord.cpp
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/data/DatabaseBackupService.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/data/PasswordRepository.h
    include/PasswordManager/domain/PasswordRecord.h
)

qt_add_executable(PasswordLibraryTransferServiceTests
    tests/PasswordLibraryTransferServiceTests.cpp
    src/app/AppPaths.cpp
    src/data/DatabaseService.cpp
    src/data/PasswordLibraryTransferService.cpp
    src/data/PasswordRepository.cpp
    src/domain/PasswordRecord.cpp
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/data/PasswordLibraryTransferService.h
    include/PasswordManager/data/PasswordRepository.h
    include/PasswordManager/domain/PasswordRecord.h
)

qt_add_executable(DiagnosticServiceTests
    tests/DiagnosticServiceTests.cpp
    src/app/AppLogger.cpp
    src/app/AppPaths.cpp
    src/app/DiagnosticService.cpp
    src/app/SevenZipProbe.cpp
    src/data/DatabaseService.cpp
    src/data/PasswordRepository.cpp
    src/domain/PasswordRecord.cpp
    include/PasswordManager/app/AppLogger.h
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/app/DiagnosticService.h
    include/PasswordManager/app/SevenZipProbe.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/data/PasswordRepository.h
    include/PasswordManager/domain/PasswordRecord.h
)

qt_add_executable(PerformanceBenchmarkServiceTests
    tests/PerformanceBenchmarkServiceTests.cpp
    src/app/AppLogger.cpp
    src/app/AppPaths.cpp
    src/app/ArchiveScanner.cpp
    src/app/PasswordMatcher.cpp
    src/app/PerformanceBenchmarkService.cpp
    src/app/PasswordTestTaskManager.cpp
    src/app/SevenZipRunner.cpp
    src/data/ArchivePasswordRepository.cpp
    src/data/ArchiveRepository.cpp
    src/data/DatabaseService.cpp
    src/data/PasswordRepository.cpp
    src/data/PasswordTestTaskRepository.cpp
    src/domain/ArchivePasswordRecord.cpp
    src/domain/ArchiveRecord.cpp
    src/domain/PasswordRecord.cpp
    include/PasswordManager/app/AppLogger.h
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/app/ArchiveScanner.h
    include/PasswordManager/app/PasswordMatcher.h
    include/PasswordManager/app/PerformanceBenchmarkService.h
    include/PasswordManager/app/PasswordTestTaskManager.h
    include/PasswordManager/app/SevenZipRunner.h
    include/PasswordManager/data/ArchivePasswordRepository.h
    include/PasswordManager/data/ArchiveRepository.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/data/PasswordRepository.h
    include/PasswordManager/data/PasswordTestTaskRepository.h
    include/PasswordManager/domain/ArchivePasswordRecord.h
    include/PasswordManager/domain/ArchiveRecord.h
    include/PasswordManager/domain/PasswordRecord.h
)

qt_add_executable(ArchiveFingerprintServiceTests
    tests/ArchiveFingerprintServiceTests.cpp
    src/app/AppPaths.cpp
    src/app/ArchiveFingerprintService.cpp
    src/data/ArchiveRepository.cpp
    src/data/DatabaseService.cpp
    src/domain/ArchiveRecord.cpp
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/app/ArchiveFingerprintService.h
    include/PasswordManager/data/ArchiveRepository.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/domain/ArchiveRecord.h
)

target_include_directories(ExtractServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ExtractServiceTests
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_test(
    NAME ExtractServiceTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:ExtractServiceTests>
)

target_include_directories(PasswordTestTaskRepositoryTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PasswordTestTaskRepositoryTests
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
)

add_test(
    NAME PasswordTestTaskRepositoryTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:PasswordTestTaskRepositoryTests>
)

target_include_directories(DatabaseBackupServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(DatabaseBackupServiceTests
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
)

add_test(
    NAME DatabaseBackupServiceTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:DatabaseBackupServiceTests>
)

target_include_directories(PasswordLibraryTransferServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PasswordLibraryTransferServiceTests
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
)

add_test(
    NAME PasswordLibraryTransferServiceTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:PasswordLibraryTransferServiceTests>
)

target_include_directories(DiagnosticServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(DiagnosticServiceTests
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
)

add_test(
    NAME DiagnosticServiceTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:DiagnosticServiceTests>
)

target_include_directories(PerformanceBenchmarkServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PerformanceBenchmarkServiceTests
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
)

add_test(
    NAME PerformanceBenchmarkServiceTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:PerformanceBenchmarkServiceTests>
)

target_include_directories(ArchiveFingerprintServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ArchiveFingerprintServiceTests
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
)

add_test(
    NAME ArchiveFingerprintServiceTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:ArchiveFingerprintServiceTests>
)

qt_add_executable(ShellIntegrationTests
    tests/ShellIntegrationTests.cpp
    src/app/ShellIntegration.cpp
    include/PasswordManager/app/ShellIntegration.h
)

qt_add_executable(ShellActionServiceTests
    tests/ShellActionServiceTests.cpp
    src/app/AppConfig.cpp
    src/app/AppLogger.cpp
    src/app/AppPaths.cpp
    src/app/ArchiveScanner.cpp
    src/app/PasswordMatcher.cpp
    src/app/PasswordTestTaskManager.cpp
    src/app/SevenZipRunner.cpp
    src/app/ShellActionService.cpp
    src/data/ArchivePasswordRepository.cpp
    src/data/ArchiveRepository.cpp
    src/data/DatabaseService.cpp
    src/data/PasswordRepository.cpp
    src/data/PasswordTestTaskRepository.cpp
    src/domain/ArchivePasswordRecord.cpp
    src/domain/ArchiveRecord.cpp
    src/domain/PasswordRecord.cpp
    include/PasswordManager/app/AppPaths.h
    include/PasswordManager/app/AppConfig.h
    include/PasswordManager/app/AppLogger.h
    include/PasswordManager/app/ArchiveScanner.h
    include/PasswordManager/app/PasswordMatcher.h
    include/PasswordManager/app/PasswordTestTaskManager.h
    include/PasswordManager/app/SevenZipRunner.h
    include/PasswordManager/app/ShellActionService.h
    include/PasswordManager/data/ArchivePasswordRepository.h
    include/PasswordManager/data/ArchiveRepository.h
    include/PasswordManager/data/DatabaseService.h
    include/PasswordManager/data/PasswordRepository.h
    include/PasswordManager/data/PasswordTestTaskRepository.h
    include/PasswordManager/domain/ArchivePasswordRecord.h
    include/PasswordManager/domain/ArchiveRecord.h
    include/PasswordManager/domain/PasswordRecord.h
)

target_include_directories(ShellIntegrationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ShellIntegrationTests
    PRIVATE
        Qt6::Test
        Qt6::Core
)

add_test(
    NAME ShellIntegrationTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:ShellIntegrationTests>
)

target_include_directories(ShellActionServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ShellActionServiceTests
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
)

add_test(
    NAME ShellActionServiceTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:ShellActionServiceTests>
)

target_include_directories(ArchivePasswordRepositoryTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ArchivePasswordRepositoryTests
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
)

add_test(
    NAME ArchivePasswordRepositoryTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:ArchivePasswordRepositoryTests>
)

target_include_directories(PasswordTestTaskManagerTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PasswordTestTaskManagerTests
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Sql
)

add_test(
    NAME PasswordTestTaskManagerTests
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::Core>/../plugins"
        $<TARGET_FILE:PasswordTestTaskManagerTests>
)
