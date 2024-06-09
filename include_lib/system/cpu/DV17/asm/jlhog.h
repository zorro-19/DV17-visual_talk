
#ifndef __JLHOG_H__
#define __JLHOG_H__


#include "typedef.h"


#define		JLHOG_EN	(0)
#define 	JLHOG_INT_EN 	(1)
#define 	JLHOG_START 	(2)
#define 	JLHOG_AXI_IDLE	(3)
#define 	JLHOG_STATUS 	(4)
#define 	JLHOG_INT_PND	(10)
#define 	JLHOG_INT_CLR	(11)


#define 	JLHOG_EN_MASK		BIT(0)
#define 	JLHOG_INT_EN_MASK	BIT(1)
#define 	JLHOG_START_MASK	BIT(2)
#define 	JLHOG_AXI_IDLE_MASK	BIT(3)
#define 	JLHOG_STATUS_MASK	BIT(4)
#define 	JLHOG_INT_PND_MASK	BIT(10)
#define		JLHOG_INT_CLR_MASK	BIT(11)





#endif // __JLHOG_H__
