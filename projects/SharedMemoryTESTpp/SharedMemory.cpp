#include "SharedMemory.h"

using namespace std;

template < typename T >
SharedMemory<T>::SharedMemory(const std::string FMNAME) :FILEMAPNAME(FMNAME)
{
	SharedMemory::getShMem();
}

template < typename T >
SharedMemory<T>::~SharedMemory()
{
	SharedMemory::releaseShMem();
}

template < typename T >
void SharedMemory<T>::getShMem()
{
	/*
	* CreateFileMapping�͎w�肳�ꂽ�t�@�C���ɑ΂���A
	* ���O�t���܂��͖��O�Ȃ��̃t�@�C���}�b�s���O�I�u�W�F�N�g���쐬�܂��͊J���܂��B
	*
	* HANDLE CreateFileMapping(
	* HANDLE hFile,                       // �t�@�C���̃n���h��
	* LPSECURITY_ATTRIBUTES lpAttributes, // �Z�L�����e�B
	* DWORD flProtect,                    // �ی�
	* DWORD dwMaximumSizeHigh,            // �T�C�Y��\����� DWORD
	* DWORD dwMaximumSizeLow,             // �T�C�Y��\������ DWORD
	* LPCTSTR lpName                      // �I�u�W�F�N�g��
	* );
	*
	* ref=>https://msdn.microsoft.com/ja-jp/library/Cc430039.aspx
	*/
	hShMem = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		1024,
		FILEMAPNAME);

	if (hShMem != NULL)
	{
		//bool isCreated = GetLastError() != ERROR_ALREADY_EXISTS;

		/*
		* MapViewOfFile�͌Ăяo�����v���Z�X�̃A�h���X��ԂɁA�t�@�C���̃r���[���}�b�v���܂��B
		*
		* LPVOID MapViewOfFile(
		* HANDLE hFileMappingObject,   // �t�@�C���}�b�s���O�I�u�W�F�N�g�̃n���h��
		* DWORD dwDesiredAccess,       // �A�N�Z�X���[�h
		* DWORD dwFileOffsetHigh,      // �I�t�Z�b�g�̏�� DWORD
		* DWORD dwFileOffsetLow,       // �I�t�Z�b�g�̉��� DWORD
		* SIZE_T dwNumberOfBytesToMap  // �}�b�v�Ώۂ̃o�C�g��
		* );
		*
		* ref=>https://msdn.microsoft.com/ja-jp/library/Cc430198.aspx
		*/
		datap = (T *)MapViewOfFile(
			hShMem,
			FILE_MAP_WRITE,
			0,
			0,
			0);
	}
}

template < typename T >
void SharedMemory<T>::releaseShMem()
{
	if (ghShMem != NULL)
	{
		if (datap != NULL)
			UnmapViewOfFile(datap);
		CloseHandle(hShMem);
		hShMem = NULL;
	}
}

template < typename T >
void SharedMemory<T>::setShMemData( T setData ,int offset)
{
	*(datap + offset ) = setData;
}

template < typename T >
T SharedMemory<T>::getShMemData(int offset)
{
	return *(datap + offset);
}
