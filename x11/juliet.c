/*	$Id: juliet.c,v 1.1.1.1 2003/04/28 18:06:56 nonaka Exp $	*/

#include	"common.h"
#include	"juliet.h"

/*
 * Juliet ���ߡ�
 */

BOOL
juliet_load(void)
{

	return FAILURE;
}

void
juliet_unload(void)
{
}

BOOL
juliet_prepare(void)
{

	return FAILURE;
}


// ---- YM2151��
// �ꥻ�åȤ�Ʊ���ˡ�OPM���åפ�̵ͭ���ǧ
void
juliet_YM2151Reset(void)
{
}

int
juliet_YM2151IsEnable(void)
{

	return FALSE;
}

int
juliet_YM2151IsBusy(void)
{

	return FALSE;
}


void
juliet_YM2151W(BYTE reg, BYTE data)
{
}

// ---- YMF288��

void
juliet_YMF288Reset(void)
{
}

int
juliet_YM288IsEnable(void)
{

	return TRUE;
}

int
juliet_YM288IsBusy(void)
{

	return 0;
}

void
juliet_YMF288A(BYTE addr, BYTE data)
{
}

void
juliet_YMF288B(BYTE addr, BYTE data)
{
}

void
juliet_YMF288W(BYTE addr, BYTE data)
{
}

BYTE
juliet_YMF288R(BYTE addr)
{

	return 0xff;
}
