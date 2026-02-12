idf_component_register(SRCS "wifi_app.c" "main.c" "button_app.c" "mqtt_app.c"
                    INCLUDE_DIRS ".")

target_add_binary_data(${COMPONENT_TARGET} "mqtt_eclipseprojects_io.pem" TEXT)
