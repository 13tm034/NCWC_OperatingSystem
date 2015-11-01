/*
 *	�o�H���̃e�L�X�g�t�@�C��(�g���q.rt)���ォ�珇�ɓǂݍ���ňړ������C��]���v�Z���C
 *	�V���A���ʐM�ŋ쓮�w�߂𑗐M����D
 */
#include "DrivingControl.h"

#define PI 3.14159265359

/*
*	�T�v:
*		Arduino�ƃV���A���ʐM���s�����߂̃n���h�����擾����
*	�����F
*		HANDLE&	hComm	�n���h���ϐ��ւ̎Q��
*	�Ԃ�l:
*		�Ȃ�
*/
void getArduinoHandle(int arduinoCOM, HANDLE& hComm , int timeoutmillisec )
{
	//�V���A���|�[�g���J���ăn���h�����擾
	string com = "\\\\.\\COM" + to_string(arduinoCOM);
	hComm = CreateFile(com.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hComm == INVALID_HANDLE_VALUE){
		printf("�V���A���|�[�g���J�����Ƃ��ł��܂���ł����B");
		char z;
		z = getchar();
		return;
	}
	//�|�[�g���J���Ă���ΒʐM�ݒ���s��
	else
	{
		DCB lpTest;
		GetCommState(hComm, &lpTest);
		lpTest.BaudRate = 9600;
		lpTest.ByteSize = 8;
		lpTest.Parity = NOPARITY;
		lpTest.StopBits = ONESTOPBIT;
		SetCommState(hComm, &lpTest);

		COMMTIMEOUTS lpTimeout;
		GetCommTimeouts(hComm, &lpTimeout);
		lpTimeout.ReadIntervalTimeout = timeoutmillisec;
		lpTimeout.ReadTotalTimeoutMultiplier = 0;
		lpTimeout.ReadTotalTimeoutConstant = timeoutmillisec;
		SetCommTimeouts(hComm, &lpTimeout);

	}
}
void DrivingControl::setControllerCOM(int ctrlerCOM)
{
	this->controllerCOM = ctrlerCOM;
	getArduinoHandle(controllerCOM, hControllerComm, 500);
}

void DrivingControl::sendDrivingCommand(int mode_int, int forward_int, int  crosswise_int, int delay_int)
{
	unsigned char	sendbuf[18];
	unsigned char	receive_data[18];
	unsigned long	len;

	unsigned char	mode;
	unsigned char	sign1, sign2;
	ostringstream	forward_sout, crosswise_sout, delay_sout;
	string			forward_str, crosswise_str, delay_str;

	mode = to_string(mode_int)[0];

	if (forward_int < 0)
	{
		forward_int *= -1;
		sign1 = '1';
	}
	else sign1 = '0';

	forward_sout << setfill('0') << setw(4) << forward_int;
	forward_str = forward_sout.str();

	if (crosswise_int < 0)
	{
		crosswise_int *= -1;
		sign2 = '1';
	}
	else sign2 = '0';

	crosswise_sout << setfill('0') << setw(4) << crosswise_int;
	crosswise_str = crosswise_sout.str();

	delay_sout << setfill('0') << setw(5) << delay_int;
	delay_str = delay_sout.str();

	// �o�b�t�@�N���A
	memset(sendbuf, 0x00, sizeof(sendbuf));

	sendbuf[0] = 'j';
	sendbuf[1] = mode;
	sendbuf[2] = sign1;
	for (int i = 3; i < 7; i++)	sendbuf[i] = forward_str[i - 3];
	sendbuf[7] = sign2;
	for (int i = 8; i < 12; i++) sendbuf[i] = crosswise_str[i - 8];
	for (int i = 12; i < 17; i++) sendbuf[i] = delay_str[i - 12];
	sendbuf[17] = 'x';

	// �ʐM�o�b�t�@�N���A
	PurgeComm(hControllerComm, PURGE_RXCLEAR);
	// ���M
	retLastSend = WriteFile(hControllerComm, &sendbuf, sizeof(sendbuf), &len, NULL);

	cout << "send:";
	for (int i = 0; i < len; i++)
	{
		cout << sendbuf[i];
	}
	cout << endl;

	// �o�b�t�@�N���A
	memset(receive_data, 0x00, sizeof(receive_data));
	// �ʐM�o�b�t�@�N���A
	PurgeComm(hControllerComm, PURGE_RXCLEAR);
	len = 0;
	// Arduino����f�[�^����M
	retLastRead = ReadFile(hControllerComm, &receive_data, sizeof(receive_data), &len, NULL);

	cout << "receive:";
	for (int i = 0; i < len; i++)
	{
		cout << receive_data[i];
	}
	cout << endl;

	cout << retLastRead << endl;
}


