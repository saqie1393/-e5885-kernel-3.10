/*海思迁基线删除bsp_reg_def.h文件，导致其他模块无法引用
增加该文件内容为和原bsp_reg_def.h一致，设置GPIO复用关系需要引用改文件
*/




#ifndef __MBB_BSP_REG_DEF_H__
#define __MBB_BSP_REG_DEF_H__

/*lint -e760 -e547*/
#define INREG32(x)          readl(x)
#define OUTREG32(x, y)      writel((UINT32)(y), (x))
#define SETREG32(x, y)      OUTREG32((x), INREG32(x) | (y))
#define CLRREG32(x, y)      OUTREG32((x), INREG32(x) & ~(y))
#define SETBITVALUE32(addr, mask, value)  OUTREG32((addr), (INREG32(addr) & (~(mask))) | ((value) & (mask)))
/*lint +e760 +e547*/

#endif
