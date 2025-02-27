idf_component_register(SRCS "uart-console.c"
                    INCLUDE_DIRS ./include
                    REQUIRES base_console driver
                    )
