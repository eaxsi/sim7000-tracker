/*
# include "communication.h"

Communication::Communication()
{
    pinMode(PWR_PIN, OUTPUT);
}


bool Communication::init()
{
    // power SIM7000
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
    SerialAT.flush();

    //power_modem();
    digitalWrite(PWR_PIN, HIGH);
    delay(300);
    digitalWrite(PWR_PIN, LOW);
    delay(300);

    m_modem.restart();
    delay(1000);
    while(!m_modem.testAT())
    {
        INFO("...");
        delay(1000);
    }
    m_modem.init();

    String imei = m_modem.getIMEI();
    imei.substring(8, 15).toCharArray(m_nodeId, 8);
    INFO("Node ID: %s", m_nodeId);

    uint8_t sim_status = m_modem.getSimStatus();
    if(sim_status == SIM_LOCKED)
    {
        ERROR("SIM-card locked!");
        return false;
    }
    else if(sim_status == SIM_ERROR)
    {
        ERROR("SIM-card missing!");
        return false;
    }

    return true;
}

void Communication::power_modem()
{
    digitalWrite(PWR_PIN, HIGH);
    delay(300);
    digitalWrite(PWR_PIN, LOW);
}

void Communication::power_off_modem()
{

}
*/