#include "DrivingControl.h"

// ��Q�������o���Ă��̗L����Ԃ�(�ߋ��̈�Y)
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

		ideal_x = (float)(l * cos(radian + (double)urgpos[3]));
		ideal_y = (float)(l * sin(radian + (double)urgpos[3]));

		// �E�Z���T�̗̈攻��
		if (urgpos[2] < 0)
		{
			if (ideal_x < 1000.0 && ideal_y < 200.0 && ideal_y > -500.0)
				//if (ideal_x < 500.0)
			{
				count += 1;
			}
		}
		// ���Z���T�̗̈攻��
		else if (urgpos[2] > 0)
		{
			if (ideal_x < 1000.0 && ideal_y < 500.0 && ideal_y > -200.0)
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

// ��Q�������o���Ă��̌��ʂ�z��ŕԂ�
void urg_driving::getObstacleData(float*& data_x, float*& data_y)
{
	this->urg_unko::getData4URG(0, 0, 0);

	data_x = new float[data_n];
	data_y = new float[data_n];
	int datacount = 1;

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

		ideal_x = (float)(l * cos(radian + (double)urgpos[3]));
		ideal_y = (float)(l * sin(radian + (double)urgpos[3]));

		// �E�Z���T�̗̈攻��
		if (urgpos[2] < 0)
		{
			if (ideal_x < 1000.0 && ideal_y < 200.0 && ideal_y > -500.0)
				//if (ideal_x < 500.0)
			{
				data_x[datacount] = ideal_x;
				data_y[datacount] = ideal_y;
				datacount++;
			}
		}
		// ���Z���T�̗̈攻��
		else if (urgpos[2] > 0)
		{
			if (ideal_x < 1000.0 && ideal_y < 500.0 && ideal_y > -200.0)
			{
				data_x[datacount] = ideal_x;
				data_y[datacount] = ideal_y;
				datacount++;
			}
		}
	}

	data_x[0] = datacount - 1;
	data_y[0] = datacount - 1;
}