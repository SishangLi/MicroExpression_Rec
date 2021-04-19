// BuildDllE.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "BuildDllE.h"

using namespace cv;
using namespace std;


// 这是导出变量的一个示例
BUILDDLLE_API int nBuildDllE=0;

// 这是导出函数的一个示例。
BUILDDLLE_API int fnBuildDllE(void)
{
    return 0;
}

// 这是已导出类的构造函数。
CBuildDllE::CBuildDllE()
{
    return;
}

// ExpressionAnalyzer
// From the code of https://github.com/tgpcai/Microexpression_recognition
AttDetector::AttDetector() {

}

AttDetector::~AttDetector() {

}

// 初始化模型，需要传入配置文件
int AttDetector::Initmodel(const char* cfgfile) {
	INIReader reader(cfgfile);
	if (reader.ParseError() != 0) {
		std::cerr << "Can't load 'config.ini' \n";
		return -1;
	}
	this->cfg_file = cfgfile;
	std::cout << "Configuration file read successfully : " << cfgfile << std::endl;
	//Read Parameters
	try
	{
		this->frequency = reader.GetInteger("Parameters", "Frequency", 2);
		this->show_information = reader.GetBoolean("Parameters", "Show_Information", true);
		if (this->GetEmotions(reader.Get("Parameters", "Emotions", "none"), this->emotions)) {
			std::cerr << "Error load emotions!" << endl;
			return -1;
		}
		std::cout << "Emotions loaded successfully ! " << std::endl;
	}
	catch (const std::exception&)
	{
		std::cerr << "Error load Parameters!" << endl;
		return -1;
	}

	//读取并加载人脸检测模型
	string FACE_DETECTOR = reader.Get("Model", "Face_Detector", "./model/haarcascade_frontalface_alt22.xml");
	this->face_detector.load(FACE_DETECTOR);
	if (face_detector.empty())
	{
		std::cerr << "Can't load weight of face detector from : " << FACE_DETECTOR << std::endl;
		return -1;
	}
	std::cout << "Face Detector loaded successfully ! " << std::endl;

	//读取并加载表情分析器
	string EXPRESSION_ANALYZER = reader.Get("Model", "Expression_Analyzer", "./model/output_graph.pb");
	this->experession_analyzer = dnn::readNetFromTensorflow(EXPRESSION_ANALYZER);
	if (experession_analyzer.empty())
	{
		std::cerr << "Can't load weight of experession-analyzer from : " << EXPRESSION_ANALYZER << std::endl;
		return -1;
	}
	std::cout << "Experession Analyzer loaded successfully ! " << std::endl;

	std::cout << "Config loaded from cfgfile : Version="
		<< reader.GetInteger("Protocol", "Version", -1) << ", Name="
		<< reader.Get("Information", "Name", "UNKNOWN") << ", Email="
		<< reader.Get("Information", "Email", "UNKNOWN") << ", Pi="
		<< reader.GetReal("Information", "Pi", -1) << ", Active="
		<< reader.GetBoolean("Information", "Active", true) << "\n";
	return 0;
}

