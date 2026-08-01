#pragma once

#include <QDateTime>
#include <QString>

namespace PasswordManager {

struct PasswordRecord {
    int id = 0;
    QString password;
    QString category;
    QString note;
    bool favorite = false;
    int successCount = 0;
    int failureCount = 0;
    QDateTime createdAt;
    QDateTime updatedAt;
};

} // namespace PasswordManager

