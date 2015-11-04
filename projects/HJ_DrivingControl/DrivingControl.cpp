/*
 *	�o�H���̃e�L�X�g�t�@�C��(�g���q.rt)���ォ�珇�ɓǂݍ���ňړ������C��]���v�Z���C
 *	�V���A���ʐM�ŋ쓮�w�߂𑗐M����D
 */
#include "DrivingControl.h"

/*
*	�T�v:
*		Arduino�ƃV���A���ʐM���s�����߂̃n���h�����擾����
*	�����F
*		int arduinoCOM	COM�|�[�g�ԍ�
*		HANDLE&	hComm	�n���h���ϐ��ւ̎Q��
*		int timeoutmillisec	�ʐM�̃^�C���A�E�g�܂ł̎���
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
// �R���g���[���ւ̃V���A���|�[�g�̃n���h�����擾
void DrivingControl::setControllerCOM(int ctrlerCOM)
{
	this->controllerCOM = ctrlerCOM;
	getArduinoHandle(controllerCOM, hControllerComm, 500);
}
// �K��̃t�H�[�}�b�g�ɏ]���ăR���g���[���ɃR�}���h�𑗐M
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