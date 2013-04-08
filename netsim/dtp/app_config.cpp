#include <string.h>
#include "../netsim/common.h"
#include "../netsim/Config.h"
#include "../netsim/Node.h"
#include "../netsim/FIFONode.h"
#include "../netsim/Packet.h"
#include "FDTPFlow.h"
#include "Host.h"
#include "Router.h"
void
Config::process_app_command(char* id)
{
    // Insert app-level commands here
    if (strcmp(id, "Router") == 0) {
        if (config_argnum != 2) {
            FATAL("Incorrect number of args for: %s", id);
        }
		//TRACE(TRL3, "Initialized router with address %d\n",(Address) config_args[0].numval);
        Router* cr= new Router((Address) config_args[0].numval,(int) config_args[1].numval);
		
    } else if (strcmp(id, "Host") == 0) {
        if (config_argnum != 1) {
            FATAL("Incorrect number of args for: %s", id);
        }
        Host* cr = new Host((Address) config_args[0].numval);
    }else if (strcmp(id, "FDTPFlow") == 0) {
        if (config_argnum != 4) {
            FATAL("Incorrect number of args for: %s", id);
        }
        FDTPFlow* cr = new FDTPFlow((Address) config_args[0].numval,
                                      (Address) config_args[1].numval,
                                      (Time) config_args[2].numval,
                                      (char *) config_args[3].numval);
    }
    config_argnum = 0;
    return;
}
