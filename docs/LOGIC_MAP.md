# 项目逻辑地图

这份文档解决一件事：**让你在半天之内看懂整个项目的逻辑。**

它不讲语法，也不逐行解释代码。它讲的是：**谁调用谁、数据在哪儿变成什么、
为什么切成这些模块。** 语法你已经会了，缺的就是这一层。

配着看的还有一份 `CODE_GUIDE.md`，那份是逐行的字典。**现在别去读它**，
等哪天想抠某一行为什么那么写，再去查。

---

## 半天怎么安排

| 时间 | 做什么 |
| --- | --- |
| 第 1 段（40 分钟） | 读本文档第 1 到第 3 节：整体结构和那条核心规则 |
| 第 2 段（50 分钟） | 打开 `apps/server/controller/servercontroller.h` 和 `.cpp`，对着第 3 节看一帧画面的旅程 |
| 第 3 段（30 分钟） | 打开 `apps/client/controller/attendancecontroller.cpp`，看客户端那一半 |
| 第 4 段（30 分钟） | 把程序跑起来，一边操作一边在代码里找对应的那几行 |
| 第 5 段（30 分钟） | 做第 7 节的自测题，答不上来的地方记下来 |

**为什么把 `servercontroller.cpp` 放在最中间**：它是整个项目的大脑。594 行，
别的文件都是它的手脚。把这个文件读顺了，项目就没有黑盒了。

---

## 1. 一句话和一张总图

**人站在摄像头前，系统认出他是谁，然后记一笔考勤。**

```
+-------------------------+                      +-------------------------+
|        客户端            |                      |        服务端            |
|                         |   一条 TCP 连线       |                         |
|  +-------------------+  |  127.0.0.1:45454     |  +-------------------+  |
|  |    界面层          |  |                      |  |    界面层          |  |
|  |   MainWindow      |  |                      |  |  ServerWindow     |  |
|  +--------+----------+  |                      |  +--------+----------+  |
|           |             |                      |           |             |
|  +--------+----------+  |                      |  +--------+----------+  |
|  |    业务层          |  |                      |  |    业务层          |  |
|  | AttendanceControl |  |                      |  | ServerController  |  |
|  +--------+----------+  |                      |  +--------+----------+  |
|           |             |                      |           |             |
|  +--------+----------+  |                      |  +--------+----------+  |
|  |    底层            |  |                      |  |    底层            |  |
|  | 摄像头 / 检测       |--+----------------------+->| 识别 / 数据库      |  |
|  | 网络发送            |<-+----------------------+--| 网络接收           |  |
|  +-------------------+  |                      |  +-------------------+  |
+-------------------------+                      +-------------------------+
```

**两个程序，每个程序都是三层，三层干的事完全一样：**

| 层 | 干什么 | 不知道什么 |
| --- | --- | --- |
| 界面层 | 显示、收集用户的点击和输入 | 不知道数据存在哪、不知道怎么识别 |
| 业务层 | 做判断、协调各方 | 不知道按钮长什么样 |
| 底层 | 干具体活（开摄像头、查库、发消息） | 不知道界面存在 |

---

## 2. 最重要的一条规则：往下直接调，往上发信号

这是理解整个项目的钥匙。**模块之间的通信方式是不对称的。**

**向下传（业务层要用下面那层的东西）：直接调用方法。**

```cpp
// ServerController 里
window_.appendStatusText("员工新增成功");   // 直接调界面
employeeRepository_.addEmployee(...);      // 直接调数据库
server_.sendToPeer(peer, payload);         // 直接调网络
```

**向上传（底层有事情要报告）：发信号。**

```cpp
// ServerWindow 里（它不直接调 controller）
emit addEmployeeRequested(工号, 姓名, 部门);

// Server 里（它不直接调 controller）
emit messageReceived(peer, message);
```

**为什么不能两边都直接调用？**

