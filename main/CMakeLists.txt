# Edit following two lines to set component requirements (see docs)
idf_component_register(SRCS "main.c" "wifi.c" "spiffs.c" "server.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES
)