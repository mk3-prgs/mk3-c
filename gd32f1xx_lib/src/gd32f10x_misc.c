#include "gd32f10x_misc.h"

//void nvic_irq_enable(NVIC_InitTypeDef* NVIC_InitStruct)
void stm_nvic_irq_enable(uint8_t nvic_irq, uint8_t nvic_irq_pre_priority, uint8_t nvic_irq_sub_priority)
{
uint32_t tmppriority = 0x00, tmppre = 0x00, tmpsub = 0x0F;
        /* Compute the Corresponding IRQ Priority --------------------------------*/
        tmppriority = (0x700 - ((SCB->AIRCR) & (uint32_t)0x700))>> 0x08;
        tmppre = (0x4 - tmppriority);
        tmpsub = tmpsub >> tmppriority;

        tmppriority = (uint32_t)nvic_irq_pre_priority << tmppre;
        tmppriority |=  (nvic_irq_sub_priority & tmpsub);
        tmppriority = tmppriority << 0x04;

        NVIC->IP[nvic_irq] = tmppriority;

        /* Enable the Selected IRQ Channels --------------------------------------*/
        NVIC->ISER[nvic_irq >> 0x05] = (uint32_t)0x01 << (nvic_irq & (uint8_t)0x1F);
}

void nvic_irq_disable(uint8_t nvic_irq)
{
        /* Disable the Selected IRQ Channels -------------------------------------*/
        NVIC->ICER[nvic_irq >> 0x05] = (uint32_t)0x01 << (nvic_irq & (uint8_t)0x1F);
}

void nvic_irq_enable(uint8_t nvic_irq,
                     uint8_t nvic_irq_pre_priority,
                     uint8_t nvic_irq_sub_priority)
{
    uint32_t temp_priority = 0x00U, temp_pre = 0x00U, temp_sub = 0x00U;

    /* use the priority group value to get the temp_pre and the temp_sub */
    switch ((SCB->AIRCR) & (uint32_t)0x700U) {
    case NVIC_PRIGROUP_PRE0_SUB4:
        temp_pre = 0U;
        temp_sub = 0x4U;
        break;
    case NVIC_PRIGROUP_PRE1_SUB3:
        temp_pre = 1U;
        temp_sub = 0x3U;
        break;
    case NVIC_PRIGROUP_PRE2_SUB2:
        temp_pre = 2U;
        temp_sub = 0x2U;
        break;
    case NVIC_PRIGROUP_PRE3_SUB1:
        temp_pre = 3U;
        temp_sub = 0x1U;
        break;
    case NVIC_PRIGROUP_PRE4_SUB0:
        temp_pre = 4U;
        temp_sub = 0x0U;
        break;
    default:
        nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
        temp_pre = 2U;
        temp_sub = 0x2U;
        break;
    }

    /* get the temp_priority to fill the NVIC->IP register */
    temp_priority = (uint32_t)nvic_irq_pre_priority << (0x4U - temp_pre);
    temp_priority |= (nvic_irq_sub_priority &(0x0FU >> (0x4U - temp_sub)));
    temp_priority = temp_priority << 0x04U;
    NVIC->IP[nvic_irq] = (uint8_t)temp_priority;

    /* enable the selected IRQ */
    NVIC->ISER[nvic_irq >> 0x05U] = (uint32_t)0x01U << (nvic_irq & (uint8_t)0x1FU);
}

/*!
    \brief      set the priority group
    \param[in]  nvic_prigroup: the NVIC priority group
      \arg        NVIC_PRIGROUP_PRE0_SUB4:0 bits for pre-emption priority 4 bits for subpriority
      \arg        NVIC_PRIGROUP_PRE1_SUB3:1 bits for pre-emption priority 3 bits for subpriority
      \arg        NVIC_PRIGROUP_PRE2_SUB2:2 bits for pre-emption priority 2 bits for subpriority
      \arg        NVIC_PRIGROUP_PRE3_SUB1:3 bits for pre-emption priority 1 bits for subpriority
      \arg        NVIC_PRIGROUP_PRE4_SUB0:4 bits for pre-emption priority 0 bits for subpriority
    \param[out] none
    \retval     none
*/
void nvic_priority_group_set(uint32_t nvic_prigroup)
{
    /* set the priority group value */
    SCB->AIRCR = NVIC_AIRCR_VECTKEY_MASK | nvic_prigroup;
}

/*!
    \brief      set the NVIC vector table base address
    \param[in]  nvic_vict_tab: the RAM or FLASH base address
      \arg        NVIC_VECTTAB_RAM: RAM base address
      \are        NVIC_VECTTAB_FLASH: Flash base address
    \param[in]  offset: Vector Table offset
    \param[out] none
    \retval     none
*/
void nvic_vector_table_set(uint32_t nvic_vict_tab, uint32_t offset)
{
    SCB->VTOR = nvic_vict_tab | (offset & NVIC_VECTTAB_OFFSET_MASK);
}

/*!
    \brief      set the state of the low power mode
    \param[in]  lowpower_mode: the low power mode state
      \arg        SCB_LPM_SLEEP_EXIT_ISR: if chose this para, the system always enter low power
                    mode by exiting from ISR
      \arg        SCB_LPM_DEEPSLEEP: if chose this para, the system will enter the DEEPSLEEP mode
      \arg        SCB_LPM_WAKE_BY_ALL_INT: if chose this para, the lowpower mode can be woke up
                    by all the enable and disable interrupts
    \param[out] none
    \retval     none
*/
void system_lowpower_set(uint8_t lowpower_mode)
{
    SCB->SCR |= (uint32_t)lowpower_mode;
}

/*!
    \brief      reset the state of the low power mode
    \param[in]  lowpower_mode: the low power mode state
      \arg        SCB_LPM_SLEEP_EXIT_ISR: if chose this para, the system will exit low power
                    mode by exiting from ISR
      \arg        SCB_LPM_DEEPSLEEP: if chose this para, the system will enter the SLEEP mode
      \arg        SCB_LPM_WAKE_BY_ALL_INT: if chose this para, the lowpower mode only can be
                    woke up by the enable interrupts
    \param[out] none
    \retval     none
*/
void system_lowpower_reset(uint8_t lowpower_mode)
{
    SCB->SCR &= (~(uint32_t)lowpower_mode);
}

/*!
    \brief      set the systick clock source
    \param[in]  systick_clksource: the systick clock source needed to choose
      \arg        SYSTICK_CLKSOURCE_HCLK: systick clock source is from HCLK
      \arg        SYSTICK_CLKSOURCE_HCLK_DIV8: systick clock source is from HCLK/8
    \param[out] none
    \retval     none
*/

void systick_clksource_set(uint32_t systick_clksource)
{
    if(SYSTICK_CLKSOURCE_HCLK == systick_clksource ){
        /* set the systick clock source from HCLK */
        SysTick->CTRL |= SYSTICK_CLKSOURCE_HCLK;
    }else{
        /* set the systick clock source from HCLK/8 */
        SysTick->CTRL &= SYSTICK_CLKSOURCE_HCLK_DIV8;
    }
}
