// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 BUILDDLLE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// BUILDDLLE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef BUILDDLLE_EXPORTS
#define BUILDDLLE_API __declspec(dllexport)
#else
#define BUILDDLLE_API __declspec(dllimport)
#endif

#include <vector>
#include <string>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <windows.h>

using namespace std;
using namespace cv;

// 此类是从 dll 导出的
class BUILDDLLE_API CBuildDllE {
public:
	CBuildDllE(void);
	// TODO: 在此处添加方法。
};

// ExpressionAnalyzer
class BUILDDLLE_API AttDetector {
public:
	AttDetector();
	~AttDetector();

	int Initmodel(const char* cfgfile);
	int Start();
	int Stop();
	int GetAttentionResult(vector<float>& result);
	string GetEmotion(int i);


private:
	void AttentionRatioCompute();
	int GetEmotions(const string& s, vector<string>& tokens, const string& delimiters = ",");

private:
	int frequency;
	bool show_information = true;
	string cfg_file;
	CascadeClassifier face_detector;
	dnn::Net experession_analyzer;
	VideoCapture videocap;
	vector<string> emotions;
	int cur_analyze_result = -1;
	float attention_ratio[7] = { 0.0 };
	bool stop = false;
	bool start_exit = false;
};

extern BUILDDLLE_API int nBuildDllE;

BUILDDLLE_API int fnBuildDllE(void);
