#ifndef _INC_DRIVING_CONTROL_
#define _INC_DRIVING_CONTROL_

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Windows.h>

#include "../Timer/Timer.h"
#include "../SharedMemoryTESTpp/SharedMemory.h"
#include "../cvPCDtest/urg_unko.h"

using namespace std;

void getArduinoHandle(int arduinoCOM, HANDLE& hComm, int timeoutmillisec);

class DrivingControl
{
protected:
    // �R���g���[���n
	int		controllerCOM;
	HANDLE	hControllerComm;

    // �V���A������M�̌���
	bool retLastSend;
	bool retLastRead;

public:
    // COM�|�[�g��ݒ肵�ăn���h�����擾
	void setControllerCOM(int ctrlerCOM);

	// Arduino�֋쓮�w�߂𑗐M
	void sendDrivingCommand(int mode_int, int forward_int, int  crosswise_int, int delay_int);
};

class urg_driving
	: public urg_mapping
{
private:
    
public:
	enum ObstacleEmergency { NONE, DETECT };

	ObstacleEmergency checkObstacle();
};

class DrivingFollowPath
	: DrivingControl
{
private:
	// �o�H�f�[�^�ǂݎ��p�ϐ�
	const string	fileName;			// �o�H�t�@�C����
	const string	searchWord = ",";	// �f�[�^�̋�؂莯�ʗp��","
	ifstream	ifs;					// �t�@�C���X�g���[��
	string	str, x_str, y_str;		// �f�[�^�ǂݎ��Ɏg�p����ϐ�
	string::size_type	x_pos, y_pos;

	// �x�N�g����2�g�p�����3�_�ۑ�����
	int	x_old, y_old;					// 1�O�̍��W
	int x_now, y_now;					// ���݂̍��W
	int	x_next = 0, y_next = 0;			// ���̍��W
    
	// �쓮�w�ߌv�Z�p�ϐ�
	double	orientation;			// ���݌����Ă�����ʊp(�X�^�[�g�����0�Ƃ��ĉE��������)
	double	radian;					// �v�Z��̉�]�p
	double	distance;				// �v�Z��̈ړ�����

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

	// Arduino�ւ̋쓮�w�ߊ֘A
	enum Direction	{ STOP, FORWARD, BACKWARD, RIGHT, LEFT };
	int		aimCount_L, aimCount_R;

	// �G���R�[�_����J�E���g�����擾���ĐώZ����
	void getEncoderCount();
	// Arduino�֋쓮�w�߂𑗐M
	void sendDrivingCommand(Direction direction, int delay_int = 99999);
	void sendDrivingCommand_count(Direction direction, int count);
	// �w�߂����쓮�̊�����ҋ@
	void waitDriveComplete();
	void waitDriveComplete_FF();

	int waittime;
	Direction nowDirection;

	void checkEmergencyStop(Timer& timer);
	void restart(int time, Timer& timer);
	bool isObstacle = false;

	urg_driving* urgdArray;

public:
	// �������̏���������
	DrivingFollowPath(string fname, double coefficientL, double coefficientR, int arduioCOM, int ctrlrCOM);

	void setURGParam(int URG_COM[], float URGPOS[][4], int NumOfURG);

	// ���̓_��ǂݍ���
	bool getNextPoint();

	// ��]�p���v�Z
	void	calcRotationAngle();
	// �������v�Z
	void	calcMovingDistance();

	void	run();
	void	run_FF();
};


#endif