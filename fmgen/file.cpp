//	$fmgen-Id: file.cpp,v 1.6 1999/12/28 11:14:05 cisc Exp $

#include "common.h"

#include "headers.h"
#include "file.h"

// ---------------------------------------------------------------------------
//	����/����
// ---------------------------------------------------------------------------

FileIO::FileIO()
{
	flags = 0;
}

FileIO::FileIO(const char* filename, uint flg)
{
	flags = 0;
	Open(filename, flg);
}

FileIO::~FileIO()
{
	Close();
}

// ---------------------------------------------------------------------------
//	�ե�����򳫤�
// ---------------------------------------------------------------------------

bool FileIO::Open(const char* filename, uint flg)
{
	Close();

	strncpy(path, filename, MAX_PATH);

	DWORD access = (flg & readonly ? 0 : GENERIC_WRITE) | GENERIC_READ;
	DWORD share = (flg & readonly) ? FILE_SHARE_READ : 0;
	DWORD creation = flg & create ? CREATE_ALWAYS : OPEN_EXISTING;

	hfile = CreateFile(filename, access, share, 0, creation, 0, 0);
	
	flags = (flg & readonly) | (hfile == INVALID_HANDLE_VALUE ? 0 : open);
	if (!(flags & open))
	{
		switch (GetLastError())
		{
		case ERROR_FILE_NOT_FOUND:		error = file_not_found; break;
		case ERROR_SHARING_VIOLATION:	error = sharing_violation; break;
		default: error = unknown; break;
		}
	}
	SetLogicalOrigin(0);

	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	�ե����뤬�ʤ����Ϻ���
// ---------------------------------------------------------------------------

bool FileIO::CreateNew(const char* filename)
{
	Close();

	strncpy(path, filename, MAX_PATH);

	DWORD access = GENERIC_WRITE | GENERIC_READ;
	DWORD share = 0;
	DWORD creation = CREATE_NEW;

	hfile = CreateFile(filename, access, share, 0, creation, 0, 0);
	
	flags = (hfile == INVALID_HANDLE_VALUE ? 0 : open);
	SetLogicalOrigin(0);

	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	�ե��������ľ��
// ---------------------------------------------------------------------------

bool FileIO::Reopen(uint flg)
{
	if (!(flags & open)) return false;
	if ((flags & readonly) && (flg & create)) return false;

	if (flags & readonly) flg |= readonly;

	Close();

	DWORD access = (flg & readonly ? 0 : GENERIC_WRITE) | GENERIC_READ;
	DWORD share = flg & readonly ? FILE_SHARE_READ : 0;
	DWORD creation = flg & create ? CREATE_ALWAYS : OPEN_EXISTING;

	hfile = CreateFile(path, access, share, 0, creation, 0, 0);
	
	flags = (flg & readonly) | (hfile == INVALID_HANDLE_VALUE ? 0 : open);
	SetLogicalOrigin(0);

	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	�ե�������Ĥ���
// ---------------------------------------------------------------------------

void FileIO::Close()
{
	if (GetFlags() & open)
	{
		CloseHandle(hfile);
		flags = 0;
	}
}

// ---------------------------------------------------------------------------
//	�ե�����̤��ɤ߽Ф�
// ---------------------------------------------------------------------------

int32 FileIO::Read(void* dest, int32 size)
{
	if (!(GetFlags() & open))
		return -1;
	
	DWORD readsize;
	if (!ReadFile(hfile, dest, size, &readsize, 0))
		return -1;
	return readsize;
}

// ---------------------------------------------------------------------------
//	�ե�����ؤν񤭽Ф�
// ---------------------------------------------------------------------------

int32 FileIO::Write(const void* dest, int32 size)
{
	if (!(GetFlags() & open) || (GetFlags() & readonly))
		return -1;
	
	DWORD writtensize;
	if (!WriteFile(hfile, dest, size, &writtensize, 0))
		return -1;
	return writtensize;
}

// ---------------------------------------------------------------------------
//	�ե�����򥷡���
// ---------------------------------------------------------------------------

bool FileIO::Seek(int32 pos, SeekMethod method)
{
	if (!(GetFlags() & open))
		return false;
	
	DWORD wmethod;
	switch (method)
	{
	case begin:	
		wmethod = FILE_BEGIN; pos += lorigin; 
		break;
	case current:	
		wmethod = FILE_CURRENT; 
		break;
	case end:		
		wmethod = FILE_END; 
		break;
	default:
		return false;
	}

	return 0xffffffff != SetFilePointer(hfile, pos, 0, wmethod);
}

// ---------------------------------------------------------------------------
//	�ե�����ΰ��֤�����
// ---------------------------------------------------------------------------

int32 FileIO::Tellp()
{
	if (!(GetFlags() & open))
		return 0;

	return SetFilePointer(hfile, 0, 0, FILE_CURRENT) - lorigin;
}

// ---------------------------------------------------------------------------
//	���ߤΰ��֤�ե�����ν�ü�Ȥ���
// ---------------------------------------------------------------------------

bool FileIO::SetEndOfFile()
{
	if (!(GetFlags() & open))
		return false;
	return ::SetEndOfFile(hfile) != 0;
}
