#pragma once

#include "common.h"

class Settings
{
    public:
        Settings();

        //bool node_id_available(); const
        //char * get_node_id(); const
        system_mode get_mode();
        uint32_t get_periodic_tracking_interval();

        //void set_node_id(char *node_id);
        
        void set_mode(system_mode mode);
        void set_periodic_tracking_interval(uint32_t interval);

    private:
        //char m_nodeId[8] = "";
        static inline system_mode m_mode = system_mode::sleep;
        uint32_t m_periodic_tracking_interval = 0;
};
