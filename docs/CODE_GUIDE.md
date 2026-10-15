# 代码导读与复刻手册

这份文档的目标只有一个：**让你能自己把这个项目重新写一遍，并且每一行都知道为什么要那么写。**

## 怎么用这份文档

不要从头读到尾。按下面的路线一站一站来，每站的姿势是：

1. 先读这一站的讲解，看懂"为什么"
2. 把仓库对应文件**全部关掉**，自己从空白文件敲一遍
3. 敲不出来再回来看，但不要复制粘贴
4. 编译、运行、验证（每站最后都写了怎么验证）
5. 通过了再进下一站

**为什么不能复制粘贴**：看懂和写出来是两种能力。面试的时候只有后一种算数。

## 复刻路线

顺序不是随便排的，是**依赖顺序**——每一站只用到前面已经做过的东西，不会出现
"还没学就用到"的情况。这也正好是当初开发时的顺序。

| 站 | 内容 | 文件数 | 行数 | 做完之后你就有了 |
| --- | --- | --- | --- | --- |
| **1** | 地基：构建系统 + 程序入口 | 6 | 184 | 两个空窗口能编译、能运行 |
| **2** | 消息分帧协议 | 2 | 66 | 能把字节流切成一条条消息 |
| **3** | 数据库地基 | 3 | 201 | 能建库、建表 |
| **4** | 员工仓储 | 2 | 209 | 员工的增删改查 |
| **5** | 人脸模板与考勤仓储 | 5 | 228 | 模板存取、考勤限流查询 |
| **6** | 服务端视觉层 | 2 | 242 | 检测人脸、提特征、比对 |
| **7** | 服务端网络层 | 4 | 248 | 能收发消息 |
| **8** | 服务端界面 | 2 | 339 | 一个能用的窗口 |
| **9** | 服务端控制器 | 2 | 594 | 业务逻辑全部串起来 |
| **10** | 客户端全部 | 12 | 691 | 客户端跑起来，端到端闭环 |
| **11** | 自动化测试 | 3 | 约 220 | 12 个用例全绿 |
| **12** | 打包 | — | — | 一个能拷给别人的文件夹 |

前十站约 3000 行。按一天一站的节奏，两周能走完，比你第一次做的时候快，
因为坑都记在 `TROUBLESHOOTING.md` 里了。

---

# 第 1 站：地基

## 1.1 为什么先讲构建系统

很多人学编程是从 `main` 函数开始读代码的，但真要**自己做一个项目**，第一件事不是写代码，
是告诉电脑"我要编译哪些文件、用哪些库"。

这件事由 `CMakeLists.txt` 负责。它不参与程序的运行，但它决定了：

- 要生成几个程序
- 每个程序由哪些源文件组成
- 每个程序依赖哪些第三方库
- 编译完之后要不要自动复制一些文件

**不先弄懂它，你加一个新文件就会报"找不到符号"，而且不知道怎么修。**

## 1.2 根目录的 CMakeLists.txt

这个文件是整个构建的入口。

```cmake
cmake_minimum_required(VERSION 3.24)
```

声明"这个项目需要 CMake 3.24 或更新的版本"。为什么写这个？如果不写，有人用旧版
CMake 来配置，它不会直接报错，而是在半路给出莫名其妙的失败，你完全找不到原因。
写了它就一句话说清楚："版本不够"。

```cmake
project(QtFaceAttendance VERSION 0.1.0 LANGUAGES CXX)
```

三件事：项目名、版本号、用到的语言。`LANGUAGES CXX` 是"只用 C++"的意思。
不写的话 CMake 会连 C 编译器也去检测一遍，浪费时间。

