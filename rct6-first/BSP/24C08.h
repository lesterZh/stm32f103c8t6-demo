

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_EVAL_I2C_EE_H
#define __STM32_EVAL_I2C_EE_H
#include "sys.h"  

#ifdef __cplusplus
 extern "C" {
#endif

extern void IIC_Init(void);
extern void AT24C08_WriteOneByte(u16 addr,u8 data);
extern u8 AT24C08_ReadOneByte(u16 addr);
extern void AT24C08_WriteByte(u16 addr,u16 len,u8 *buffer);
extern void AT24C08_ReadByte(u16 addr,u16 len,u8 *buffer);


#ifdef __cplusplus
}
#endif

#endif /* __STM32_EVAL_I2C_EE_H */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

