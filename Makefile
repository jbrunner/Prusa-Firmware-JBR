build_nr:=$(shell expr `sed -n 's/^.*CUSTOM_MENDEL_NAME "FW [0-9.]*-\([0-9]*\).*/\1/p' Firmware/Configuration_prusa.h` + 1)
build_ver:=$(shell sed -n 's/^.*CUSTOM_MENDEL_NAME "FW \([0-9.]*\)-.*/\1/p' Firmware/Configuration_prusa.h)
all: clean build

clean:
	git clean -Xf

build:
	sed -i '' 's/CUSTOM_MENDEL_NAME "[^"]*"/CUSTOM_MENDEL_NAME "FW ${build_ver}-${build_nr}"/' Firmware/Configuration_prusa.h
	docker run --rm -v `pwd`:/app jb5r/prusa-firmware-builder en
	cp lang/firmware.hex builds/firmware_${build_ver}-${build_nr}.hex
