# 打包说明

这份文档解决一个问题：**怎么把项目变成一个"拷到别人电脑上双击就能运行"的文件夹。**

## 为什么需要打包

你在自己电脑上写代码，程序能跑，是因为这台电脑装了 Qt（`E:\Dev\Qt`）和
OpenCV（`E:\Dev\OpenCV`）。程序启动的时候，会去这些安装目录里找它需要的 DLL。

别人的电脑上没有这些。你把 `face_attendance_server.exe` 单独拷过去，双击的结果是
"找不到 xxx.dll"，或者弹一个"应用程序无法正常启动"。

所以"打包"要做的事，就是把程序运行时真正需要的东西，**全部收进一个文件夹**。

## 完整步骤

一共四步，在项目根目录下执行。

**第一步，编一个 Release 版。**

```powershell
cd E:\Projects\QtFaceAttendance
cmake --build build\Qt_6_9_1_msvc2022_64 --config Release --parallel 4
```

**第二步，建目录，把 exe、模型、OpenCV 的 DLL 放进去。**

```powershell
$root = "E:\Projects\QtFaceAttendance"
$rel  = "$root\build\Qt_6_9_1_msvc2022_64\Release"
$dist = "$root\dist\FaceAttendance"

New-Item -ItemType Directory -Force -Path $dist | Out-Null

Copy-Item -LiteralPath "$rel\face_attendance_client.exe" -Destination $dist -Force
Copy-Item -LiteralPath "$rel\face_attendance_server.exe" -Destination $dist -Force
Copy-Item -LiteralPath "$rel\opencv_world4120.dll"    -Destination $dist -Force
Copy-Item -LiteralPath "$rel\models" -Destination $dist -Recurse -Force
```

**第三步，让 Qt 把它那部分收齐。** 这一步要跑两次，一次给客户端，一次给服务端：

```powershell
$wd = "E:\Dev\Qt\6.9.1\msvc2022_64\bin\windeployqt.exe"

& $wd --release --no-translations --compiler-runtime "$dist\face_attendance_client.exe"
& $wd --release --no-translations --compiler-runtime "$dist\face_attendance_server.exe"
```

**第四步，验证。** 见下面"怎么验证"。

## 每一步在干什么

### 为什么要单独编一个 Release 版

平时在 Qt Creator 里按 Ctrl+R，编出来的是 **Debug 版**。Debug 版带着调试符号、
不做优化，每个库都大一圈。

最直观的对比是 OpenCV 那个 DLL：

| 版本 | 文件名 | 体积 |
| --- | --- | --- |
| Debug | `opencv_world4120d.dll` | 121 MB |
| Release | `opencv_world4120.dll` | 61 MB |

注意 Debug 版的文件名末尾多一个字母 `d`。这两个是**完全不同的两个库**，
不能互相替换：Debug 编出来的 exe 运行时找的是带 `d` 的那个。

给别人用的必须是 Release，理由有三个：体积小、跑得快，而且不会把调试符号暴露出去。

### 为什么要用 windeployqt

这是整个打包里最关键的一步，值得讲清楚。

Qt 的程序分两部分。一部分是**库**，比如 `Qt6Core.dll`、`Qt6Gui.dll`、
`Qt6Widgets.dll`，这些是必须的。另一部分是**插件**，插件是"按需加载"的小模块：

- `platforms/qwindows.dll` —— 窗口系统。负责创建窗口、接收鼠标键盘。**没有它，
  程序会弹一句 `This application failed to start because no Qt platform plugin
  could be initialized` 然后直接退出**，而且它不会告诉你到底缺哪个文件，极难排查。
- `sqldrivers/qsqlite.dll` —— 数据库驱动。没有它，程序能起来，但一连数据库就报
  "Driver not loaded"。
- `styles/qmodernwindowsstyle.dll` —— 界面外观。

插件不在 exe 旁边，Qt 默认会去它的安装目录找。所以换台电脑，库找得到、插件找不到，
就会出现"能起来但窗口打不开"或者"窗口能开但数据库连不上"这种半死不活的状态。

`windeployqt` 是 Qt 自带的部署工具。它会读一遍 exe 依赖了哪些库，然后把这些库和
对应的插件统统复制过来。你不需要自己数"我要哪几个 DLL"。

常用的几个参数：

| 参数 | 作用 |
| --- | --- |
| `--release` | 按 Release 版本来挑 DLL（不写的话它会自己猜，容易猜错） |
| `--no-translations` | 不要翻译文件（省体积，中文界面不依赖这个） |
| `--compiler-runtime` | 把 VC++ 运行时一起带上，别人的电脑没装也可能跑 |

### 为什么 OpenCV 的 DLL 要手工复制

因为 `windeployqt` **只认识 Qt 自己的东西**，它不知道 OpenCV 是什么。
所以 `opencv_world4120.dll` 必须自己拷进去，少了它，服务端一启动就会报缺 DLL。

