# Logger&LogFile

Logger 在构造时创建 `Impl`，内部持有 `LogStream`（包含 `FixedBuffer`），首先在 `formatTime()` 中写入时间、线程 ID 和日志级别等前缀；随后通过重载的 `operator<<` 将用户日志内容追加到 `FixedBuffer`。Logger 为栈对象，在语句结束时触发析构函数 `~Logger()`，调用 `Impl::finish()` 完成换行等收尾操作，然后取出 `buffer()` 中的数据，通过全局函数指针 `g_output` 输出。默认 `defaultOutput()` 使用 `fwrite` 写入 stdout，也可通过 `Logger::setOutput()` 重定向到 `LogFile::append()`，实现文件落盘。

---
