# Use with MSYS prompt
export ARM_GCC_TOOLCHAIN=/c/Program\ Files\ \(x86\)/Arduino/hardware/tools/g++_arm_none_eabi/bin

# test variant build
cd /c/Program\ Files\ \(x86\)/Arduino/hardware/arduino/sam/variants/arduino_due_opentracker/build_gcc/
make

# build library (output to "../../core/arduino" folder)
cd /c/Program\ Files\ \(x86\)/Arduino/hardware/arduino/sam/system/libsam/build_gcc/
make

cd /c/Program\ Files\ \(x86\)/
find Arduino/hardware/arduino/sam/ -type f -mtime -20 | zip -@ -9 -v ${USERPROFILE}/Desktop/OpenTracker_Arduino.zip