因为底层是通用的。`Server` 这个类只负责"收字节、发字节"，
它压根不知道上面有个窗口、有个数据库。如果它直接调 `controller_->onMessageReceived()`，
那它就被绑死在这个项目上了，换个项目就没法复用。

发信号的好处是：**它只管喊一嗓子"我收到消息了"，谁想听谁去听。**
它自己不知道、也不关心有谁在听。

**这条规则在代码里的样子**（都在 controller 的构造函数里连好）：

| 谁发信号 | 发给谁 | 干什么 |
| --- | --- | --- |
| `ServerWindow::addEmployeeRequested` | controller | 加了新员工 |
| `ServerWindow::deleteEmployeeRequested` | controller | 要删员工 |
| `ServerWindow::updateEmployeeRequested` | controller | 改了员工 |
| `ServerWindow::refreshEmployeesRequested` | controller | 要刷新列表 |
| `ServerWindow::registerFaceRequested` | controller | 要登记人脸 |
| `Server::clientConnected` | controller | 客户端连上了 |
| `Server::clientDisconnected` | controller | 客户端断了 |
| `Server::messageReceived` | controller | 收到一条消息 |
| `Server::clientError` | controller | 网络出错 |
| `Server::listeningStarted` | **直接连到界面** | 只是显示一行日志 |

**最后一条是特例**，值得注意：监听成功这条信号没有走 controller，直接连到了窗口的
`appendStatusText`。因为"显示一行字"不需要任何业务处理，绕一层没意义。

**看到这个特例要能反应过来**：规则是"向上发信号"，但**信号连给谁是可以选的**，
需要业务处理就连给 controller，只是显示就直接连给界面。

---

## 3. 两条链路走一遍

读懂这个项目，就把这两条链路在脑子里跑一遍。**一条是本地的（服务端自己干），
一条是跨网络的（客户端和服务端配合）。**

### 链路 A：服务端本地操作，点一下"新增员工"

```
用户点按钮
   |
   v
ServerWindow 的 lambda 读三个输入框
   |  emit addEmployeeRequested(工号, 姓名, 部门)      <- 向上：发信号
   v
ServerController::onAddEmployee
   |
   +- 1. 去掉字符串两头的空格（trimmed）
   +- 2. 工号或姓名为空？-> 提示并返回
   +- 3. 调 findByEmployeeNo 查重                     <- 向下：直接调用
   |     查到 -> 提示"工号已存在"并返回
   +- 4. 调 addEmployee -> 写进数据库                  <- 向下：直接调用
   +- 5. window_.appendStatusText("员工新增成功")       <- 向下：直接调用
   +- 6. loadEmployees() -> 重新查一遍全表 -> 刷新列表和计数
```

**这条链路上有个设计要点**：第 3 步的查重和第 4 步的写入是**两道防线**。

为什么查完重还要靠数据库的唯一约束兜底？因为第 3 步和第 4 步之间有间隙，
理论上两个人同时操作会都通过检查。真正的保证来自数据库的 `UNIQUE` 约束，
查重只是为了给用户一句人话（不然用户看到的是一串 SQL 报错）。

**"约束保正确性，前置检查保体验"**，这句话可以直接用在面试里。

### 链路 B：一帧画面的完整旅程（核心）

这是整个项目最长的一条链路，跨了两个程序。**跟着它走一遍，项目就没有黑盒了。**

**客户端这一半：**

