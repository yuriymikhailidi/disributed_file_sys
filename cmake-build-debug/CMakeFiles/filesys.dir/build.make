# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/yuriymikhaildi/Desktop/filesys

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/yuriymikhaildi/Desktop/filesys/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/filesys.dir/depend.make
# Include the progress variables for this target.
include CMakeFiles/filesys.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/filesys.dir/flags.make

CMakeFiles/filesys.dir/dfclient/dfclient.c.o: CMakeFiles/filesys.dir/flags.make
CMakeFiles/filesys.dir/dfclient/dfclient.c.o: ../dfclient/dfclient.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/yuriymikhaildi/Desktop/filesys/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/filesys.dir/dfclient/dfclient.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/filesys.dir/dfclient/dfclient.c.o -c /Users/yuriymikhaildi/Desktop/filesys/dfclient/dfclient.c

CMakeFiles/filesys.dir/dfclient/dfclient.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/filesys.dir/dfclient/dfclient.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/yuriymikhaildi/Desktop/filesys/dfclient/dfclient.c > CMakeFiles/filesys.dir/dfclient/dfclient.c.i

CMakeFiles/filesys.dir/dfclient/dfclient.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/filesys.dir/dfclient/dfclient.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/yuriymikhaildi/Desktop/filesys/dfclient/dfclient.c -o CMakeFiles/filesys.dir/dfclient/dfclient.c.s

CMakeFiles/filesys.dir/dfserver/dfserver.c.o: CMakeFiles/filesys.dir/flags.make
CMakeFiles/filesys.dir/dfserver/dfserver.c.o: ../dfserver/dfserver.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/yuriymikhaildi/Desktop/filesys/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/filesys.dir/dfserver/dfserver.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/filesys.dir/dfserver/dfserver.c.o -c /Users/yuriymikhaildi/Desktop/filesys/dfserver/dfserver.c

CMakeFiles/filesys.dir/dfserver/dfserver.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/filesys.dir/dfserver/dfserver.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/yuriymikhaildi/Desktop/filesys/dfserver/dfserver.c > CMakeFiles/filesys.dir/dfserver/dfserver.c.i

CMakeFiles/filesys.dir/dfserver/dfserver.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/filesys.dir/dfserver/dfserver.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/yuriymikhaildi/Desktop/filesys/dfserver/dfserver.c -o CMakeFiles/filesys.dir/dfserver/dfserver.c.s

# Object files for target filesys
filesys_OBJECTS = \
"CMakeFiles/filesys.dir/dfclient/dfclient.c.o" \
"CMakeFiles/filesys.dir/dfserver/dfserver.c.o"

# External object files for target filesys
filesys_EXTERNAL_OBJECTS =

filesys: CMakeFiles/filesys.dir/dfclient/dfclient.c.o
filesys: CMakeFiles/filesys.dir/dfserver/dfserver.c.o
filesys: CMakeFiles/filesys.dir/build.make
filesys: CMakeFiles/filesys.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/yuriymikhaildi/Desktop/filesys/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable filesys"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/filesys.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/filesys.dir/build: filesys
.PHONY : CMakeFiles/filesys.dir/build

CMakeFiles/filesys.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/filesys.dir/cmake_clean.cmake
.PHONY : CMakeFiles/filesys.dir/clean

CMakeFiles/filesys.dir/depend:
	cd /Users/yuriymikhaildi/Desktop/filesys/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/yuriymikhaildi/Desktop/filesys /Users/yuriymikhaildi/Desktop/filesys /Users/yuriymikhaildi/Desktop/filesys/cmake-build-debug /Users/yuriymikhaildi/Desktop/filesys/cmake-build-debug /Users/yuriymikhaildi/Desktop/filesys/cmake-build-debug/CMakeFiles/filesys.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/filesys.dir/depend

