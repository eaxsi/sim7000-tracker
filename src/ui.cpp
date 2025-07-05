#include "ui.h"

Ui::Ui(platform * platform)
{
    m_platform = platform;
    m_state = state::off;
}

void Ui::set_state(state new_state)
{
    m_state = new_state;
}

void Ui::set_notification()
{
    m_notification_activated = true;
}

void Ui::Ui_task()
{
    while(true)
    {
        if(m_notification_activated)
        {
            m_platform->turn_off_led();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            m_platform->turn_on_led();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            m_platform->turn_off_led();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            m_platform->turn_on_led();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            m_platform->turn_off_led();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            m_platform->turn_on_led();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            m_platform->turn_off_led();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            m_platform->turn_on_led();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            m_platform->turn_off_led();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            
            m_notification_activated = false;
        }

        switch (m_state)
        {
        case state::off:
            m_platform->turn_off_led();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            break;
        
        case state::full_on:
            m_platform->turn_on_led();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            break;
        
        case state::half_blink:
            m_platform->turn_on_led();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            m_platform->turn_off_led();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            break;
        
        case state::single_blink:
            m_platform->turn_off_led();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            m_platform->turn_on_led();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            break;
        
        case state::two_blinks:
            m_platform->turn_off_led();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            m_platform->turn_on_led();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            m_platform->turn_off_led();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            m_platform->turn_on_led();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            break;
        
        default:
            m_platform->turn_off_led();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            break;
        }
    }
}
