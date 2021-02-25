// ConsoleApplication2.cpp : �������̨Ӧ�ó������ڵ㡣
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
	ifstream fin("calibdata.txt");//����ͼƬ·��
	ofstream fout("caliberation_result.txt");//�궨���

	//��ȡ�ǵ�
	int img_num = 0;
	//vector<Size> img_size;
	Size img_size;  //ͼ���С
	Size board_size = Size(4, 6);   //�С��еĽǵ���
	vector<Point2f> img_points_buf; //�ǵ㻺�� 
	vector<vector<Point2f>> img_points_seq;//�洢���нǵ�
	int count = -1;//�ǵ����
	Size temp_Size;
	string picpath;
	while (getline(fin, picpath))//��calibdata.txt�ж�ȡ����ͼƬ·��
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
			find4QuadCornerSubpix(view_gray, img_points_buf, Size(5, 5)); //�Դ���ȡ�Ľǵ���о�ȷ��
			img_points_seq.push_back(img_points_buf);  //���������ؽǵ�
			// ��ͼ������ʾ�ǵ�λ�� 
			//drawChessboardCorners(view_gray, board_size, img_points_buf, true); //������ͼƬ�б�ǽǵ�
			//imshow("Camera Calibration", view_gray);//��ʾͼƬ
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
	int CornerNum = board_size.width*board_size.height;  //ÿ��ͼƬ���ܵĽǵ���
	cout << "CornerNum : " << CornerNum << endl;
	for (int i = 0; i < total; i++)
	{
		cout << " �� " << i + 1 << "ͼƬ������: " << endl;
		for (int j = 0; j < CornerNum; j++)
		{
			if (0 == j % 4) cout << endl;
			cout << "��" << setw(7) << img_points_seq[i][j].x << "," << setw(7) << img_points_seq[i][j].y << ") ";
		}
		cout << endl;
	}
	//�ǵ���ȡ���
	//������궨

	Size square_size = Size(10, 10);//���̸��С
	vector<vector<Point3f>> object_points;//�ǵ����ά����

	/*�������*/
	Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); //������ڲ�������
	vector<int> point_counts;  // ÿ��ͼ���нǵ������
	Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0)); // �������5������ϵ����k1,k2,p1,p2,k3
	vector<Mat> tvecsMat;  //ÿ��ͼ�����ת���� 
	vector<Mat> rvecsMat; //ÿ��ͼ���ƽ������ 
	//��ʼ���궨���Ͻǵ����ά���� 
	int i, j, t;
	for (t = 0; t<img_num; t++)
	{
		vector<Point3f> tempPointSet;
		for (i = 0; i<board_size.height; i++)
		{
			for (j = 0; j<board_size.width; j++)
			{
				Point3f realPoint;
				/* ����궨�������������ϵ��z=0��ƽ���� */
				realPoint.x = i*square_size.width;
				realPoint.y = j*square_size.height;
				realPoint.z = 0;
				tempPointSet.push_back(realPoint);
			}
		}
		object_points.push_back(tempPointSet);
	}
	// ��ʼ��ÿ��ͼ���еĽǵ��������ٶ�ÿ��ͼ���ж����Կ��������ı궨�� 
	for (i = 0; i<img_num; i++)
	{
		point_counts.push_back(board_size.width*board_size.height);
	}
	cout << "here" << endl;
	// ��ʼ�궨 

	calibrateCamera(object_points, img_points_seq, img_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, 0);

	cout << "�궨��ɣ�\n";

	//�Ա궨�����������
	cout << "��ʼ���۱궨���������������\n";
	double total_err = 0.0; // ����ͼ���ƽ�������ܺ�
	double err = 0.0; // ÿ��ͼ���ƽ����� 
	vector<Point2f> img_points2; // �������¼���õ���ͶӰ�� 
	cout << "\tÿ��ͼ��ı궨��\n";
	fout << "ÿ��ͼ��ı궨��\n";
	for (i = 0; i<img_num; i++)
	{
		vector<Point3f> tempPointSet = object_points[i];
		// ͨ������õ�������ڲκ���Σ�����ά�ռ�����½���ͶӰ���õ��궨���ͼ�����꣬����ͼ��������ԭ�ǵ�����֮���ƫ�ƫ��ԽС��궨Ч��Խ�á�
		projectPoints(tempPointSet, rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, img_points2);
		// �����µ�ͶӰ��;ɵ�ͶӰ��֮������
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
		std::cout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
		fout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
	}
	std::cout << "����ƽ����" << total_err / img_num << "����" << endl;
	fout << "����ƽ����" << total_err / img_num << "����" << endl << endl;
	std::cout << "������ɣ�" << endl;
	//���涨����  	
	std::cout << "��ʼ���涨����������������" << endl;
	Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); // ����ÿ��ͼ�����ת���� 

	fout << "����ڲ�������" << endl;
	fout << cameraMatrix << endl << endl;
	fout << "����ϵ����\n";
	fout << distCoeffs << endl << endl << endl;
	for (int i = 0; i<img_num; i++)
	{
		fout << "��" << i + 1 << "��ͼ�����ת������" << endl;
		fout << tvecsMat[i] << endl;
		//����ת����ת��Ϊ���Ӧ����ת���� 
		Rodrigues(tvecsMat[i], rotation_matrix);
		fout << "��" << i + 1 << "��ͼ�����ת����" << endl;
		fout << rotation_matrix << endl;
		fout << "��" << i + 1 << "��ͼ���ƽ��������" << endl;
		fout << rvecsMat[i] << endl << endl;
	}
	std::cout << "��ɱ���" << endl;
	fout << endl;

	//��ʾ������

	Mat mapx = Mat(img_size, CV_32FC1);
	Mat mapy = Mat(img_size, CV_32FC1);
	Mat R = Mat::eye(3, 3, CV_32F);
	std::cout << "�������ͼ��" << endl;
	string imageFileName;
	std::stringstream StrStm;
	for (int i = 0; i != img_num; i++)
	{


		std::cout << "Frame #" << i + 1 << "..." << endl;
		initUndistortRectifyMap(cameraMatrix, distCoeffs, R, cameraMatrix, img_size, CV_32FC1, mapx, mapy);
		StrStm.clear();
		imageFileName.clear();

		//��ȡͼ���ļ���
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
		//imshow("ԭʼͼ��", imageSource);
		//imshow("������ͼ��", newimage);
		imshow("����ǰ��Ա�", mergeimg);
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
	std::cout << "�������" << endl;
	return;
}