/*
 * �R���X�g���N�^
 * �o�H�t�@�C����ǂݍ���Ńw�b�_���Ƃ΂�
 */
DrivingFollowPath::DrivingFollowPath(string fname, double coefficientL, double coefficientR, int ecdrCOM , int ctrlrCOM)
	: fileName(fname), leftCoefficient(coefficientL), rightCoefficient(coefficientR), encoderCOM(ecdrCOM)
{
	controllerCOM = ctrlrCOM;

	// �o�H�f�[�^��ǂݍ���
	ifs.open(fileName);
	if (ifs.fail())
	{
		cerr << "False" << endl;
		return;
	}
	// �w�b�_�������Ƃ΂�
	getline(ifs, str);

	// ���_���擾���Ă���
	getNextPoint();

	// ���߂̉�]�p�v�Z�p�Ƃ��Č��_�̏�������ɓ_��ǉ�
	x_now = x_next - 5;
	y_now = y_next;

	getArduinoHandle(encoderCOM, hEncoderComm, 0);
	setControllerCOM(controllerCOM);

}

/*
 * ���̓_��ǂݍ���
 */
bool DrivingFollowPath::getNextPoint()
{
	// �Â����W��ۑ�
	x_old = x_now;
	y_old = y_now;
	x_now = x_next;
	y_now = y_next;

	// ���̍s�����݂��Ȃ����false��Ԃ�
	if (!getline(ifs, str)) return false;

	//�擪����","�܂ł̕������int�^�Ŏ擾
	x_pos = str.find(searchWord);
	if (x_pos != string::npos){
		x_str = str.substr(0, x_pos);
		x_next = stoi(x_str);
	}

	//x�̒l�̌�납��","�܂ł̕������int�^�Ŏ擾
	y_pos = str.find(searchWord, x_pos + 1);
	if (y_pos != string::npos){
		y_str = str.substr(x_pos + 1, y_pos);
		y_next = stoi(y_str);
	}
	cout << x_next << "," << y_next << endl;

	return true;
}

void DrivingFollowPath::getEncoderCount()
{
	unsigned char	sendbuf[1];
	unsigned char	receive_data[2];
	unsigned long	len;

	// �o�b�t�@�N���A
	memset(sendbuf, 0x01, sizeof(sendbuf));
	// �ʐM�o�b�t�@�N���A
	PurgeComm(hEncoderComm, PURGE_RXCLEAR);
	// ���M
	WriteFile(hEncoderComm, &sendbuf, 1, &len, NULL);
	// �o�b�t�@�N���A
	memset(receive_data, 0x00, sizeof(receive_data));
	// �ʐM�o�b�t�@�N���A
	PurgeComm(hEncoderComm, PURGE_RXCLEAR);
	// Arduino����f�[�^����M
	ReadFile(hEncoderComm, &receive_data, 2, &len, NULL);
	

	//����������Ă��Ȃ���Ώ�����(���߂̃f�[�^���̂Ă�)
	if (!isEncoderInitialized)
	{
		isEncoderInitialized = true;
		return;
	}

	leftCount += (signed char)receive_data[0];
	rightCount += (signed char)receive_data[1];
	//cout << "L:" << leftCount << ",R:" << rightCount << endl;
}

void DrivingFollowPath::sendDrivingCommand_count( Direction direction , int count)
{
	if ( count < 0) count *= -1;

	switch (direction)
	{
	case STOP:
		sendDrivingCommand(STOP);
		break;

	case FORWARD:
		sendDrivingCommand(FORWARD, count / 9.0 * 1000);
		break;

	case BACKWARD:
		sendDrivingCommand(BACKWARD,count / 3.125 * 1000);
		break;

	case RIGHT:
		sendDrivingCommand(RIGHT, count / 10.875 * 1000);
		break;

	case LEFT:
		sendDrivingCommand(LEFT, count / 10.25 * 1000);
		break;

	default:
		break;
	}
}

