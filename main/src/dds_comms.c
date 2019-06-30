#include <uxr/client/client.h>
#include <ucdr/microcdr.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Handle to save for the dds task
TaskHandle_t task_handle = NULL;

bool on_agent_found(const uxrAgentAddress* address, int64_t timestamp, void* args) {
    (void) timestamp; (void) args;

    printf("Found agent => ip: %s, port: %d\n", address->ip, address->port);

    return false;
}

/*
bool discover_agent(uxrAgentAddress* agent, void* found_agent_func) {
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
                                              found_agent_func,
                                              NULL,
                                              agent,
                                              agent_list,
                                              agent_list_size);

    #else
        return uxr_discovery_agents_multicast(CONFIG_UXR_DISCOVERY_ATTEMPTS,
                                              CONFIG_UXR_DISCOVERY_PERIOD,
                                              found_agent_func,
                                              NULL,
                                              agent);
    #endif
}
*/

void dds_task(void* parms) {
    (void) parms;

    // Selected agent to use
    /*
    uxrAgentAddress chosen_agent;

    // Discover the agent that will be used for later
    while (true) {
        if (discover_agent(&chosen_agent, &on_agent_found)) {
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
    */

    #ifdef CONFIG_PROFILE_UDP_TRANSPORT
    uxrUDPTransport transport;
    uxrUDPPlatform platform;

    if (!uxr_init_udp_transport(&transport, &platform, "192.168.10.150", 20000)) {
        printf("Failed to create transport.");
        // return;
    }

    #else
    #error No transport was selected.
    #endif

    uxrSession session;
    uxr_init_session(&session, &transport.comm, 0x00112233);
    if (!uxr_create_session(&session)) {
        printf("Failed to create session with agent.\n");
        printf("ID: %hhu\n", session.info.id);
        printf("Key: 0x%02hhX 0x%02hhX 0x%02hhX 0x%02hhX\n", session.info.key[0], session.info.key[1], session.info.key[2], session.info.key[3]);
        printf("Requested Status: %hu\n", session.info.last_requested_status);
        printf("Request ID: %hhu\n", session.info.last_request_id);
        // return;
    }

    // printf("Node online!");
    fflush(stdout);
    while(true) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void setup_dds_task(void) {
    BaseType_t task_status;

    // Create the task to run DDS out of.
    task_status = xTaskCreatePinnedToCore(dds_task,
                                          "DDS_TASK",
                                          4096, //STACK_SIZE,
                                          NULL,
                                          configMAX_PRIORITIES - 1,
                                         &task_handle,
                                          portNUM_PROCESSORS - 1
                                         );

    // Complain if the task creation didn't work.
}

