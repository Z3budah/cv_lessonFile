/*****************************************************************************
*   Markerless AR desktop application.
******************************************************************************
*   by Khvedchenia Ievgen, 5th Dec 2012
*   http://computer-vision-talks.com
******************************************************************************
*   Ch3 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

////////////////////////////////////////////////////////////////////
// File includes:
#include "PatternDetector.hpp"
#include "DebugHelpers.hpp"

////////////////////////////////////////////////////////////////////
// Standard includes:
#include <cmath>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <cassert>

PatternDetector::PatternDetector(cv::Ptr<cv::FeatureDetector> detector, 
    cv::Ptr<cv::DescriptorExtractor> extractor, 
    cv::Ptr<cv::DescriptorMatcher> matcher, 
    bool ratioTest)
    : m_detector(detector)
    , m_extractor(extractor)
    , m_matcher(matcher)
    , enableRatioTest(ratioTest)
    , enableHomographyRefinement(true)
    , homographyReprojectionThreshold(3)
{
}


void PatternDetector::train(const Pattern& pattern)
{
    // Store the pattern object
	//保存模板图像数据结构体，有角点数据，描述子数据。
    m_pattern = pattern;
	
    // API of cv::DescriptorMatcher is somewhat tricky
    // First we clear old train data:
	m_matcher->clear();;//清空特征描述子训练集.

    // Then we add vector of descriptors (each descriptors matrix describe one image). 
    // This allows us to perform search across multiple images:
	//然后增加描述符向量，包含很多描述符，每一个描述符矩阵描述一个图像，这可以让我们在多个图像中进行搜索，当下就用了一个描述符
   
	std::vector<cv::Mat> descriptors(1);
    descriptors[0] = pattern.descriptors.clone(); 
	std::cout <<"描述子："<< descriptors.size() << std::endl;
	//在描述符匹配器中增加描述符。增加特征描述子用于特征描述子集训练.在之后的匹配中，已经默认有这个训练描述符子集了。
    m_matcher->add(descriptors);
	
    // After adding train data perform actual train:
	//利用指定的描述符匹配器进行训练模板匹配器，训练一个特征描述子匹配器，在匹配之前都要先进行训练的。为训练描述子建立索引树，更快查找。
    m_matcher->train();

}

void PatternDetector::buildPatternFromImage(const cv::Mat& image, Pattern& pattern) const
{
	//这个img是obj
    int numImages = 4;
    float step = sqrtf(2.0f);

    // Store original image in pattern structure
    pattern.size = cv::Size(image.cols, image.rows);
    pattern.frame = image.clone();
    getGray(image, pattern.grayImg);
    
    // Build 2d and 3d contours (3d contour lie in XY plane since it's planar)
    pattern.points2d.resize(4);
    pattern.points3d.resize(4);

    // Image dimensions
    const float w = image.cols;
    const float h = image.rows;

    // Normalized dimensions:
    const float maxSize = std::max(w,h);
    const float unitW = w / maxSize;
    const float unitH = h / maxSize;

    pattern.points2d[0] = cv::Point2f(0,0);
    pattern.points2d[1] = cv::Point2f(w,0);
    pattern.points2d[2] = cv::Point2f(w,h);
    pattern.points2d[3] = cv::Point2f(0,h);

    pattern.points3d[0] = cv::Point3f(-unitW, -unitH, 0);
    pattern.points3d[1] = cv::Point3f( unitW, -unitH, 0);
    pattern.points3d[2] = cv::Point3f( unitW,  unitH, 0);
    pattern.points3d[3] = cv::Point3f(-unitW,  unitH, 0);

    extractFeatures(pattern.grayImg, pattern.keypoints, pattern.descriptors);
}



bool PatternDetector::findPattern(const cv::Mat& image, PatternTrackingInfo& info)
{
	//这个img是背景Scene
    // Convert input image to gray
	//优化性能，因为检测角点和提取特征描述子都需要灰度化图像，这样就减小一次灰度化。
    getGray(image, m_grayImg);
    
    // Extract feature points from input gray image
	//这里是计算测试图像的角点和特征描述子。和模板图像的计算一致。
    extractFeatures(m_grayImg, m_queryKeypoints, m_queryDescriptors);
    // Get matches with current pattern
	//对测试图像的特征描述子，在训练描述子中找到一个最佳匹配，二者组成一个匹配，放在匹配集中。在下面有具体函数实现。
    getMatches(m_queryDescriptors, m_matches);//我停在这里
	//在debug模式下显示并存储匹配图
#if _DEBUG
    cv::showAndSave("Raw matches", getMatchesImage(image, m_pattern.frame, m_queryKeypoints, m_pattern.keypoints, m_matches, 100));
#endif

#if _DEBUG
    cv::Mat tmp = image.clone();
#endif

    // Find homography transformation and detect good matches
    bool homographyFound = refineMatchesWithHomography(
        m_queryKeypoints, 
        m_pattern.keypoints, 
        homographyReprojectionThreshold, 
        m_matches, 
        m_roughHomography);

    if (homographyFound)
    {
#if _DEBUG
        cv::showAndSave("Refined matches using RANSAC", getMatchesImage(image, m_pattern.frame, m_queryKeypoints, m_pattern.keypoints, m_matches, 100));
#endif
        // If homography refinement enabled improve found transformation
        if (enableHomographyRefinement)
        {
            // Warp image using found homography
            cv::warpPerspective(m_grayImg, m_warpedImg, m_roughHomography, m_pattern.size, cv::WARP_INVERSE_MAP | cv::INTER_CUBIC);
#if _DEBUG
            cv::showAndSave("Warped image",m_warpedImg);
#endif
            // Get refined matches:
            std::vector<cv::KeyPoint> warpedKeypoints;
            std::vector<cv::DMatch> refinedMatches;

            // Detect features on warped image
            extractFeatures(m_warpedImg, warpedKeypoints, m_queryDescriptors);

            // Match with pattern
            getMatches(m_queryDescriptors, refinedMatches);

            // Estimate new refinement homography
            homographyFound = refineMatchesWithHomography(
                warpedKeypoints, 
                m_pattern.keypoints, 
                homographyReprojectionThreshold, 
                refinedMatches, 
                m_refinedHomography);
#if _DEBUG
            cv::showAndSave("MatchesWithRefinedPose", getMatchesImage(m_warpedImg, m_pattern.grayImg, warpedKeypoints, m_pattern.keypoints, refinedMatches, 100));
#endif
            // Get a result homography as result of matrix product of refined and rough homographies:
            info.homography = m_roughHomography * m_refinedHomography;

            // Transform contour with rough homography
#if _DEBUG
            cv::perspectiveTransform(m_pattern.points2d, info.points2d, m_roughHomography);
            info.draw2dContour(tmp, CV_RGB(0,200,0));
#endif

            // Transform contour with precise homography
            cv::perspectiveTransform(m_pattern.points2d, info.points2d, info.homography);
#if _DEBUG
            info.draw2dContour(tmp, CV_RGB(200,0,0));
#endif
        }
        else
        {
            info.homography = m_roughHomography;

            // Transform contour with rough homography
            cv::perspectiveTransform(m_pattern.points2d, info.points2d, m_roughHomography);
#if _DEBUG
            info.draw2dContour(tmp, CV_RGB(0,200,0));
#endif
        }
    }

#if _DEBUG
    if (1)
    {
        cv::showAndSave("Final matches", getMatchesImage(tmp, m_pattern.frame, m_queryKeypoints, m_pattern.keypoints, m_matches, 100));
    }
    std::cout << "Features:" << std::setw(4) << m_queryKeypoints.size() << " Matches: " << std::setw(4) << m_matches.size() << std::endl;
#endif

    return homographyFound;
}

void PatternDetector::getGray(const cv::Mat& image, cv::Mat& gray)
{
    if (image.channels()  == 3)
        cv::cvtColor(image, gray, CV_BGR2GRAY);
    else if (image.channels() == 4)
        cv::cvtColor(image, gray, CV_BGRA2GRAY);
    else if (image.channels() == 1)
        gray = image;
}

bool PatternDetector::extractFeatures(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors) const
{
    assert(!image.empty());
    assert(image.channels() == 1);
	//特征检测器 m_detector检测模板图案中的角点，保存在keypoints中，这是一个角点的容器。
    m_detector->detect(image, keypoints,cv::Mat());
//用于展示寻找到的角点
#if _DEBUG
	cv::Mat keypoint_img;
	cv::drawKeypoints(image, keypoints, keypoint_img, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT);
	//cv::imshow("KeyPoints Image", keypoint_img);
	cv::showAndSave("KeyPoints Image", keypoint_img);
#endif
    if (keypoints.empty())
        return false;

    m_extractor->compute(image, keypoints, descriptors);
    //?if (keypoints.empty())
	if (descriptors.empty())
		return false;
    return true;
}

void PatternDetector::getMatches(const cv::Mat& queryDescriptors, std::vector<cv::DMatch>& matches)
{
    matches.clear();

    if (enableRatioTest)
    {

        // To avoid NaN's when best match has zero distance we will use inversed ratio. 
        const float minRatio = 1.f / 1.5f;
        
        // KNN match will return 2 nearest matches for each query descriptor
        m_matcher->knnMatch(queryDescriptors, m_knnMatches, 2);

        for (size_t i=0; i<m_knnMatches.size(); i++)
        {
            const cv::DMatch& bestMatch   = m_knnMatches[i][0];
            const cv::DMatch& betterMatch = m_knnMatches[i][1];

            float distanceRatio = bestMatch.distance / betterMatch.distance;
            
            // Pass only matches where distance ratio between 
            // nearest matches is greater than 1.5 (distinct criteria)
            if (distanceRatio < minRatio)
            {
                matches.push_back(bestMatch);
            }
        }
    }
    else
    {
        // Perform regular match
        m_matcher->match(queryDescriptors, matches);
    }
}

bool PatternDetector::refineMatchesWithHomography
    (
    const std::vector<cv::KeyPoint>& queryKeypoints,
    const std::vector<cv::KeyPoint>& trainKeypoints, 
    float reprojectionThreshold,
    std::vector<cv::DMatch>& matches,
    cv::Mat& homography
    )
{
    const int minNumberMatchesAllowed = 8;

    if (matches.size() < minNumberMatchesAllowed)
        return false;

    // Prepare data for cv::findHomography
    std::vector<cv::Point2f> srcPoints(matches.size());
    std::vector<cv::Point2f> dstPoints(matches.size());

    for (size_t i = 0; i < matches.size(); i++)
    {
        srcPoints[i] = trainKeypoints[matches[i].trainIdx].pt;
        dstPoints[i] = queryKeypoints[matches[i].queryIdx].pt;
    }

    // Find homography matrix and get inliers mask
    std::vector<unsigned char> inliersMask(srcPoints.size());
    homography = cv::findHomography(srcPoints, 
                                    dstPoints, 
                                    CV_FM_RANSAC, 
                                    reprojectionThreshold, 
                                    inliersMask);

    std::vector<cv::DMatch> inliers;
    for (size_t i=0; i<inliersMask.size(); i++)
    {
        if (inliersMask[i])
            inliers.push_back(matches[i]);
    }

    matches.swap(inliers);
    return matches.size() > minNumberMatchesAllowed;
}
