#include "DrivingControl.h"

/*
* �R���X�g���N�^
* �o�H�t�@�C����ǂݍ���Ńw�b�_���Ƃ΂�
*/
DrivingFollowPath::DrivingFollowPath(string fname, double coefficientL, double coefficientR, int ecdrCOM, int ctrlrCOM)
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
	begin = str.find(searchWord);
	if (begin != string::npos){
		x_str = str.substr(0, begin);
		x_next = stoi(x_str);
	}

	//x�̒l�̌�납��","�܂ł̕������int�^�Ŏ擾
	end = str.find(searchWord, begin + 1);
	if (end != string::npos){
		y_str = str.substr(begin + 1, end - begin + 1);
		y_next = stoi(y_str);
	}

	begin = end;
	end = str.find(searchWord, begin + 1);
	if (end != string::npos){
		data_str = str.substr(begin + 1, end - begin + 1);
		doMatching = stoi(data_str);
	}

	cout << "X:" << x_next << ",Y:" << y_next << ",Matching" << doMatching << endl;

	return true;
}
// �G���R�[�_�̃J�E���g�����擾
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

// �����ƃG���R�[�_�̃J�E���g�����w�肵�ăR�}���h�𑗐M
void DrivingFollowPath::sendDrivingCommand_count(Direction direction, int count)
{
	if (count < 0) count *= -1;

	switch (direction)
	{
	case STOP:
		sendDrivingCommand(STOP);
		break;

	case FORWARD:
		sendDrivingCommand(FORWARD, count / 9.0 * 1000);
		break;

	case BACKWARD:
		sendDrivingCommand(BACKWARD, count / 3.125 * 1000);
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
// �����Ǝ��Ԃ��w�肵�ăR�}���h�𑗐M
void DrivingFollowPath::sendDrivingCommand(Direction direction, int delay_int)
{
	int mode = 1;

	preDirection = nowDirection;
	nowDirection = direction;

	// delay_int��99999�ȏ���w��ł��Ȃ��̂�
	if (overdelayCount)
	{
		delay_int = 90000;
		overdelayCount--;
	}
	else if (delay_int > 99999)
	{
		overdelayCount = delay_int / 90000;
		delay_int = delay_int % 90000;
	}

	// STOP�̎��͑ҋ@���Ԃ�ۑ����Ȃ�
	if (direction != STOP) waittime = delay_int;

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
	double vector1_x, vector1_y;
	double vector2_x, vector2_y;
	
	vector1_x = cos(orientation);
	vector1_y = sin(orientation);

	vector2_x = x_next - x_now;
	vector2_y = y_next - y_now;

	double absVec2 = pow(vector2_x*vector2_x + vector2_y*vector2_y, 0.5);
	vector2_x = vector2_x / absVec2;
	vector2_y = vector2_y / absVec2;

	// a�~b��|a|,|b|���Z�o����arcsin�ŉ�]�p���Z�o
	double det = vector1_x * vector2_y - vector1_y * vector2_x;
	radian = asin(det);
	double inner = vector1_x * vector2_x + vector1_y * vector2_y;
	radian = atan2(det, inner);

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

	aimCount_L = 50 * distance / (dDISTANCE * leftCoefficient); // Left
	aimCount_R = 50 * distance / (dDISTANCE * rightCoefficient); // Right

	//cout << "L:" << aimCount_L << ",R:" << aimCount_R << endl;

}
// �G���R�[�_�̃J�E���g�����Q�Ƃ��Ĉړ�������҂�(FB����̈�Y)
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
// �ً}��~��̋쓮�ĊJ�w��
void DrivingFollowPath::restart(int time, Timer& timer, int encoderLRtmp[])
{
	// �G���R�[�_�̒l����~�O����ω����Ă�����D�D�D�Ƃ肠�����ω����ĂȂ��������Ƃɂ���
	//getEncoderCount();
	//int dLeft = leftCount - encoderLRtmp[0];
	//int dRight = rightCount - encoderLRtmp[1];
	//aimCount_L += dLeft;
	//aimCount_R += dRight;
	leftCount = encoderLRtmp[0];
	rightCount = encoderLRtmp[1];

	if (nowDirection != STOP) sendDrivingCommand(nowDirection, waittime - time);
	else sendDrivingCommand(preDirection, waittime - time);
	timer.getLapTime();
}
// ����~���Ă��邩�ǂ����ƁC����ׂ����ǂ����̃`�F�b�N
void DrivingFollowPath::checkEmergencyStop(Timer& timer)
{
	bool left = false;
	bool right = false;

	int encoderLRtmp[2] = { leftCount, rightCount };

	int time = timer.getLapTime(1, Timer::millisec, false) - 1000;
	if (time < 0) time = 0;
	/*
	cout << time << "millisec" << endl;
	cout << time * abs(aimCount_L) << "," << abs(leftCount) * waittime << endl;
	cout << leftCount << "," << rightCount << endl;
	cout << aimCount_L << "," << aimCount_R << endl;
	cout << waittime << endl;
	*/


	if (((float)time + 1000) / (float)waittime * 100 > 98) return;

	if (time * abs(aimCount_L) > abs(leftCount) * waittime)     left = true;
	if (time * abs(aimCount_R) > abs(rightCount) * waittime)    right = true;

	if (left && right&& nowDirection != STOP)
	{
		cout << "����~���Ă邩��" << endl;
		DrivingControl::sendDrivingCommand(1, 0, 0, 0);
		if (!retLastRead){
			if (MessageBoxA(NULL, "���������Ĕ���~���Ă�H�H\n�����Ă������H�H", "���������āI", MB_YESNO | MB_ICONSTOP) == IDYES)
				restart(time, timer,encoderLRtmp);
		}
	}/*
	if (mUrgd.checkObstacle())
	{
		if (nowDirection != STOP) sendDrivingCommand(STOP);

		while (mUrgd.checkObstacle());
		restart(time, timer, encoderLRtmp);
	}*/
}
// �ړ������܂őҋ@����
void DrivingFollowPath::waitDriveComplete_FF()
{
	cout << "Wait time [millisec]:" << waittime << endl;

	Timer waitDriveTimer;
	waitDriveTimer.Start();

	while (waitDriveTimer.getLapTime(1, Timer::millisec, false) < waittime)
	{
		getEncoderCount();
		checkEmergencyStop(waitDriveTimer);
	}

	leftCount = 0;
	rightCount = 0;
}

// FF�ŋ쓮���J�n����
void DrivingFollowPath::run_FF()
{
	getEncoderCount();

	char z = getchar();

	//mUrgd.getAroundImage();

	while (getNextPoint())
	{
		cout << "��]" << endl;
		calcRotationAngle();
		do{
			if (aimCount_L > 0) sendDrivingCommand_count(RIGHT, aimCount_L);
			else sendDrivingCommand_count(LEFT, aimCount_L);
			waitDriveComplete_FF();
		} while (overdelayCount);
		Sleep(500);

		cout << "���i" << endl;
		calcMovingDistance();
		do{
			if (aimCount_L > 0) sendDrivingCommand_count(FORWARD, aimCount_L);
			else sendDrivingCommand_count(BACKWARD, aimCount_L);
			waitDriveComplete_FF();
		} while (overdelayCount);
		Sleep(500);

		if(doMatching)	mUrgd.tMatching(x_next, y_next, orientation);
	}
}
// FB�ŋ쓮���J�n����(�ߋ��̈�Y)
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
// URG�̃p�����[�^���Z�b�g����
void DrivingFollowPath::setURGParam(int URG_COM[], float URGPOS[][4], int NumOfURG)
{
	mUrgd.setURGParam(URG_COM, URGPOS, NumOfURG);
}
void DrivingFollowPath::readMapImage(string mapName)
{
	mUrgd.readMapImage(mapName);
}
