idf_component_register(
    SRCS "states.c" "state_machine.c"
    INCLUDE_DIRS "include"
    REQUIRES storage common wifi_manager
)