void DrivingFollowPath::sendDrivingCommand(Direction direction, int delay_int)
{
	int mode = 1;

	if (direction != STOP) nowDirection = direction;
	waittime = delay_int;

	switch (direction)
	{
	case STOP:
		mode = 0;
		DrivingControl::sendDrivingCommand(mode, 0, 0, delay_int);
		break;

	case FORWARD:
		DrivingControl::sendDrivingCommand(mode, -1000, 405, delay_int);
		break;

	case BACKWARD:
		DrivingControl::sendDrivingCommand(mode, 600, 509, delay_int);
		break;

	case RIGHT:
		DrivingControl::sendDrivingCommand(mode, -380, -1500, delay_int);
		break;

	case LEFT:
		DrivingControl::sendDrivingCommand(mode, 0, 1500, delay_int);
		break;

	default:
		break;
	}
}



// �O�ςŉ�]�p���v�Z
void	DrivingFollowPath::calcRotationAngle()
{
	// 3�_����x�N�g����2�p��
	int vector1_x, vector1_y;
	int vector2_x, vector2_y;

	vector1_x = x_now - x_old;
	vector1_y = y_now - y_old;

	vector2_x = x_next - x_now;
	vector2_y = y_next - y_now;

	// a�~b��|a|,|b|���Z�o����arcsin�ŉ�]�p���Z�o
	int det = vector1_x * vector2_y - vector1_y * vector2_x;
	double d1 = pow((double)(vector1_x*vector1_x + vector1_y*vector1_y),0.5);
	double d2 = pow((double)(vector2_x*vector2_x + vector2_y*vector2_y),0.5);
	radian = asin((double)det / (d1*d2));

	int inner = vector1_x * vector1_y + vector2_y * vector2_x;
	//radian = atan2(det, inner);

	orientation += radian;

	cout << "rad:" << radian << ", deg:" << radian / PI * 180 << endl;
	cout << "rad:" << orientation << ", deg:" << orientation / PI * 180 << endl;

	aimCount_L = (wheelDistance * radian) / (dDISTANCE * leftCoefficient); // Left
	aimCount_R = -(wheelDistance * radian) / (dDISTANCE * rightCoefficient); // Right
	if (aimCount_L) aimCount_L += abs(aimCount_L) / aimCount_L;
	if (aimCount_R) aimCount_R += abs(aimCount_R) / aimCount_R;

	//cout << "L:" << aimCount_L << ",R:" << aimCount_R << endl;

}

// �������v�Z
void	DrivingFollowPath::calcMovingDistance()
{
	double	x_disp = x_next - x_now;
	double	y_disp = y_next - y_now;

	distance = sqrt(x_disp*x_disp + y_disp*y_disp);

	cout << "distance[m]:" << distance * 0.05 << endl;

	aimCount_L = 5*distance / (dDISTANCE * leftCoefficient); // Left
	aimCount_R = 5*distance / (dDISTANCE * rightCoefficient); // Right

	//cout << "L:" << aimCount_L << ",R:" << aimCount_R << endl;

}

void DrivingFollowPath::waitDriveComplete()
{
	while (abs(leftCount) < abs(aimCount_L) && abs(rightCount) < abs(aimCount_R))
	{
		getEncoderCount();
	}
	leftCount = 0;
	rightCount = 0;
	sendDrivingCommand(STOP);
}

void DrivingFollowPath::restart(int time , Timer& timer )
{
	sendDrivingCommand(nowDirection, waittime - time);
	//Sleep(1000);
	timer.getLapTime();
}

void DrivingFollowPath::checkEmergencyStop(Timer& timer)
{
	bool left = false;
	bool right = false;

	int time = timer.getLapTime(1, Timer::millisec, false) - 1000;
	if (time < 0) time = 0;
	/*
	cout << time << "millisec" << endl;
	cout << time * abs(aimCount_L) << "," << abs(leftCount) * waittime << endl;
	cout << leftCount << "," << rightCount << endl;
	cout << aimCount_L << "," << aimCount_R << endl;
	cout << waittime << endl;
    */
	

	if (((float)time + 1000) / (float)waittime * 100 > 98 ) return;

	if (time * abs(aimCount_L) > abs(leftCount) * waittime)     left = true;
	if (time * abs(aimCount_R) > abs(rightCount) * waittime)    right = true;

	if (left && right)
	{
		cout << "����~���Ă邩��" << endl;
		DrivingControl::sendDrivingCommand(1, 0, 0, 0);
		if (retLastRead){
			if (MessageBoxA(NULL, "���������Ĕ���~���Ă�H�H\n�����Ă������H�H", "���������āI", MB_YESNO | MB_ICONSTOP) == IDYES)
                restart(time, timer);
		}
	}
	if (urgdArray[0].checkObstacle() || urgdArray[1].checkObstacle())
	{
		sendDrivingCommand(STOP);
		if (MessageBoxA(NULL, "�����Ă������H�H", "���������ĂȂ񂩊�Ȃ��H�H", MB_YESNO | MB_ICONSTOP) == IDYES)
            restart(time, timer);

		isObstacle = true;
	}
	else if (isObstacle)
	{
		isObstacle = false;
		restart(time, timer);
	}
}

