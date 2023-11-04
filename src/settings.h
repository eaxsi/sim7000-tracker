#pragma once

#include "common.h"

class Settings
{
    public:
        Settings();

        //bool node_id_available(); const
        //char * get_node_id(); const
        system_mode get_mode();

        //void set_node_id(char *node_id);
        void set_mode(system_mode mode);

    private:
        //char m_nodeId[8] = "";
        static inline system_mode m_mode = system_mode::sleep;
};
