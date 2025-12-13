# Qt QKeySequence 标准快捷键说明

## 📋 QKeySequence::Undo 和 QKeySequence::Redo 的默认按键

### QKeySequence::Undo

**平台差异：**
- **Windows**: `Ctrl+Z`
- **Linux**: `Ctrl+Z`
- **macOS**: `Cmd+Z` (⌘Z)

**说明：**
- 在所有平台上，`QKeySequence::Undo` 都对应撤销操作
- Windows/Linux 使用 `Ctrl+Z`
- macOS 使用 `Cmd+Z`（Qt会自动转换）

---

### QKeySequence::Redo

**平台差异：**
- **Windows**: `Ctrl+Y`
- **Linux**: `Ctrl+Shift+Z` 或 `Ctrl+Y`（取决于Qt版本和配置）
- **macOS**: `Cmd+Shift+Z` (⌘⇧Z)

**说明：**
- **Windows**: 通常使用 `Ctrl+Y` 作为重做快捷键
- **Linux**: 可能使用 `Ctrl+Shift+Z`（更符合Unix传统）或 `Ctrl+Y`
- **macOS**: 使用 `Cmd+Shift+Z`（macOS标准）

---

## 🔍 为什么显式设置更好？

### 问题

使用 `QKeySequence::Undo` 和 `QKeySequence::Redo` 时：
- 在不同平台上可能对应不同的按键组合
- Linux 上重做可能是 `Ctrl+Shift+Z` 而不是 `Ctrl+Y`
- 用户可能不熟悉平台特定的快捷键

### 解决方案

显式设置快捷键：
```cpp
// 显式设置，确保在所有平台上都一致
ui->undoButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));  // Ctrl+Z
ui->redoButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));  // Ctrl+Y
```

**优点：**
- ✅ 在所有平台上保持一致（Windows/Linux/macOS）
- ✅ 用户更容易理解和记忆
- ✅ 符合大多数Windows/Linux软件的习惯

---

## 📊 对比表

| 方式 | Windows | Linux | macOS | 一致性 |
|------|---------|-------|-------|--------|
| `QKeySequence::Undo` | Ctrl+Z | Ctrl+Z | Cmd+Z | ⚠️ 不一致 |
| `QKeySequence::Redo` | Ctrl+Y | Ctrl+Shift+Z | Cmd+Shift+Z | ❌ 不一致 |
| 显式设置 `Ctrl+Z` | Ctrl+Z | Ctrl+Z | Ctrl+Z | ✅ 一致 |
| 显式设置 `Ctrl+Y` | Ctrl+Y | Ctrl+Y | Ctrl+Y | ✅ 一致 |

**注意：** 在macOS上，显式设置 `Ctrl+Z` 和 `Ctrl+Y` 仍然会使用 `Ctrl` 键，而不是 `Cmd` 键。如果需要macOS原生体验，可以使用平台检测：

```cpp
#ifdef Q_OS_MAC
    ui->undoButton->setShortcut(QKeySequence(Qt::META | Qt::Key_Z));  // Cmd+Z on macOS
    ui->redoButton->setShortcut(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_Z));  // Cmd+Shift+Z on macOS
#else
    ui->undoButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));  // Ctrl+Z on Windows/Linux
    ui->redoButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));  // Ctrl+Y on Windows/Linux
#endif
```

---

## 💡 当前实现

**当前代码：**
```cpp
ui->undoButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));  // Ctrl+Z
ui->redoButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));  // Ctrl+Y
```

**效果：**
- ✅ 在所有平台上都使用 `Ctrl+Z` 和 `Ctrl+Y`
- ✅ 符合Windows/Linux用户习惯
- ⚠️ macOS用户需要使用 `Ctrl` 而不是 `Cmd`（如果需要macOS原生体验，可以添加平台检测）

---

## 🎯 建议

对于跨平台应用，有两种选择：

### 方案一：统一使用 Ctrl（当前方案）
- 优点：所有平台一致，简单明了
- 缺点：macOS用户可能不习惯（macOS通常用Cmd）

### 方案二：平台特定快捷键（推荐用于macOS）
- 优点：符合各平台原生习惯
- 缺点：需要平台检测代码

**当前实现采用方案一**，适合主要面向Windows/Linux用户的应用。

---

**最后更新**: 2025-01-27