```
(1) previewTimer_ 每 33 毫秒触发一次
    -> AttendanceController::processFrame()

(2) camera_.read(frame)          拿到一帧 -> cv::Mat（彩色，BGR 顺序）

(3) FrameProcessor::mirror(frame) 水平翻转，前置摄像头要像照镜子

(4) 每 100 毫秒才做一次：
    faceDetector_.detect(frame, detectedFaces_)
      YuNet 找出人脸框，只用来在预览上画绿框
      window_.setFaceCount(几个)        <- 向下："检测到人脸数量：1"

(5) FrameProcessor::toPreviewImage(frame, 框)
      变成 QImage -> window_.showPreviewImage()   显示给人看

(6) 每 500 毫秒才做一次（而且要连着服务端）：
    FrameProcessor::encodeJpeg(image, 80)  压缩成 JPEG 字节
      前面拼上 "JPEG\n"
      networkClient_.sendPayload(payload) <- 向下：直接调用

(7) NetworkClient::sendPayload
      FrameProtocol::pack()  前面加 4 字节长度
      socket_.write()        发出去
```

**这里三个"频率"要分清，面试爱问：**

| 频率 | 干什么 | 为什么不更快 |
| --- | --- | --- |
| 33 毫秒 | 读摄像头、刷新预览 | 这是人眼感觉流畅的下限 |
| 100 毫秒 | 本地检测画框 | 检测是神经网络，每帧跑会卡住界面 |
| 500 毫秒 | 上传给服务端 | 带宽和 CPU 吃不消，而人脸半秒内不会变 |

**服务端这一半：**

```
(8)  QTcpSocket 收到字节 -> ClientConnection::onReadyRead
       全部追加到 receiveBuffer_（缓冲）
       FrameProtocol::takeFrame()
         数据不够 -> Incomplete，继续等（这就是"半包"）
         切出一整条 -> Complete
       emit messageReceived(一条完整消息)     <- 向上：发信号

(9)  Server 转发信号，并配上"是谁发来的"
       emit Server::messageReceived(peer, message)

(10) ServerController::onMessageReceived(peer, message)
       开头不是 "JPEG\n"？-> 是文本消息，记一行日志就结束
       是图像 -> message.mid(5) 切掉前缀，剩下的才是真图片

(11) faceEngine_.analyzeJpeg(图片字节, faces)      <- 向下：直接调用
       cv::imdecode()              字节 -> cv::Mat
       detector_->setInputSize()   告诉模型这帧多大
       detector_->detect()         得到 N 行 15 列的表
         前 4 列是框、再 10 列是 5 个关键点、最后 1 列是置信度

(12) 挑出置信度最高的那张脸（不能取第一行，输出顺序没有文档承诺）

(13) recognizer_->alignCrop()  用 5 个关键点把脸摆正，裁成 112x112
     recognizer_->feature()    变成 128 个小数
       顺手存进 lastEmbedding_（"登记人脸"要用它）

(14) recognize(embedding)                        <- 内部往下调 FaceEngine
       遍历数据库里的每一个模板：
         fromBytes -> 把 BLOB 还原成 128 个数
         similarity -> 算余弦相似度
       取最高的那个

(15) 状态变了吗？（比"有没有脸 / 认出没有 / 认出的是谁"，不比那行文字）
       没变 -> 什么都不做（这一帧就结束了）
       变了 -> 继续往下

(16) 认出人了？
       recordAttendance(员工id, 相似度)
         hasRecordWithin(120 秒)   最近记过吗
           记过 -> 返回"120 秒内已经记过一次"
           没记过 -> addRecord() 写一条

(17) sendResult(peer, 结果, 考勤结论)             <- 向下：直接调用
       拼成 "RESULT + 制表符 + 身份 + 制表符 + 考勤结论"
       server_.sendToPeer(peer, payload)   只回给发消息的那个客户端
```

**回到客户端：**

```
(18) socket 收到字节 -> NetworkClient::onReadyRead
       同一套 takeFrame 切帧
       emit messageReceived

(19) AttendanceController::onServerMessage
       开头不是 "RESULT"？-> 不是给它的，忽略
       按制表符拆成两个字段
       window_.setResultText("张三 · 考勤已记录：签到")
```

**这条链路走完，你应该注意到一件事：**

**同一个"切帧"逻辑，服务端和客户端各调了一次；同一个"长度前缀"格式，
两边都遵守。** 这就是为什么它被放进 `common` 目录：**协议是双方共用的规则，
必须只有一份实现。**

