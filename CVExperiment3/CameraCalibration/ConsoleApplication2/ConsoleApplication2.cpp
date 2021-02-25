// ConsoleApplication2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace cv;
using namespace std;

void main()
{
	ifstream fin("calibdata.txt");//棋盘图片路径
	ofstream fout("caliberation_result.txt");//标定结果

	//提取角点
	int img_num = 0;
	//vector<Size> img_size;
	Size img_size;  //图像大小
	Size board_size = Size(4, 6);   //行、列的角点数
	vector<Point2f> img_points_buf; //角点缓存 
	vector<vector<Point2f>> img_points_seq;//存储所有角点
	int count = -1;//角点个数
	Size temp_Size;
	string picpath;
	while (getline(fin, picpath))//从calibdata.txt中读取棋盘图片路径
	{
		img_num++;
		cout << "Number of Image :" << img_num << endl;
		Mat imgInput = imread(picpath);
		//imshow("InputImage", imgInput);
		//temp_Size.width = imgInput.cols;
		//temp_Size.height = imgInput.rows;
		//img_size.push_back(temp_Size);
		img_size.width = imgInput.cols;
		img_size.height = imgInput.rows;
		//cout << "Image Width :" << img_size[img_num-1].width << endl;
		//cout << "Image Height :" << img_size[img_num - 1].height << endl;

		bool isFind = findChessboardCorners(imgInput, board_size, img_points_buf);
		if (0 == isFind) cout << "Can not find corners" << endl;
		if (isFind)
		{
			Mat view_gray;
			cvtColor(imgInput, view_gray, CV_RGB2GRAY);
			find4QuadCornerSubpix(view_gray, img_points_buf, Size(5, 5)); //对粗提取的角点进行精确化
			img_points_seq.push_back(img_points_buf);  //保存亚像素角点
			// 在图像上显示角点位置 
			//drawChessboardCorners(view_gray, board_size, img_points_buf, true); //用于在图片中标记角点
			//imshow("Camera Calibration", view_gray);//显示图片
			drawChessboardCorners(imgInput, board_size, img_points_buf, true);
			

			string imageFileName;
			std::stringstream StrStm;
			StrStm.clear();
			StrStm << "Corners_";
			StrStm << img_num;
			StrStm >> imageFileName;
			imageFileName += ".jpg";
			imwrite(imageFileName, imgInput);
			imshow("Camera Calibration", imgInput);

			waitKey(500);
		}
	}
	int total = img_points_seq.size();
	cout << "total : " << total << " corner :" << img_points_seq[0].size() << endl;
	int CornerNum = board_size.width*board_size.height;  //每张图片上总的角点数
	cout << "CornerNum : " << CornerNum << endl;
	for (int i = 0; i < total; i++)
	{
		cout << " 第 " << i + 1 << "图片的数据: " << endl;
		for (int j = 0; j < CornerNum; j++)
		{
			if (0 == j % 4) cout << endl;
			cout << "（" << setw(7) << img_points_seq[i][j].x << "," << setw(7) << img_points_seq[i][j].y << ") ";
		}
		cout << endl;
	}
	//角点提取完成
	//摄像机标定

	Size square_size = Size(10, 10);//棋盘格大小
	vector<vector<Point3f>> object_points;//角点的三维坐标

	/*内外参数*/
	Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); //摄像机内参数矩阵
	vector<int> point_counts;  // 每幅图像中角点的数量
	Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0)); // 摄像机的5个畸变系数：k1,k2,p1,p2,k3
	vector<Mat> tvecsMat;  //每幅图像的旋转向量 
	vector<Mat> rvecsMat; //每幅图像的平移向量 
	//初始化标定板上角点的三维坐标 
	int i, j, t;
	for (t = 0; t<img_num; t++)
	{
		vector<Point3f> tempPointSet;
		for (i = 0; i<board_size.height; i++)
		{
			for (j = 0; j<board_size.width; j++)
			{
				Point3f realPoint;
				/* 假设标定板放在世界坐标系中z=0的平面上 */
				realPoint.x = i*square_size.width;
				realPoint.y = j*square_size.height;
				realPoint.z = 0;
				tempPointSet.push_back(realPoint);
			}
		}
		object_points.push_back(tempPointSet);
	}
	// 初始化每幅图像中的角点数量，假定每幅图像中都可以看到完整的标定板 
	for (i = 0; i<img_num; i++)
	{
		point_counts.push_back(board_size.width*board_size.height);
	}
	cout << "here" << endl;
	// 开始标定 

	calibrateCamera(object_points, img_points_seq, img_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, 0);

	cout << "标定完成！\n";

	//对标定结果进行评价
	cout << "开始评价标定结果………………\n";
	double total_err = 0.0; // 所有图像的平均误差的总和
	double err = 0.0; // 每幅图像的平均误差 
	vector<Point2f> img_points2; // 保存重新计算得到的投影点 
	cout << "\t每幅图像的标定误差：\n";
	fout << "每幅图像的标定误差：\n";
	for (i = 0; i<img_num; i++)
	{
		vector<Point3f> tempPointSet = object_points[i];
		// 通过计算得到的相机内参和外参，对三维空间点重新进行投影，得到标定后的图像坐标，计算图像坐标于原角点坐标之间的偏差，偏差越小则标定效果越好。
		projectPoints(tempPointSet, rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, img_points2);
		// 计算新的投影点和旧的投影点之间的误差
		vector<Point2f> tempImagePoint = img_points_seq[i];
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		Mat img_points2Mat = Mat(1, img_points2.size(), CV_32FC2);
		for (int j = 0; j < tempImagePoint.size(); j++)
		{
			img_points2Mat.at<Vec2f>(0, j) = Vec2f(img_points2[j].x, img_points2[j].y);
			tempImagePointMat.at<Vec2f>(0, j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
		}
		err = norm(img_points2Mat, tempImagePointMat, NORM_L2);
		total_err += err /= point_counts[i];
		std::cout << "第" << i + 1 << "幅图像的平均误差：" << err << "像素" << endl;
		fout << "第" << i + 1 << "幅图像的平均误差：" << err << "像素" << endl;
	}
	std::cout << "总体平均误差：" << total_err / img_num << "像素" << endl;
	fout << "总体平均误差：" << total_err / img_num << "像素" << endl << endl;
	std::cout << "评价完成！" << endl;
	//保存定标结果  	
	std::cout << "开始保存定标结果………………" << endl;
	Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); // 保存每幅图像的旋转矩阵 

	fout << "相机内参数矩阵：" << endl;
	fout << cameraMatrix << endl << endl;
	fout << "畸变系数：\n";
	fout << distCoeffs << endl << endl << endl;
	for (int i = 0; i<img_num; i++)
	{
		fout << "第" << i + 1 << "幅图像的旋转向量：" << endl;
		fout << tvecsMat[i] << endl;
		//将旋转向量转换为相对应的旋转矩阵 
		Rodrigues(tvecsMat[i], rotation_matrix);
		fout << "第" << i + 1 << "幅图像的旋转矩阵：" << endl;
		fout << rotation_matrix << endl;
		fout << "第" << i + 1 << "幅图像的平移向量：" << endl;
		fout << rvecsMat[i] << endl << endl;
	}
	std::cout << "完成保存" << endl;
	fout << endl;

	//显示定标结果

	Mat mapx = Mat(img_size, CV_32FC1);
	Mat mapy = Mat(img_size, CV_32FC1);
	Mat R = Mat::eye(3, 3, CV_32F);
	std::cout << "保存矫正图像" << endl;
	string imageFileName;
	std::stringstream StrStm;
	for (int i = 0; i != img_num; i++)
	{


		std::cout << "Frame #" << i + 1 << "..." << endl;
		initUndistortRectifyMap(cameraMatrix, distCoeffs, R, cameraMatrix, img_size, CV_32FC1, mapx, mapy);
		StrStm.clear();
		imageFileName.clear();

		//获取图像文件名
		string filePath = "chess";
		StrStm << i + 1;
		StrStm >> imageFileName;
		filePath += imageFileName;
		filePath += ".jpg";

		Mat imageSource = imread(filePath);
		Mat newimage = imageSource.clone();
		undistort(imageSource,newimage,cameraMatrix,distCoeffs);

		Mat mergeimg(img_size.height* 2, img_size.width , CV_8UC3, Scalar(255, 255, 255));
		imageSource.copyTo(mergeimg(Rect(0, 0, img_size.width, img_size.height)));
		newimage.copyTo(mergeimg(Rect(0, img_size.height, img_size.width, img_size.height)));
		//imshow("原始图像", imageSource);
		//imshow("矫正后图像", newimage);
		imshow("矫正前后对比", mergeimg);
		waitKey();
		StrStm.clear();
		filePath.clear();

		StrStm << "Prod_";
		StrStm << i + 1;
		StrStm >> imageFileName;
		imageFileName += ".jpg";
		imwrite(imageFileName, newimage);

		StrStm.clear();
		filePath.clear();
		StrStm << "Comp_";
		StrStm << i + 1;
		StrStm >> imageFileName;
		imageFileName += ".jpg";
		imwrite(imageFileName, mergeimg);
	}
	std::cout << "保存结束" << endl;
	return;
}

