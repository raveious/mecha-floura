#include <uxr/client/client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool on_agent_found(const uxrAgentAddress* address, int64_t timestamp, void* args) {
    (void) timestamp; (void) args;

    printf("Found agent => ip: %s, port: %d\n", address->ip, address->port);

    return false;
}

bool discover_agent(uxrAgentAddress* chosen_agent) {
    #if CONFIG_UXR_DISCOVERY_TYPE_UNICAST
        /// TODO: Add support to list multiple IP addresses
        size_t agent_list_size = 1;
        uxrAgentAddress agent_list[agent_list_size];

        /// TODO: Might not want to rebuild this list every time
        strncpy(agent_list[agent_list_size].ip,
                CONFIG_UXR_DISCOVERY_IP,
                sizeof(agent_list[agent_list_size].ip));

        agent_list[agent_list_size].port = CONFIG_UXR_DISCOVERY_PORT;

        return uxr_discovery_agents_unicast(  CONFIG_UXR_DISCOVERY_ATTEMPTS,
                                              CONFIG_UXR_DISCOVERY_PERIOD,
                                              on_agent_found,
                                              NULL,
                                              chosen_agent,
                                              agent_list,
                                              agent_list_size);

    #else
        return uxr_discovery_agents_multicast(CONFIG_UXR_DISCOVERY_ATTEMPTS,
                                              CONFIG_UXR_DISCOVERY_PERIOD,
                                              on_agent_found,
                                              NULL,
                                              chosen_agent);
    #endif
}

void dds_task(void) {

    uxrAgentAddress chosen;

    while (true) {
        if (discover_agent(&chosen)) {
            // True -> The user returns true in the callback.
            printf("Chosen agent => ip: %s, port: %d\n", chosen.ip, chosen.port);
            break;
        } else {
            printf("Failed to discover an agent @ %lli.\n", esp_timer_get_time());
            fflush(stdout);

            vTaskDelay(1000 / portTICK_PERIOD_MS);

            //esp_restart();
        }
    }
    fflush(stdout);
}

