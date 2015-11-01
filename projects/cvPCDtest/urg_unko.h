#ifndef _INC_URG_UNKO
#define _INC_URG_UNKO

#include "urg_sensor.h"
#include "urg_utils.h"
#include "urg_open.h"
#include "pcimage.h"
#include "../SharedMemoryTESTpp/SharedMemory.h"

#include <Windows.h>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_lib.hpp>


#define DEBUG_WRITELINE


//�ڑ�����URG�̌��������Ŕ��f����悤�ɂ����}�N��
#define getDataUNKO(aURGCOM , aURGPOS , ARDUINOCOM) getDataUNKOOrigin( (aURGCOM),(aURGPOS),(ARDUINOCOM),sizeof((aURGCOM))/sizeof(aURGCOM[0])) 

//�w�肵��COM�|�[�g�����
int CommClose(HANDLE hComm);
//Arduino�̃n���h�����擾
void getArduinoHandle(int arduinoCOM, HANDLE& hComm);
//urg_unko��main���[�v
void getDataUNKOOrigin(int URG_COM[], float URGPOS[][4], int ARDUINO_COM, int NumOfURG);

class writePCD
{
private:
	std::ofstream ofs;	//�t�@�C���X�g���[���I�u�W�F�N�g�Dpcd�t�@�C���쐬�ɗp����
	int pcdnum;			//pcd�t�@�C���̔ԍ����J�E���g����ϐ�
	int pcdcount;		//pcd�t�@�C���ɏ������ޓ_�̐����J�E���g����ϐ�

	std::string dirname;

public:
	bool isWritePCD;

	writePCD(std::string dirName = "");
	//pcd�t�@�C�����쐬���ď������ޏ������s��
	void pcdinit();
	//pcd�t�@�C���֓_����������
	void pcdWrite(float x, float y);
	void pcdWrite(float x, float y, float pos_x, float pos_y, float droidAngle[], float droidGPS[]);
	//pcd�t�@�C���ւ̏������݂��I�����ĕۑ�����
	void pcdSave();
};


/*
*
*�@URG�Ńf�[�^���擾���ă}�b�s���O���s���N���X
*
*/
class urg_unko:
	public writePCD
{
protected:
	/*
	*	�����o�ϐ�
	*/
	int COMport;	//URG��COM�|�[�g

	//float urgpos[3];	
	//NCWC�̉�]���S���猩��URG�̈ʒu�D�Z���T�̒n�ʂ���̍����C�Z���T�̊�ʒu����̋����C����ѐ����ʂ���̘�p

	float urgpos[4];	
	//NCWC�̉�]���S���猩��URG�̈ʒu�D�Z���T�̒n�ʂ���̍����C�Z���T�̊�ʒu�����x�����̋����C�������̋����C����ѐ����ʂ���̘�p
	
	urg_t urg;			//URG�I�u�W�F�N�g
	long *data = NULL;	
	long time_stamp;

	float scaninterval = 0.0;//�v�������{����Œ�Ԋu[mm]

	enum {
		CAPTURE_TIMES = 1,
	};

	float currentCoord_x = 0.0, currentCoord_y = 0.0;
	float distance = 0.0, distance_old = 0.0;
	float radian = 0.0;

	SharedMemory<int> shMem;
	enum {EMARGENCY};

	float* pointpos[2];
	int data_n;

	/***********************
	 *	private�ȃ��\�b�h  *
	 ***********************/

	//URG�Ƃ̐ڑ���ؒf
	int disconnectURG();
	//URG�Ɛڑ�
	int connectURG();

	//�擾�����f�[�^������ۂ̓񎟌������v�Z���ă}�b�v�Cpcd�t�@�C���ւ̏������݂��s��
	void calcSurface2D();

public:
	/*
	*	public�ȃ��\�b�h
	*/
	//�R���X�g���N�^
	urg_unko();
	//�f�X�g���N�^
	~urg_unko();

	//���g�̏������������s��
	void init(int COM, float pos[]);
	//URG����f�[�^���擾���郁�\�b�h
	int getData4URG(float& dist, float& old, float& rad);

	void updateCurrentCoord(float coord_x, float coord_y);
	void updateCurrentCoord(float coordXY[]);

};

class urg_mapping : 
	public urg_unko
{
private:
	static PCImage pcimage;	//�}�b�v�摜�쐬�p�N���X

public:
	void setWriteLine(bool isLine);
	std::string	getDirName();

	static void initPCImage(PCImage& pci);
	static void initPCImage(int width, int height, int resolution);
	void setPCImageColor(PCImage::BGR bgr);
};

#endif