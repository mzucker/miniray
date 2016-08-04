set(RAYTRACERS_CPP
  miniray_0.1
  miniray_0.2
  miniray_0.3
  miniray_0.4
  miniray_0.5
  miniray_0.6
  miniray_1.0
  miniray_1.1
  miniray_1.2
  miniray_1.3
  miniray_1.4
  miniray_1.4_commented
  miniray_2.0
  miniray_2.1
  miniray_2.2
  miniray_2.3
  miniray_2.4
  miniray_3.0
  )

set(RAYTRACERS_C
  miniray_3.2
  miniray_3.3
  miniray_3.4
  miniray_3.5
  miniray_3.6
  miniray_3.7
  miniray_3.8
  miniray_3.8_commented
  zucker
  )

set(RAYTRACERS
  ${RAYTRACERS_CPP}
  ${RAYTRACERS_C})

foreach(RAYTRACER_CPP ${RAYTRACERS_CPP})
  add_executable(${RAYTRACER_CPP} "${RAYTRACER_CPP}.cpp")
endforeach()

foreach(RAYTRACER_C ${RAYTRACERS_C})
  add_executable(${RAYTRACER_C} "${RAYTRACER_C}.c")
endforeach()

add_custom_target(ppm_images)

foreach(RAYTRACER ${RAYTRACERS})
  add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/${RAYTRACER}.ppm"
    COMMAND time ./${RAYTRACER} > "${RAYTRACER}.ppm"
    DEPENDS ${RAYTRACER}
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
  add_custom_target(${RAYTRACER}_image
    DEPENDS "${PROJECT_BINARY_DIR}/${RAYTRACER}.ppm")
  add_dependencies(ppm_images ${RAYTRACER}_image)
endforeach()

add_executable(encode "encode.cpp")