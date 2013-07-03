// ---------------------------------------------------------------------------------------
//  IRQH.C - IRQ Handler (�Ͷ��ΥǥХ����ˤ�)
// ---------------------------------------------------------------------------------------

#include "common.h"
#include "m68000.h"
#include "irqh.h"

	BYTE	IRQH_IRQ[8];
	void	*IRQH_CallBack[8];

// -----------------------------------------------------------------------
//   �����
// -----------------------------------------------------------------------
void IRQH_Init(void)
{
	ZeroMemory(IRQH_IRQ, 8);
}


// -----------------------------------------------------------------------
//   �ǥե���ȤΥ٥������֤��ʤ��줬�����ä����Ѥ�����
// -----------------------------------------------------------------------
DWORD FASTCALL IRQH_DefaultVector(BYTE irq)
{
	IRQH_IRQCallBack(irq);
	return -1;
}


// -----------------------------------------------------------------------
//   ¾�γ����ߤΥ����å�
//   �ƥǥХ����Υ٥������֤��롼���󤫤�ƤФ�ޤ�
// -----------------------------------------------------------------------
void IRQH_IRQCallBack(BYTE irq)
{
	int i;
	IRQH_IRQ[irq] = 0;
	regs.IRQ_level = 0;
	for (i=7; i>0; i--)
	{
		if (IRQH_IRQ[i])
		{
			regs.irq_callback = IRQH_CallBack[i];
			regs.IRQ_level = i;
			if ( m68000_ICount ) {					// ¿�ų����߻���CARAT��
				m68000_ICountBk += m68000_ICount;		// ����Ū�˳����ߥ����å��򤵤���
				m68000_ICount = 0;				// �����κ� ^^;
			}
			break;
		}
	}
}


// -----------------------------------------------------------------------
//   ������ȯ��
// -----------------------------------------------------------------------
void IRQH_Int(BYTE irq, void* handler)
{
	int i;
	IRQH_IRQ[irq] = 1;
	if (handler==NULL)
		IRQH_CallBack[irq] = &IRQH_DefaultVector;
	else
		IRQH_CallBack[irq] = handler;
	for (i=7; i>0; i--)
	{
		if (IRQH_IRQ[i])
		{
			regs.irq_callback = IRQH_CallBack[i];
			regs.IRQ_level = i;
			if ( m68000_ICount ) {					// ¿�ų����߻���CARAT��
				m68000_ICountBk += m68000_ICount;		// ����Ū�˳����ߥ����å��򤵤���
				m68000_ICount = 0;				// �����κ� ^^;
			}
			return;
		}
	}
}