void DrivingFollowPath::waitDriveComplete_FF()
{
	cout << "Wait time [millisec]:" << waittime << endl;

	Timer waitDriveTimer;
	//Sleep(1000);
	waitDriveTimer.Start();
	
	while (waitDriveTimer.getLapTime(1, Timer::millisec, false) < waittime)
	{
		getEncoderCount();
		checkEmergencyStop(waitDriveTimer);
	}

	leftCount = 0;
	rightCount = 0;
}

void DrivingFollowPath::run_FF()
{
	getEncoderCount();

	char z = getchar();

	while (getNextPoint())
	{
		calcRotationAngle();
		if (aimCount_L > 0) sendDrivingCommand_count(RIGHT , aimCount_L);
		else sendDrivingCommand_count(LEFT, aimCount_L);
		cout << "��]" << endl;
		waitDriveComplete_FF();
		Sleep(500);

		calcMovingDistance();
		if (aimCount_L > 0) sendDrivingCommand_count(FORWARD, aimCount_L);
		else sendDrivingCommand_count(BACKWARD, aimCount_L);
		cout << "���i" << endl;
		waitDriveComplete_FF();
		Sleep(500);
	}
}
void DrivingFollowPath::run()
{
	getEncoderCount();

	while (getNextPoint())
	{
		calcRotationAngle();
		if (aimCount_L > 0) sendDrivingCommand(RIGHT);
		else sendDrivingCommand(LEFT);
		cout << "��]" << endl;
		waitDriveComplete();
		Sleep(1000);

		calcMovingDistance();
		if (aimCount_L > 0) sendDrivingCommand(FORWARD);
		else sendDrivingCommand(BACKWARD);
		cout << "���i" << endl;
		waitDriveComplete();
		Sleep(1000);
	}
}

void DrivingFollowPath::setURGParam(int URG_COM[], float URGPOS[][4], int NumOfURG)
{
	urgdArray = new urg_driving[NumOfURG];
	for (int i = 0; i < NumOfURG; i++)
	{
		urgdArray[i].init(URG_COM[i], URGPOS[i]);
	}
}

urg_driving::ObstacleEmergency urg_driving::checkObstacle()
{
	this->urg_unko::getData4URG(0, 0, 0);

	int count = 0;
	for (int i = 0; i < data_n; ++i) {
		long l = data[i];	//�擾�����_�܂ł̋���
		double radian;
		float x, y, z;
		float ideal_x, ideal_y;

		//�ُ�l�Ȃ�Ƃ΂�
		if (!this->pointpos[0][i] && !this->pointpos[1][i])	continue;

		//�_�܂ł̊p�x���擾����xy�ɕϊ�
		//(�ɍ��W�Ŏ擾�����f�[�^���f�J���g���W�ɕϊ�)
		radian = urg_index2rad(&urg, i);
		x = (float)(l * cos(radian));
		y = (float)(l * sin(radian));
		z = urgpos[0];

		ideal_x = l * cos(radian - urgpos[3]);
		ideal_y = l * sin(radian - urgpos[3]);

		// ���Z���T�̗̈攻��
		if (urgpos[2] < 0)
		{
			if (ideal_x < 1000.0 && ideal_y < 50.0 && ideal_y > -20.0)
				//if (ideal_x < 1000.0)
			{
				count += 1;
			}
		}
		// �E�Z���T�̗̈攻��
		else if (urgpos[2] > 0)
		{
			if (ideal_x < 1000.0 && ideal_y < 20.0 && ideal_y > -50.0)
			{
				count += 1;
			}
		}
	}
	if (count > 8){
		return DETECT;
		//printf("�_�̐��@= %d\n", count);
	}
	return NONE;
}