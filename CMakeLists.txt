cmake_minimum_required(VERSION 3.12)

project(AssaultCube)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(TARGET_OS "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(TARGET_OS "linux")
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(ENet REQUIRED)
find_package(Vorbis REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Ogg REQUIRED)
find_package(OpenAL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

set(CLIENT_SOURCES
	crypto.cpp
	audiomanager.cpp
	autodownload.cpp
	client.cpp
	clientgame.cpp
	clients2c.cpp
	command.cpp
	console.cpp
	docs.cpp
	editing.cpp
	entities.cpp
	http.cpp
	log.cpp
	main.cpp
	menus.cpp
	oggstream.cpp
	openal.cpp
	packetqueue.cpp
	physics.cpp
	protocol.cpp
	rendercubes.cpp
	rendergl.cpp
	renderhud.cpp
	rendermodel.cpp
	renderparticles.cpp
	rendertext.cpp
	rndmap.cpp
	scoreboard.cpp
	serverms.cpp
	server.cpp
	serverbrowser.cpp
	shadow.cpp
	soundlocation.cpp
	soundscheduler.cpp
	stream.cpp
	texture.cpp
	tools.cpp
	water.cpp
	weapon.cpp
	wizard.cpp
	world.cpp
	worldio.cpp
	worldlight.cpp
	worldocull.cpp
	worldrender.cpp
	zip.cpp
	bot/bot.cpp
	bot/botmanager.cpp
	bot/bot_ai.cpp
	bot/bot_util.cpp
	bot/bot_waypoint.cpp
	bot/ac_bot.cpp
	bot/ac_bot_ai.cpp
)

set(CLIENT_INCLUDES
	command.h
	console.h
	crypto_tools.h
	cube.h
	entity.h
	geom.h
	hudgun.h
	jpegenc.h
	md2.h
	md3.h
	model.h
	modelcache.h
	platform.h
	protocol.h
	protos.h
	scale.h
	server.h
	sound.h
	tools.h
	tristrip.h
	varray.h
	vertmodel.h
	weapon.h
	world.h
	ge25519_base.data
)

set(SERVER_SOURCES
	crypto.cpp
	log.cpp
	serverms.cpp
	server.cpp
	tools.cpp
	protocol.cpp
	wizard.cpp
	stream.cpp
	zip.cpp
)

set(SERVER_INCLUDES
	cube.h
	platform.h
	tools.h
	geom.h
	model.h
	protocol.h
	weapon.h
	world.h
	entity.h
	command.h
	console.h
	protos.h
	
	md2.h
	md3.h
	
	varray.h

	server.h
	serveractions.h
	serverchecks.h
	serverevents.h
	serverfiles.h
)

set(CLIENT_LIBRARY_INCLUDES
	#${SDL2_INCLUDE_DIR}
	#${SDL2_IMAGE_INCLUDE_DIR}
	${VORBIS_INCLUDE_DIRS}
	${JPEG_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
	${OGG_INCLUDE_DIRS}
	${OPENAL_INCLUDE_DIR}
	${X11_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${ENet_INCLUDE_DIRS}
)

set(SERVER_LIBRARY_INCLUDES
	${ENet_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
)

list(TRANSFORM CLIENT_SOURCES PREPEND "src/")
list(TRANSFORM CLIENT_INCLUDES PREPEND "src/")
list(TRANSFORM SERVER_SOURCES PREPEND "src/")
list(TRANSFORM SERVER_INCLUDES PREPEND "src/")

list(SORT CLIENT_SOURCES)
list(SORT CLIENT_INCLUDES)
list(SORT SERVER_SOURCES)
list(SORT SERVER_INCLUDES)

# Client

set(CLIENT AssaultCube-Client)

add_executable(${CLIENT} ${CLIENT_SOURCES} ${CLIENT_INCLUDES})

target_include_directories(${CLIENT} PRIVATE ${CLIENT_LIBRARY_INCLUDES})

target_link_libraries(${CLIENT} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} ${OGG_LIBRARIES} ${OPENAL_LIBRARY} ${OPENGL_LIBRARIES} ${X11_LIBRARIES} ${ENet_LIBRARIES} SDL2_image::SDL2_image ${VORBIS_LIBRARIES} SDL2::SDL2)

if(TARGET_OS STREQUAL "windows")
	target_link_libraries(${CLIENT} opengl32 winmm imm32 ws2_32 -mwindows)
	target_link_options(${CLIENT} PRIVATE -mwindows)
	set_target_properties(${CLIENT} PROPERTIES WIN32_EXECUTABLE FALSE)

	#target_compile_definitions(${CLIENT} PRIVATE NOMINMAX)
    #target_compile_definitions(${CLIENT} PRIVATE WIN32_LEAN_AND_MEAN)
    #target_compile_definitions(${CLIENT} PRIVATE NTDDI_VERSION=0x06000000)
    #target_compile_definitions(${CLIENT} PRIVATE _WIN32_WINNT=0x0600)
    #target_compile_definitions(${CLIENT} PRIVATE UNICODE)
    #target_compile_definitions(${CLIENT} PRIVATE _UNICODE)

    #target_link_libraries(${CLIENT} -static-libgcc)
    #target_link_libraries(${CLIENT} -static-libstdc++)
    
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-mwindows")
else()
	set(SERVER AssaultCube-Server)

	add_executable(${SERVER} ${SERVER_SOURCES} ${SERVER_INCLUDES})
	target_compile_definitions(${SERVER} PRIVATE STANDALONE=1)
	target_include_directories(${SERVER} PRIVATE ${SERVER_LIBRARY_INCLUDES})
	target_link_libraries(${SERVER} ${ENet_LIBRARIES} ${ZLIB_LIBRARIES})
endif()
