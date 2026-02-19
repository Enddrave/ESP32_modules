idf_component_register(
    SRCS "state_machine.c" "states.c"
    INCLUDE_DIRS "include"
    REQUIRES storage common
)