//开始检测函数，阻塞式进程，直到发送停止信号，
int AttDetector::Start() {
	if (!videocap.isOpened())
	{
		videocap.open(0);
		if (!videocap.isOpened())
		{
			cout << "Cannot open the video file" << endl;
			return -1;
		}
	}
	Mat frame, img_gray;
	int step = 0;
	vector<Rect> faces;
	Rect face_rect;
	this->start_exit = false;
	while (true)
	{
		// read frame from camera
		videocap >> frame;
		if (frame.empty()) {
			cerr << "ERROR! Blank frame grabbed\n";
			break;
		}
		//clear vetcor cache
		faces.clear();
		// rgb2gray
		cvtColor(frame, img_gray, COLOR_BGR2GRAY);
		//image, objects, scaleFactor, minNeighbors, 
		face_detector.detectMultiScale(img_gray, faces, 1.1, 3, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
		// if find face, then we find the max face
		if (faces.size() > 0) {
			auto max_face_rect = faces[0];
			//find max face
			for (auto face : faces)  if (face.area() > max_face_rect.area()) max_face_rect = face;
			face_rect = max_face_rect;
			//crop and resize face from frame 
			img_gray = Mat(img_gray, max_face_rect);
			try
			{
				resize(img_gray, img_gray, Size(48, 48));
			}
			catch (const std::exception&)
			{
				cerr << "Problem during resize !\n";
			}
		}
		// if step is ok, analyse the expression
		if (step == (100 / this->frequency)) {
			step = 0;
			Mat inputBlob = dnn::blobFromImage(img_gray, 1.0f, Size(1, 2304));
			this->experession_analyzer.setInput(inputBlob);
			auto result = this->experession_analyzer.forward();
			if (!result.empty()) {
				this->cur_analyze_result = -1;
				int max_ration = -1;
				for (int ncol = 0; ncol < result.cols; ncol++) {
					if (result.at<float>(0, ncol) > max_ration) {
						max_ration = (int)result.at<float>(0, ncol);
						this->cur_analyze_result = ncol;
					}
				}
				AttentionRatioCompute();
			}
		}
		step++;
		//Show detect information
		if (this->show_information) {
			rectangle(frame, face_rect, Scalar(0, 255, 0), 2);
			for (int i = 0; i < emotions.size(); i++) {
				Point text_point; text_point.x = 10; text_point.y = i * 20 + 20;
				putText(frame, emotions[i], text_point, 5, 1, (0, 0, 255));
				Point rect_point_1; rect_point_1.x = 130; rect_point_1.y = i * 20 + 10;
				Point rect_point_2; rect_point_2.x = 130 + int(this->attention_ratio[i] * 100); rect_point_2.y = (i + 1) * 20 + 4;
				rectangle(frame, rect_point_1, rect_point_2, Scalar(0, 255), -1);
			}
		}
		//Display on Screen
		imshow("Display", frame);
		if (char(waitKey(10)) == 'q' || this->stop) {
			destroyWindow("Display");
			this->start_exit = true;
			break;
		}
	}
}

//停止检测函数
int AttDetector::Stop() {
	this->stop = true;
	int delay = 1000;
	while (!this->start_exit || delay <= 0) {
		Sleep(100);
		delay -= 100;
	}
	if (this->start_exit)return -1;
	else return 0;
}

//将当前时间点的分析结果添加到这之前的时间段里
void  AttDetector::AttentionRatioCompute() {
	if (cur_analyze_result < 0) {
		std::cerr << "Anaylze Error!" << endl;
		return;
	}
	else
	{
		this->attention_ratio[cur_analyze_result] += 1;
		float sum = 0;
		for (int i = 0; i < 7; i++) sum += attention_ratio[i];
		for (int i = 0; i < 7; i++) attention_ratio[i] = attention_ratio[i] / sum;
	}
}

//返回当前一段时间的检测结果
int AttDetector::GetAttentionResult(vector<float>& result) {
	try
	{
		if ( result.size() != 7 ) result.resize(7, 0.0);
		for (int i = 0; i < 7; i++) result[i] = this->attention_ratio[i];
		return 0;
	}
	catch (const std::exception&)
	{
		std::cerr << "GetAttentionResult Error!" << endl;
		return -1;
	}
}

//从配置文件中读取表情，存入类属性中
int AttDetector::GetEmotions(const string& s, vector<string>& tokens, const string& delimiters) {
	if (s == "none") {
		return -1;
	}
	else
	{
		try
		{
			string::size_type lastPos = s.find_first_not_of(delimiters, 0);
			string::size_type pos = s.find_first_of(delimiters, lastPos);
			while (string::npos != pos || string::npos != lastPos) {
				tokens.push_back(s.substr(lastPos, pos - lastPos));//use emplace_back after C++11
				lastPos = s.find_first_not_of(delimiters, pos);
				pos = s.find_first_of(delimiters, lastPos);
			}
			return 0;
		}
		catch (const std::exception&)
		{
			return -1;
		}
	}
}

//获取表情名函数
string AttDetector::GetEmotion(int i) {
	return this->emotions[i];
}