```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

三行连在一起看：

- `STANDARD 20`：用 C++20 标准
- `STANDARD_REQUIRED ON`：**必须**是 20，编译器不支持就报错，不允许降级到 17 凑合
- `EXTENSIONS OFF`：只用标准 C++，不要编译器自己加的扩展（比如 MSVC 特有的语法）。
  这样做的好处是代码换一个编译器还能编

```cmake
find_package(Qt6 6.9 REQUIRED COMPONENTS Core Network Sql Widgets)
```

让 CMake 去找 Qt。拆开看：

- `6.9`：至少要 6.9 版本
- `REQUIRED`：找不到就直接停下报错，别硬着头皮继续
- `COMPONENTS Core Network Sql Widgets`：只要这四个模块

为什么只列四个？因为 Qt 有几十个模块，有些体积很大（比如 WebEngine 几百兆）。
只声明用得到的，链接出来的程序才小。这四个分别是：

| 模块 | 提供什么 | 项目里用在哪 |
| --- | --- | --- |
| Core | 字符串、容器、定时器、文件系统 | 到处都在用 |
| Network | TCP 通信 | 收发消息 |
| Sql | 数据库 | 员工、模板、考勤 |
| Widgets | 窗口、按钮、列表 | 那三个界面 |

```cmake
qt_standard_project_setup()
```

Qt 提供的便利函数。它自动打开了三个开关，最要紧的是 **AUTOMOC**。

为什么需要它？Qt 的类如果写了 `Q_OBJECT` 宏（比如 `ServerWindow`），就多需要一个
"翻译"步骤——生成一份 `moc_xxx.cpp` 参与编译，信号槽才能工作。手工配置这个很烦，
`qt_standard_project_setup()` 帮你全打开。

**所以你不需要在源文件列表里写 `serverwindow.moc` 之类的东西**，构建系统自己会处理。

```cmake
add_subdirectory(common)
add_subdirectory(apps/client)
add_subdirectory(apps/server)

