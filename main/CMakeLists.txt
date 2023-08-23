# Edit following two lines to set component requirements (see docs)
idf_component_register(SRCS "main.c" "wifi.c" "spiffs.c" "server.c"
                    INCLUDE_DIRS "." "${CMAKE_CURRENT_SOURCE_DIR}/../include/"
                    EMBED_FILES
)