---

## 4. 模块职责一览

每个模块只做一件事，并且**不知道别人的事**。

### 客户端

| 文件 | 一句话职责 | 它不知道什么 |
| --- | --- | --- |
| `camera/CameraController` | 包着 OpenCV 摄像头：打开、读一帧、释放 | 不知道网络、不知道界面 |
| `vision/FaceDetector` | 给一张彩色图，返回人脸框的坐标 | 不知道人脸是谁 |
| `vision/FrameProcessor` | 图像的几种变换：镜像、转 Qt 图片、画框、编码 JPEG | 不知道这些图要拿去干嘛 |
| `network/NetworkClient` | 管一条 TCP 连接：连接、发送、接收、自动重连 | 不知道发的是什么内容 |
| `ui/MainWindow` | 窗口长什么样、按钮在哪、点一下发个信号 | 不知道摄像头、不知道数据库 |
| `controller/AttendanceController` | 把上面五个串起来 | —— |
| `main.cpp` | 把对象创建出来、组装、启动 | —— |

### 服务端

| 文件 | 一句话职责 | 它不知道什么 |
| --- | --- | --- |
| `database/DatabaseManager` | 管数据库连接：打开、建表、关闭 | 不知道表里存的是什么业务 |
| `database/EmployeeRepository` | 员工的增删改查 | 不知道界面长什么样 |
| `database/FaceTemplateRepository` | 人脸模板的存取 | 不知道模板是怎么算出来的 |
| `database/AttendanceRepository` | 写考勤、查"最近有没有记过" | 不知道谁是员工 |
| `vision/FaceEngine` | 加载模型、检测人脸、提特征、比相似度 | 不知道数据库存在 |
| `network/ClientConnection` | 代表"一个连上来的客户端"，收它的字节 | 不知道消息内容 |
| `network/Server` | 监听端口，管理所有连接 | 不知道谁是谁 |
| `ui/ServerWindow` | 服务端窗口长什么样 | 不知道数据从哪来 |
| `controller/ServerController` | 把上面八个串起来，所有判断都在这里 | —— |
| `main.cpp` | 组装、启动 | —— |

### 公共和测试

| 文件 | 一句话职责 |
| --- | --- |
| `common/FrameProtocol` | 消息的分帧规则：打包、切帧。**协议只有这一份实现** |
| `tests/tst_frameprotocol` | 验证分帧规则的各种边界情况 |
| `tests/tst_database` | 验证数据库几个仓储的行为 |

---

## 5. 数据形态变化表

这是我觉得最有意思的一张表。**一帧画面在流转过程中，形态变了十几次**，
每一层只认识自己那种形态。

| 在哪 | 数据是什么形态 |
| --- | --- |
| 摄像头 | `cv::Mat`，彩色，BGR 顺序 |
| 本地检测 | 同上，额外多一个"框的坐标列表" |
| 预览显示 | `QImage`（Qt 的类型，RGB 顺序） |
| 网络发送前 | `QByteArray`，里面是 JPEG 压缩后的字节 |
| 加上协议头 | 前面多 4 个字节的长度 |
| 网络传输 | 纯粹的字节流，没有边界 |
| 服务端缓冲 | 还是字节，堆在 `receiveBuffer_` 里 |
| 切帧之后 | 一条完整的消息（`QByteArray`） |
| 解码之后 | `cv::Mat`（又变回图像） |
| 检测输出 | `cv::Mat`，N 行 15 列的数字表 |
| 对齐之后 | `cv::Mat`，112x112 的人脸 |
| 提特征之后 | `cv::Mat`，1 行 128 列的浮点数 |
| 存进数据库 | 512 字节的 BLOB |
| 比对结果 | 一个 `double`（余弦相似度） |
| 回传消息 | `QByteArray`，`RESULT` 加两个制表符分隔的字段 |
| 客户端收到 | `QByteArray` |
| 最后显示 | `QString`，贴在 `QLabel` 上 |

