# Edit following two lines to set component requirements (see docs)
idf_component_register(SRCS "main.c" "ota.c" "spiffs.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES
)