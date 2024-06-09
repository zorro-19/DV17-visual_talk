#ifndef CPU_ADC_H
#define CPU_ADC_H

#include "typedef.h"

/*
 * ADC通道选择
 */
enum {
    AD_CH00_PA02,
    AD_CH01_PB00,
    AD_CH02_PB03,
    AD_CH03_PB12,
    AD_CH04_PB14,
    AD_CH05_PE03,
    AD_CH06_PG00,
    AD_CH07_PG07,
    AD_CH08_PE05,
    AD_CH09_PH12,
    AD_CH10_PH14,
    AD_CH11_PA05,
    AD_CH12_PH10,
    AD_CH13_RTC_2_ADC,
    AD_CH14_ADC_ANA_DET,
    AD_CH15_LDO_VBG,
};

#define ADC_WTIME(x)    GPADC_CON = (GPADC_CON & ~(0xf<<12)) | (x<<12)
#define ADCSEL(x)       GPADC_CON = (GPADC_CON & ~(0xf<<8))  | (x<<8)
#define ADC_PND()       (GPADC_CON & BIT(7))
#define KITSTART()      GPADC_CON |= BIT(6)
#define ADC_IE(x)       GPADC_CON = (GPADC_CON & ~BIT(5)) | (x<<5)
#define ADC_EN(x)       GPADC_CON = (GPADC_CON & ~BIT(4)) | (x<<4)
#define ADCEN(x)        GPADC_CON = (GPADC_CON & ~BIT(3)) | (x<<3)
#define ADC_BAUD(x)     GPADC_CON = (GPADC_CON & ~(BIT(2)| BIT(1)| BIT(0))) | (x)

struct adc_scan {
    u8 channel;
    u16 value;
    void (*updata)();
};

extern struct adc_scan adc_scan_begin[];
extern struct adc_scan adc_scan_end[];

#define REGISTER_ADC_SCAN(scan) \
	static struct adc_scan scan sec(.adc_scan) = {


u16 adc_scan_manual(u8 ch);

#endif

