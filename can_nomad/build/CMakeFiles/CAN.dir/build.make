# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/robot/camilos_dev/can_nomad

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/robot/camilos_dev/can_nomad/build

# Include any dependencies generated for this target.
include CMakeFiles/CAN.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/CAN.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/CAN.dir/flags.make

CMakeFiles/CAN.dir/src/CANDevice.cpp.o: CMakeFiles/CAN.dir/flags.make
CMakeFiles/CAN.dir/src/CANDevice.cpp.o: ../src/CANDevice.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/robot/camilos_dev/can_nomad/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/CAN.dir/src/CANDevice.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CAN.dir/src/CANDevice.cpp.o -c /home/robot/camilos_dev/can_nomad/src/CANDevice.cpp

CMakeFiles/CAN.dir/src/CANDevice.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CAN.dir/src/CANDevice.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/robot/camilos_dev/can_nomad/src/CANDevice.cpp > CMakeFiles/CAN.dir/src/CANDevice.cpp.i

CMakeFiles/CAN.dir/src/CANDevice.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CAN.dir/src/CANDevice.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/robot/camilos_dev/can_nomad/src/CANDevice.cpp -o CMakeFiles/CAN.dir/src/CANDevice.cpp.s

CMakeFiles/CAN.dir/src/CANDevice.cpp.o.requires:

.PHONY : CMakeFiles/CAN.dir/src/CANDevice.cpp.o.requires

CMakeFiles/CAN.dir/src/CANDevice.cpp.o.provides: CMakeFiles/CAN.dir/src/CANDevice.cpp.o.requires
	$(MAKE) -f CMakeFiles/CAN.dir/build.make CMakeFiles/CAN.dir/src/CANDevice.cpp.o.provides.build
.PHONY : CMakeFiles/CAN.dir/src/CANDevice.cpp.o.provides

CMakeFiles/CAN.dir/src/CANDevice.cpp.o.provides.build: CMakeFiles/CAN.dir/src/CANDevice.cpp.o


CMakeFiles/CAN.dir/src/PCANDevice.cpp.o: CMakeFiles/CAN.dir/flags.make
CMakeFiles/CAN.dir/src/PCANDevice.cpp.o: ../src/PCANDevice.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/robot/camilos_dev/can_nomad/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/CAN.dir/src/PCANDevice.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CAN.dir/src/PCANDevice.cpp.o -c /home/robot/camilos_dev/can_nomad/src/PCANDevice.cpp

CMakeFiles/CAN.dir/src/PCANDevice.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CAN.dir/src/PCANDevice.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/robot/camilos_dev/can_nomad/src/PCANDevice.cpp > CMakeFiles/CAN.dir/src/PCANDevice.cpp.i

CMakeFiles/CAN.dir/src/PCANDevice.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CAN.dir/src/PCANDevice.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/robot/camilos_dev/can_nomad/src/PCANDevice.cpp -o CMakeFiles/CAN.dir/src/PCANDevice.cpp.s

CMakeFiles/CAN.dir/src/PCANDevice.cpp.o.requires:

.PHONY : CMakeFiles/CAN.dir/src/PCANDevice.cpp.o.requires

CMakeFiles/CAN.dir/src/PCANDevice.cpp.o.provides: CMakeFiles/CAN.dir/src/PCANDevice.cpp.o.requires
	$(MAKE) -f CMakeFiles/CAN.dir/build.make CMakeFiles/CAN.dir/src/PCANDevice.cpp.o.provides.build
.PHONY : CMakeFiles/CAN.dir/src/PCANDevice.cpp.o.provides

CMakeFiles/CAN.dir/src/PCANDevice.cpp.o.provides.build: CMakeFiles/CAN.dir/src/PCANDevice.cpp.o


CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o: CMakeFiles/CAN.dir/flags.make
CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o: ../src/SocketCANDevice.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/robot/camilos_dev/can_nomad/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o -c /home/robot/camilos_dev/can_nomad/src/SocketCANDevice.cpp

CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/robot/camilos_dev/can_nomad/src/SocketCANDevice.cpp > CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.i

CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/robot/camilos_dev/can_nomad/src/SocketCANDevice.cpp -o CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.s

CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o.requires:

.PHONY : CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o.requires

CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o.provides: CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o.requires
	$(MAKE) -f CMakeFiles/CAN.dir/build.make CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o.provides.build
.PHONY : CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o.provides

CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o.provides.build: CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o


CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o: CMakeFiles/CAN.dir/flags.make
CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o: ../src/RealTimeTask.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/robot/camilos_dev/can_nomad/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o -c /home/robot/camilos_dev/can_nomad/src/RealTimeTask.cpp

CMakeFiles/CAN.dir/src/RealTimeTask.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CAN.dir/src/RealTimeTask.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/robot/camilos_dev/can_nomad/src/RealTimeTask.cpp > CMakeFiles/CAN.dir/src/RealTimeTask.cpp.i

CMakeFiles/CAN.dir/src/RealTimeTask.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CAN.dir/src/RealTimeTask.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/robot/camilos_dev/can_nomad/src/RealTimeTask.cpp -o CMakeFiles/CAN.dir/src/RealTimeTask.cpp.s

CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o.requires:

.PHONY : CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o.requires

CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o.provides: CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o.requires
	$(MAKE) -f CMakeFiles/CAN.dir/build.make CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o.provides.build
.PHONY : CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o.provides

CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o.provides.build: CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o


# Object files for target CAN
CAN_OBJECTS = \
"CMakeFiles/CAN.dir/src/CANDevice.cpp.o" \
"CMakeFiles/CAN.dir/src/PCANDevice.cpp.o" \
"CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o" \
"CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o"

# External object files for target CAN
CAN_EXTERNAL_OBJECTS =

libCAN.a: CMakeFiles/CAN.dir/src/CANDevice.cpp.o
libCAN.a: CMakeFiles/CAN.dir/src/PCANDevice.cpp.o
libCAN.a: CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o
libCAN.a: CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o
libCAN.a: CMakeFiles/CAN.dir/build.make
libCAN.a: CMakeFiles/CAN.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/robot/camilos_dev/can_nomad/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX static library libCAN.a"
	$(CMAKE_COMMAND) -P CMakeFiles/CAN.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/CAN.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/CAN.dir/build: libCAN.a

.PHONY : CMakeFiles/CAN.dir/build

CMakeFiles/CAN.dir/requires: CMakeFiles/CAN.dir/src/CANDevice.cpp.o.requires
CMakeFiles/CAN.dir/requires: CMakeFiles/CAN.dir/src/PCANDevice.cpp.o.requires
CMakeFiles/CAN.dir/requires: CMakeFiles/CAN.dir/src/SocketCANDevice.cpp.o.requires
CMakeFiles/CAN.dir/requires: CMakeFiles/CAN.dir/src/RealTimeTask.cpp.o.requires

.PHONY : CMakeFiles/CAN.dir/requires

CMakeFiles/CAN.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/CAN.dir/cmake_clean.cmake
.PHONY : CMakeFiles/CAN.dir/clean

CMakeFiles/CAN.dir/depend:
	cd /home/robot/camilos_dev/can_nomad/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/robot/camilos_dev/can_nomad /home/robot/camilos_dev/can_nomad /home/robot/camilos_dev/can_nomad/build /home/robot/camilos_dev/can_nomad/build /home/robot/camilos_dev/can_nomad/build/CMakeFiles/CAN.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/CAN.dir/depend
