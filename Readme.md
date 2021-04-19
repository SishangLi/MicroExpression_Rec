## MicroExpression_Rec
## 微表情识别

### 综述
- 本项目为C++实现的微表情识别，通过读取摄像头画面，分析画面中的人脸表情
- 项目基于opencv（4.5.1）采用C++语言开发，封装为动态链接库，使用时可通过.h文件直接调用
- 本项目参考[微表情识别](https://github.com/tgpcai/Microexpression_recognition)，表情识别模型也来自此项目，在此鸣谢！

### 运行环境
- 本项目的编译需要opencv（4.5.1），若使用opencv动态库编译，则需要在运行dll的环境中同样配置opencv环境，因此推介使用opencv静态库编译dll。
- 通过opencv静态库编译的dll文件会略大，这是因为dll集成了opencv库函数。经过opencv静态库编译生成的动态dll在部署环境中不在需要配置opencv环境。
- 建议再Release X64模式下编译

### 函数说明
- BuildDllE工程提供一个```AttDetector```类及其实现，编译生成一个dll动态链接库
- ```int Initmodel(const char* cfgfile);```： 初始化人脸检测模型和表情分析模型，需要传入配置文件路径；
- ```int Start();``` : 开始检测函数，建议通过使用建立多线程任务来启动，示例代码见TestDll.cpp
- ```int Stop();```：停止检测函数
- ```int GetAttentionResult(vector<float>& result);``` : 获取从开始到当前时间点各种表情的占比
- ```string GetEmotion(int i);```：指定序号，返回表情名

