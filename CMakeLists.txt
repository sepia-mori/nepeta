CMAKE_MINIMUM_REQUIRED( VERSION 3.12 )
project(nepeta VERSION 0.9.0)

# Setup environment
include(GNUInstallDirs)
find_package(Boost COMPONENTS unit_test_framework)


# Setup configurable parameters
if(Boost_FOUND)
	set(COMPILE_TESTS true CACHE BOOL "Compile tests")
endif()
set(COMPILE_EXAMPLES true CACHE BOOL "Compile examples")
set(INSTALL_SELF_CONTAINED false CACHE BOOL "Install the self-contained header instead of multiple headers")


# Setup library target
add_library(nepeta INTERFACE)
target_compile_features(nepeta INTERFACE cxx_std_17)


# Setup installation
if(INSTALL_SELF_CONTAINED)
	target_include_directories(nepeta INTERFACE
		$<BUILD_INTERFACE:${nepeta_SOURCE_DIR}/self-contained>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	)
	install(
		DIRECTORY ${PROJECT_SOURCE_DIR}/self-contained/
		DESTINATION include/nepeta
	)
else()
	target_include_directories(nepeta INTERFACE
		$<BUILD_INTERFACE:${nepeta_SOURCE_DIR}/src>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	)
	install(
		DIRECTORY ${PROJECT_SOURCE_DIR}/src/
		DESTINATION include/nepeta
	)
endif()

install(
	TARGETS nepeta
	EXPORT nepeta_Targets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"NepetaConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
	"${PROJECT_SOURCE_DIR}/NepetaConfig.cmake.in"
	"${PROJECT_BINARY_DIR}/NepetaConfig.cmake"
	INSTALL_DESTINATION
	${CMAKE_INSTALL_DATAROOTDIR}/nepeta/cmake
)

install(
	EXPORT nepeta_Targets
	FILE nepetaTargets.cmake
	NAMESPACE nepeta::
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/nepeta/cmake
)

install(
	FILES
		"${PROJECT_BINARY_DIR}/NepetaConfig.cmake"
		"${PROJECT_BINARY_DIR}/NepetaConfigVersion.cmake"
	DESTINATION
		${CMAKE_INSTALL_DATAROOTDIR}/nepeta/cmake
)


# Setup testing and examples
if(COMPILE_TESTS)
	enable_testing()
	add_executable(nepeta_test
		test/test_main.cpp
		test/nepeta_reader.cpp
		test/nepeta_writer.cpp
		test/nepeta_algorithm.cpp
	)
	add_test(NAME nepeta_test COMMAND nepeta_test)
	target_link_libraries(nepeta_test
		nepeta Boost::unit_test_framework
	)
	
	add_executable(simple_performance
		test/simple_performance.cpp
	)
	target_link_libraries(simple_performance nepeta)
endif()
if(COMPILE_EXAMPLES)
	add_executable(basic_usage example/basic_usage.cpp)
	target_link_libraries(basic_usage nepeta)
	
	add_executable(read_file example/read_file.cpp)
	target_link_libraries(read_file nepeta)
	
	add_executable(hello_world example/hello_world.cpp)
	target_link_libraries(hello_world nepeta)
endif()


# Add documentation target
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMENT "Generating API documentation" VERBATIM)
endif(DOXYGEN_FOUND)
