idf_component_register(SRCS "day4_state_mcahine.c"
                    INCLUDE_DIRS "."
                    REQUIRES state_machine storage event_bus diagnostics
                    )