enable_testing()
add_subdirectory(tests)
```

告诉构建系统："还有四个子目录，进去看它们各自的 CMakeLists"。

**为什么要分目录？** 因为三个目标（公共库、客户端、服务端）的源文件列表、
依赖库、编译选项都不同。混在一个文件里会很难读。

`enable_testing()` 打开测试功能，它和最后的 `add_subdirectory(tests)` 配合，
让 `ctest` 命令能找到测试。

## 1.3 common/CMakeLists.txt

```cmake
add_library(face_attendance_common STATIC
    frameprotocol.h
    frameprotocol.cpp
)
```

建一个**静态库**。库和可执行文件的区别：库不能单独运行，它只是"一堆可以被别人
链接过去的代码"。

`STATIC` 的意思是编译成一个 `.lib` 文件，最终被塞进使用它的 exe 里。
另一种是 `SHARED`，编译成一个独立的 `.dll`，运行时才加载。一个只有一个小类
的库用静态链接最简单——不用操心 DLL 复制的问题。

**为什么要单独建一个库？** 因为分帧协议客户端和服务端都要用。写成一份、两边链接，
比复制两份代码强得多。（当初就是先复制了两份，后来才合并的，见 TROUBLESHOOTING 5.7）

```cmake
target_link_libraries(face_attendance_common
    PUBLIC
        Qt6::Core
)
```

`PUBLIC` 这个词很关键。它有三个取值，含义是"我要不要把这个依赖传给用我的人"：

| 关键字 | 意思 |
| --- | --- |
| `PRIVATE` | 我自己用，别人不用管 |
| `PUBLIC` | 我用，**用我的人也需要** |
| `INTERFACE` | 我不用，但用我的人需要 |

这里必须是 `PUBLIC`。因为 `frameprotocol.h` 里包含了 `QByteArray`，
任何包含这个头文件的代码都会被牵连需要 Qt Core。如果写成 `PRIVATE`，
客户端链接这个库时就会报一堆"找不到 QByteArray"的错。

```cmake
target_include_directories(face_attendance_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
```

把当前目录加入头文件搜索路径，也是 `PUBLIC`，这样两个应用才能写
`#include "frameprotocol.h"` 而不需要写完整路径。

`${CMAKE_CURRENT_SOURCE_DIR}` 是一个 CMake 变量，值是"当前这个 CMakeLists 所在的
目录"。用它而不是写死绝对路径，项目换到别人电脑上才能编。

## 1.4 apps/server/CMakeLists.txt

```cmake
find_package(OpenCV 4.12 REQUIRED COMPONENTS core imgproc objdetect dnn)
```

找 OpenCV。四个组件各有用途：

| 组件 | 提供什么 | 为什么需要 |
| --- | --- | --- |
| core | `cv::Mat`、基础类型 | 图像数据的基本容器 |
| imgproc | 图像处理（缩放、颜色转换） | 预览图的处理 |
| objdetect | **`FaceDetectorYN`、`FaceRecognizerSF`** | 人脸检测和识别就在这里 |
| dnn | 神经网络推理 | 上面那两个类内部靠它跑 ONNX 模型 |

**这条是当初查了半天才搞清楚的**：人脸识别需要的 `FaceRecognizerSF` 不在
需要另外编译的 contrib 里，就在主仓库的 objdetect 里（见 TROUBLESHOOTING 4.1）。

```cmake
qt_add_executable(face_attendance_server
    main.cpp
    network/server.h
    ...
)
```

声明"要生成一个 Qt 可执行程序"，后面括号里列出它包含的**所有**源文件（`.cpp`）
和头文件（`.h`）。

**记住一条铁律：加一个新文件，必须加进这个列表。** 忘了加，报错是
"找不到符号"或者"无法解析的外部符号"——看起来像代码问题，其实是构建配置问题。

```cmake
target_link_libraries(face_attendance_server
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        Qt6::Widgets
        face_attendance_common
        ${OpenCV_LIBS}
)
```

这里用 `PRIVATE` 是因为这是一个可执行程序，**不会再有人链接它**，所以不需要往上传。

`${OpenCV_LIBS}` 是 `find_package` 找到 OpenCV 之后自动填好的变量，
里面是 OpenCV 的库文件名。你不需要知道具体是什么，交给 CMake。

```cmake
target_include_directories(face_attendance_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OpenCV_INCLUDE_DIRS})
```

把当前目录加进搜索路径，这样代码里能写 `#include "database/databasemanager.h"`。

**注意这里的路径写法**：`database/xxx.h` 里的 `database` 是相对
`apps/server` 这个目录的。所以 `apps/server` 必须在这条搜索路径里。
这就是当初写测试时头文件路径写错的原因（见 TROUBLESHOOTING 7.x）。

```cmake
add_custom_command(TARGET face_attendance_server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:opencv_world>
            $<TARGET_FILE_DIR:face_attendance_server>
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:face_attendance_server>/models
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/assets/face_detection_yunet_2023mar.onnx
            $<TARGET_FILE_DIR:face_attendance_server>/models/face_detection_yunet_2023mar.onnx
        ...
    )
```

这是整份文件里最值得琢磨的一段。

`POST_BUILD` 的意思是"**编译完成之后**自动执行"。它做三件事：复制 OpenCV 的 DLL、
建一个 models 目录、把两个模型文件复制进去。

几个符号的含义：

| 写法 | 意思 |
| --- | --- |
| `$<TARGET_FILE:opencv_world>` | OpenCV 这个库在当前构建配置下的实际文件路径 |
| `$<TARGET_FILE_DIR:face_attendance_server>` | 服务端 exe 的输出目录 |
| `${CMAKE_SOURCE_DIR}` | 项目根目录 |
| `copy_if_different` | 内容一样就不重复复制（省时间） |

**为什么要用生成器表达式而不是写死路径？** 因为同一个项目有 Debug 和 Release 两种
构建。Debug 下 OpenCV 的文件叫 `opencv_world4120d.dll`（多了个 d），Release 下叫
`opencv_world4120.dll`。写死一个，另一种就复制错了。

**为什么要复制模型？** 因为程序运行时是按**exe 所在目录**去找模型的
（代码里写的是 `applicationDirPath() + "/models/..."`），不是按源码目录。
从 Qt Creator 里运行时，工作目录不一定是 exe 目录，所以必须把模型复制到 exe 旁边。

```cmake
set_target_properties(face_attendance_server PROPERTIES
    WIN32_EXECUTABLE ON
)
```

编成 Windows 的 GUI 程序。区别很直观：双击运行时不弹那个黑色控制台窗口。

代价是 **printf / cout 的输出看不见了**（没有控制台可打）。所以调试信息要显示在
界面里，这就是这个项目把日志写在状态区的原因之一。

## 1.5 apps/client/CMakeLists.txt 有什么不同

结构和上一条几乎一样，只有两处差别，但每处都有原因：

**第一，多了一个 `videoio` 组件**：

```cmake
find_package(OpenCV 4.12 REQUIRED COMPONENTS core imgproc videoio objdetect dnn)
```

`videoio` 是"视频输入输出"，摄像头就归它管。服务端不碰摄像头，所以不需要。

**第二，复制的是检测模型，没有识别模型**：

```cmake
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/assets/face_detection_yunet_2023mar.onnx
    $<TARGET_FILE_DIR:face_attendance_client>/models/face_detection_yunet_2023mar.onnx
```

因为识别在服务端做，客户端只需要检测（用来在预览上画绿框），不需要那个 37 MB
的识别模型。**这就是"识别放在服务端"这个架构决策在构建文件里的体现。**

## 1.6 服务端 main.cpp 逐行

```cpp
#include <QApplication>
#include <QHostAddress>

#include "database/databasemanager.h"
#include "controller/servercontroller.h"
#include "network/server.h"
#include "ui/serverwindow.h"
```

注意两种 include 的区别，这不是随便写的：

- `#include <...>` 尖括号：先在**系统目录和第三方库**里找。Qt 的头文件属于这一类。
- `#include "..."` 引号：先在**当前项目**里找，找不到再去系统目录。

```cpp
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
```

`main` 是 C++ 程序的入口。`argc` 和 `argv` 是命令行参数（比如你用
`app.exe --help` 启动，这些参数就在这里）。

`QApplication app(...)` 这一行**每个 Qt 图形程序都必须在最前面写一次，而且只能有一次**。
它负责初始化 Qt 内部环境、创建事件循环、处理命令行参数。

为什么必须在最前面？因为它要先于任何界面对象创建。你把 `ServerWindow window;` 写到
它前面，程序会崩或者报警告。

```cpp
    ServerWindow window;
    Server server;
    DatabaseManager database;
```

创建三个对象。注意它们**都是栈上的局部变量，没有用 `new`**。

很多人觉得 Qt 里什么都得 `new`，其实不是。这三个对象活到 `main` 结束就够了，
用栈对象最简单：不用管释放，退出时自动析构。

（Qt 里用 `new` 的主要原因是**父对象机制**——父对象析构时会自动删掉子对象。
这三个没有父子关系，所以不需要。）

```cpp
    ServerController controller(
        server,
        window,
        database,
        &window);
```

创建控制器，把前面三个对象"交给它"。注意前三个是**引用**（不是拷贝一份），
最后一个 `&window` 是**指针**，用作 QObject 的父对象。

为什么要传引用？因为这些对象是"外面的真身"。如果参数是值传递，
函数里拿到的是副本，改副本不影响真身，整个程序就散了。

**为什么 main 这么短，只有四十多行？** 因为所有逻辑都在 `ServerController` 里。
`main` 只负责"把零件组装起来"，不负责业务。这是这个项目里最重要的一条设计，
面试经常问。

```cpp
    if (!controller.initializeDatabase())
    {
        window.appendStatusText("数据库不可用，服务器未启动");
    }
    else if (!controller.initializeVision())
    {
        window.appendStatusText("人脸模型不可用，服务器未启动");
    }
    else
    {
        controller.loadEmployees();
        controller.loadTemplates();
        server.listen(QHostAddress::LocalHost, 45454);
    }
```

这是**启动前置条件**的写法，三段用 `else if` 串起来。

为什么必须串起来？因为这三件事有先后依赖：数据库打不开，就没法读员工和模板；
模型加载不了，就没法识别。缺一个都不该继续往下走。

**这里踩过一个真实的坑**：原来的代码把最后那句 `server.listen()` 写在了
if/else 外面，于是数据库坏掉的时候，界面显示"服务器未启动"，实际端口却已经监听了
（见 TROUBLESHOOTING 6.6）。**写在判断之外的语句，会把判断绕过去。**

```cpp
    window.show();
    return app.exec();
}
```

`show()` 把窗口显示出来。在这之前窗口对象已经存在，只是不可见。

`app.exec()` 是**进入事件循环**。这一行会"卡住"不返回，直到窗口被关闭。
你点的每一个按钮、收到的每一条网络消息、每一次定时器触发，都是在这个循环里
被分发出去的。

**为什么必须有它？** 没有它，`main` 会直接执行到 `}` 就退出，窗口一闪而过。
这在早期验证过（见 PROGRESS 的 Day 1）。

它的返回值会成为进程的退出码。

## 1.7 客户端 main.cpp 的一处关键差异

客户端 `main` 的结构和服务端一样：创建对象 → 加载模型 → 组装控制器 → 连服务器
→ 显示窗口 → 进事件循环。只有模型路径这里值得单独讲：

```cpp
    const QString modelPath =
        QCoreApplication::applicationDirPath()
        + "/models/face_detection_yunet_2023mar.onnx";
```

为什么不用相对路径 `"models/xxx.onnx"`？

因为相对路径是相对于**当前工作目录**的，而工作目录取决于你怎么启动程序：
从 Qt Creator 里启动，工作目录是项目目录；双击 exe 启动，工作目录是 exe 目录。
两者不一样，用相对路径必然有一个找不到。

`applicationDirPath()` 拿的是**exe 自己所在的目录**，这个值不管怎么启动都一样。

还有一处不同：

```cpp
    if (!faceDetector.load(modelPath.toStdString()))
    {
        window.setStatusText("状态：人脸检测模型加载失败");
    }
```

模型加载失败时，客户端只提示，**没有直接退出**。而服务端是"模型不行就不启动"。

为什么区别对待？因为客户端没模型还能开摄像头看预览（只是画不出框），
用户能看到问题出在哪；服务端没模型就什么都干不了，硬启动只会给出错误的结论。

**这就是"同一个失败，在不同模块里的正确处理方式可能完全不同"的例子。**

## 1.8 这一站要能回答的问题

做完这一站，你应该能脱口而出：

1. `CMakeLists.txt` 是干什么的？它参与程序运行吗？
2. `PRIVATE` 和 `PUBLIC` 链接的区别是什么？什么情况下必须用 `PUBLIC`？
3. `POST_BUILD` 那一段为什么要把模型复制到 exe 旁边？不复制会怎样？
4. `$<TARGET_FILE:opencv_world>` 这种写法为什么不直接写死文件名？
5. `QApplication` 为什么必须第一个创建？
6. `app.exec()` 这一行在做什么？去掉会怎样？
7. 为什么模型路径要用 `applicationDirPath()` 拼，而不是写相对路径？
8. 为什么 `main` 这么短？业务逻辑去哪了？

## 1.9 自己复刻的操作

1. 建一个新目录，进里面
2. 建 `CMakeLists.txt`，按 1.2 的内容敲一遍（**别复制**，敲）
3. 建 `common/`、`apps/client/`、`apps/server/` 三个目录，各放一个空的 CMakeLists
4. 根 CMakeLists 里先把 `tests` 那两行注释掉（还没做测试）
5. 两个 `main.cpp` 先只写"创建 QApplication + 空窗口 + exec"，**不要**写
   ServerWindow、DatabaseManager 这些还不存在的类
6. 用 CMake 配置、编译、运行，确认能弹出一个空窗口
7. 这一步成功了，第 1 站就算完成

**关键：第 5 步只写当时存在的东西。** 一次性把最终的 `main.cpp` 抄下来，
必然编译不过，因为那些类还不存在。复刻的正道是**让程序每一步都能编译能运行**，
这个项目当初就是这么长出来的。