**为什么要看这张表？** 因为"数据在哪一层变成了什么"决定了"哪一层该负责什么"。

比如为什么解 JPEG 这件事放在服务端而不是客户端？因为客户端**本来就有** `cv::Mat`，
它压缩是为了省带宽；服务端**只想拿到像素**，所以解压归它。**责任跟着数据形态走。**

---

## 6. 如果让你从零设计，应该怎么想

这一节回答你的问题："我自己设计不一定能写出来"。**设计是可以按步骤推出来的**，
不是靠灵感。

### 第一步：先想清楚"数据从哪来、到哪去"

不要先想类怎么分。先拿张纸画数据的流向：

```
摄像头 -> 画面 -> ? ? ? -> 数据库
                    ^
              中间要发生什么？
```

答案是：要判断"这张脸是谁"。判断需要两样东西：**能算脸的模型**和**存过模板的数据库**。

### 第二步：把"要判断"和"要显示"分开

凡是**需要做判断**的，放在业务层；凡是**只是显示或者收集输入**的，放在界面层；
凡是**具体干一件技术活**的（连数据库、开摄像头、发网络包），放在底层。

用你项目的例子对一遍：

- "姓名不能为空" -> 判断 -> 业务层
- "这张脸已经登记给别人了" -> 判断 -> 业务层
- "显示一行日志" -> 显示 -> 界面层
- "把这条记录写进数据库" -> 技术活 -> 底层

**分不清的时候问一句：这句话会出现在需求文档里，还是会出现在技术文档里？**
需求 -> 业务层，技术 -> 底层。

### 第三步：找出哪些东西是"一对多"或者"需要长期存在"的

- 数据库连接：全局一个，长期存在 -> 做成一个管理器
- 客户端的连接：可能有多个 -> 每个连接做成一个对象，再由 Server 管着它们
- 摄像头：一个 -> 一个类

### 第四步：决定边界怎么切

**切割的标准是"变化的方向一致"。**

`FrameProcessor` 和 `CameraController` 为什么分开？因为**换摄像头的方式和
换图像处理方式完全是两回事**。相机驱动出问题不会影响 JPEG 编码，反过来也一样。

而 `ServerController` 里面为什么塞了那么多东西？因为它管的是**这个项目的业务规则**，
这些规则天生是一起的。

**一条实用的判断：如果两块代码总是一起改，就放在一起；总是分别改，就切开。**

### 第五步：最后才想"怎么把它们连起来"

先用信号槽把"往上报告"的路径连好，再在 controller 里写"往下调用"的逻辑。

**这也是为什么 `main.cpp` 很短**：它只负责最后一步，把对象创建出来、连好信号槽。
所有"怎么做事"的知识都在 controller 里，`main` 只负责"把零件凑齐"。

---

## 7. 自测：这十个问题答得上来，就算读懂了

1. 客户端和服务端各自的三层分别是什么？界面层为什么不能直接连数据库？
2. 为什么"往上报告"要用信号，"往下指挥"要直接调用？两边都直接调用会有什么问题？
3. `Server::listeningStarted` 为什么直接连到窗口，而 `messageReceived` 要先经过 controller？
4. 一帧画面从摄像头到数据库，形态变了多少次？至少说出五种。
5. 客户端为什么有三个不同的频率（33 / 100 / 500 毫秒）？
6. 分帧这件事为什么放在 `common` 里，而不是客户端服务端各写一份？
7. 新增员工时，既然已经查过重了，为什么还要靠数据库的唯一约束？
8. 识别成功后，为什么不是每一帧都写一条考勤？
9. 如果让你加一个"签退"功能，你会改哪几个文件？为什么？
10. 为什么 `main.cpp` 只有四十多行？如果所有逻辑都写在 `main` 里会怎样？