这一步已经写在 CMake 里了（POST_BUILD 自动从 OpenCV 目录复制到输出目录），
所以第二步只需要把它从输出目录搬到打包目录。

## 打出来的东西长什么样

```
dist/FaceAttendance/
├── face_attendance_client.exe      客户端，60 KB
├── face_attendance_server.exe      服务端，140 KB
├── opencv_world4120.dll            OpenCV 全模块，61 MB
├── Qt6Core.dll / Qt6Gui.dll / ...  Qt 的库，约 27 MB
├── opengl32sw.dll                  Qt 的软件 OpenGL 兜底，19.7 MB
├── D3Dcompiler_47.dll              Direct3D 着色器编译，4 MB
├── models/                         两个人脸模型，37 MB
│   ├── face_detection_yunet_2023mar.onnx
│   └── face_recognition_sface_2021dec.onnx
├── platforms/                      窗口系统插件（关键）
├── sqldrivers/                     数据库驱动插件（关键）
├── styles/ imageformats/ ...       其他按需加载的插件
└── data/                           空数据库，第一次运行自动生成
```

总共约 **154 MB**，31 个文件。占大头的是 OpenCV（61 MB）、Qt 的图形库（27 MB）、
软件 OpenGL 兜底（19.7 MB）和两个模型（37 MB）。

**想瘦身的话**：`opengl32sw.dll` 是给显卡驱动有问题的机器兜底用的，
加 `--no-opengl-sw` 参数可以不复制它，省 20 MB。代价是万一对方机器显卡驱动有问题，
程序可能起不来。演示用的包建议留着，别为 20 MB 冒险。

## 怎么验证

### 第一层：看它从哪儿加载插件

光看"程序能跑"是不够的——因为你的电脑上装着 Qt，程序有可能偷偷回去用安装目录里的
插件，看起来一切正常，拷到别人电脑上就崩了。

用这个开关能看到真相：

```powershell
$env:QT_DEBUG_PLUGINS = "1"
$env:QT_ASSUME_STDERR_HAS_CONSOLE = "1"
```

然后在打包目录里启动服务端，观察输出里的插件路径。正确的结果是三个插件都来自
打包目录：

```
... loaded library "E:/.../dist/FaceAttendance/platforms/qwindows.dll"
... loaded library "E:/.../dist/FaceAttendance/styles/qmodernwindowsstyle.dll"
... loaded library "E:/.../dist/FaceAttendance/sqldrivers/qsqlite.dll"
```

如果路径指向 `E:/Dev/Qt/...`，说明它用的是本机安装的 Qt，这个包拿到别的电脑上会挂。

### 第二层：看它能不能干活

在打包目录里双击服务端，状态区应该依次出现：

```
服务器正在启动
数据库初始化成功：.../data/attendance.db
人脸模型加载成功
已加载 0 张人脸模板
服务器正在监听 127.0.0.1:45454
```

"已加载 0 张"是正常的——这是一个全新的空库。

### 第三层：真的换一台电脑

前两层只能证明"依赖都在文件夹里"。**唯一能百分百确认的办法，是把整个文件夹拷到
一台没装过 Qt 和 OpenCV 的电脑上，双击运行。** 虚拟机也行。

这一步别省。你自己电脑上的环境变量、注册表、系统 PATH 都可能帮它兜底，
在本机测一百遍都不等于在别人机器上能跑。

## 常见问题

### 双击没反应，或者黑窗口一闪就没了

GUI 程序双击运行时看不到报错。用命令行启动就能看到原因：

```powershell
cd E:\Projects\QtFaceAttendance\dist\FaceAttendance
.\face_attendance_server.exe
```

报错会打在控制台里。

### 报 no Qt platform plugin could be initialized

`platforms` 目录丢了，或者里面没有 `qwindows.dll`。

**最常见的原因是压缩包的问题**：某些解压工具会把空目录或者深层目录漏掉。
所以给对方的是压缩包时，提醒他用 Windows 自带的解压，别用那些会"智能跳过"的。

### 报缺 opencv_world4120.dll

忘了复制第二步里的那个文件。注意别复制成带 `d` 的那个（那是 Debug 版）。

### 报缺 vcruntime140.dll 之类的

对方电脑没装 Visual C++ 运行库。打包时加 `--compiler-runtime` 参数就能一起带上。

### 打包出来是空数据库

正常。`data/attendance.db` 是程序第一次运行时自己建的，里面没有员工数据。

如果想连同演示数据一起打包，把你自己开发时用的
`build/Qt_6_9_1_msvc2022_64/Debug/data/attendance.db` 复制到打包目录的 `data/`
里面覆盖掉就行——**但注意那里面有真实的员工和人脸特征**，别随便发给别人。

### 换台电脑之后识别不出来

人脸识别对光照和摄像头很敏感。同一个人的脸，用不同的摄像头采集，相似度会下降。
如果换电脑后识别率明显变差，重新登记一次人脸模板通常就能解决。
