project(smart_home)

set(SRCS main.c 
		 ble_gatt/ble_gatt.c
		 color_tb/color_tb.c
		 gesture_sensor/gesture_sensor.c
		 gesture_sensor/gpio.c)

add_executable(${PROJECT_NAME} ${SRCS})
