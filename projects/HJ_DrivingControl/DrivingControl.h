#ifndef _INC_DRIVING_CONTROL_
#define _INC_DRIVING_CONTROL_

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <stdarg.h>

#include <ypspur.h>

#include "../Timer/Timer.h"
#include "../cvPCDtest/urg_unko.h"
#include "../HJ_ReceiveAndroidSensors/receiveAndroidSensors.h"

using namespace std;

#define PI 3.14159265359

// ���i���Ɋp�x�␳����臒l
const int angleThresh = 10;

void getArduinoHandle(int arduinoCOM, HANDLE& hComm, int timeoutmillisec);

class Ypspur
{
protected:
	const float WHEEL_RADIUS;
	const float MAX_VEL;
	const float MAX_ACC;

public:
	Ypspur(float wheel_rad, float max_vel, float max_acc);




};

// �쓮�w�߂̊�{�N���X
class DrivingControl
{
protected:
    // �R���g���[���n
	int		controllerCOM;
	HANDLE	hControllerComm;

    // �V���A������M�̌���
	bool retLastSend;
	bool retLastRead;
	unsigned long lastReadBytes;

public:
    // COM�|�[�g��ݒ肵�ăn���h�����擾
	void setControllerCOM(int ctrlerCOM);

	// Arduino�֋쓮�w�߂𑗐M
	void sendDrivingCommand(int mode_int, int forward_int, int  crosswise_int, int delay_int);
};

// URG�ŋ쓮���̏�Q�����o���s���N���X
class urg_driving
	: public urg_mapping
{
public:
	enum ObstacleEmergency { NONE, DETECT, DETECT_LEFT, DETECT_RIGHT, SLOW1, SLOW2 };
	ObstacleEmergency checkObstacle();
	void getObstacleData(float*& data_x , float*& data_y);
};

// ���E��URG���܂Ƃ߂ĊǗ�����N���X
class Manage2URG_Drive
{
private:
	urg_driving urgdArray[2];
	vector<cv::Mat> tmMap;
	cv::Mat tmTemplate;
public:
	~Manage2URG_Drive();

	void setURGParam(int URG_COM[], float URGPOS[][4], int NumOfURG);
	void readMapImage(string mapName);
	
    urg_driving::ObstacleEmergency checkObstacle();

	void getAroundImage(int width = 300 , int height = 300 , int resolution = 5 ,int measurementTimes = 10);

	void tMatching(int& pos_x , int& pos_y , double& angle , int mapNum);
};

// �o�H�f�[�^��ǂݍ���ŋ쓮�w�߂��s�����
class DrivingFollowPath
{
private:
	// �o�H�f�[�^�ǂݎ��p�ϐ�
	const string	fileName;			// �o�H�t�@�C����
	const string	searchWord = ",";	// �f�[�^�̋�؂莯�ʗp��","
	ifstream	ifs;					// �t�@�C���X�g���[��
	string	str, x_str, y_str , match_str , map_str;		// �f�[�^�ǂݎ��Ɏg�p����ϐ�
	string::size_type	begin, end;

	// �x�N�g����2�g�p�����3�_�ۑ�����
	int	x_old, y_old;					// 1�O�̍��W
	int x_now, y_now;					// ���݂̍��W
	int	x_next = 0, y_next = 0;			// ���̍��W

	int origin_y;

	int doMatching;
	int mapNum;
    
	// �쓮�w�ߌv�Z�p�ϐ�
	double	orientation;			// ���݌����Ă�����ʊp(�X�^�[�g�����0�Ƃ��ĉE��������)
	//double	radian;					// �v�Z��̉�]�p
	//double	distance;				// �v�Z��̈ړ�����

	const int wheelDistance = 530 / 2;	// �^�C���ԋ����̔���[mm]
	//const double dDISTANCE = 24.87094184; // 1�J�E���g������̋���[mm](�^�C���a��72����)
	const double dDISTANCE = 1; // 1�J�E���g������̋���[mm](�^�C���a��72����)

	const double leftCoefficient;
	const double rightCoefficient;

	// �G���R�[�_�̒l�֘A
	int		encoderCOM;
	HANDLE	hEncoderComm;
	bool	isEncoderInitialized = false;
	int		leftCount, rightCount;
	long	totalLeftCount, totalRightCount;
	int		emergencyCount;

	// Arduino�ւ̋쓮�w�ߊ֘A
	enum Direction	{ STOP, FORWARD , FORWARD_SLOW , BACKWARD, RIGHT, LEFT };
	double		aimCount_L, aimCount_R;

	// Android�֘A
	float defaultOrientation[3];
	float nowOrientation[3];
	float dAzimuth; // ���ʊp�̕ψ�

	// �G���R�[�_����J�E���g�����擾���ĐώZ����
	void getEncoderCount();
	// Arduino�֋쓮�w�߂𑗐M
	void sendDrivingCommand(Direction direction, int delay_int = 99999);
	void sendDrivingCommand_count(Direction direction, int count);
	// �w�߂����쓮�̊�����ҋ@
	void waitDriveComplete();
	void waitDriveComplete_FF();

	int waittime;
	int overdelayCount;
	Direction preDirection;
	Direction nowDirection;
	bool encoderOutlier = false;

	void checkEmergencyStop(Timer& timer);
	void restart(int time, Timer& timer);

	Manage2URG_Drive mUrgd;
	rcvAndroidSensors rcvDroid;

public:
	// �������̏���������
	DrivingFollowPath(string fname, double coefficientL, double coefficientR, int arduioCOM);

	// Manage2URG_Drive�N���X�֘A
	void setURGParam(int URG_COM[], float URGPOS[][4], int NumOfURG);
	void readMapImage(string mapName);
	void readMapImage(int num, ...);

	// rcvAndroidSensors�N���X�֘A
	void setAndroidCOM(int comport);

	// ���̓_��ǂݍ���
	bool getNextPoint();

	// ��]�p���v�Z
	double	calcRotationAngle( int nowCoord_x = -99999 , int nowCoord_y = -99999 );
	void	sendRotation(double radian);
	// �������v�Z
	double	calcMovingDistance(int nowCoord_x = -99999, int nowCoord_y = -99999);
	void	sendStraight(double distance);
	// ���ݒn���Z�o
	void	calcNowCoord(int time, int nowCoord[2]);
	void	calcNowCoord(int time);

	// ���݂̎p��(���ʊp)�̊�l����̃Y�����Z�o
	void	checkCurrentAzimuth();

	void	run();
	void	run_FF();
};


#endif