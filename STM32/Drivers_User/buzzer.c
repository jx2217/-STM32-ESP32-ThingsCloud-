#include "buzzer.h"
#include "main.h"

static uint8_t s_buzzer_state = 0U;

/*
 * 如果你的蜂鸣器是高电平有效：
 *   BUZZER_ACTIVE_LEVEL = GPIO_PIN_SET
 * 如果你的蜂鸣器是低电平有效：
 *   BUZZER_ACTIVE_LEVEL = GPIO_PIN_RESET
 */
#define BUZZER_ACTIVE_LEVEL     GPIO_PIN_SET
#define BUZZER_INACTIVE_LEVEL   GPIO_PIN_RESET

void buzzer_init(void)
{
    s_buzzer_state = 0U;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, BUZZER_INACTIVE_LEVEL);
	
		buzzer_on();
		HAL_Delay(200);
		buzzer_off();
}

void buzzer_set(uint8_t on)
{
    if (on)
    {
        s_buzzer_state = 1U;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, BUZZER_ACTIVE_LEVEL);
    }
    else
    {
        s_buzzer_state = 0U;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, BUZZER_INACTIVE_LEVEL);
    }
}

void buzzer_on(void)
{
    buzzer_set(1U);
}

void buzzer_off(void)
{
    buzzer_set(0U);
}

void buzzer_toggle(void)
{
    if (s_buzzer_state)
    {
        buzzer_off();
    }
    else
    {
        buzzer_on();
    }
}

uint8_t buzzer_get_state(void)
{
    return s_buzzer_state;
}
