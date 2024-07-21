build_lib_example(
  NAME aodv
  SOURCE_FILES aodv.cc
  LIBRARIES_TO_LINK
    ${libwifi}
    ${libinternet}
    ${libaodv}
    ${libinternet-apps}
)

build_lib_example(
  NAME aodv_ex1
  SOURCE_FILES aodv_ex1.cc
  LIBRARIES_TO_LINK
    ${libwifi}
    ${libinternet}
    ${libaodv}
    ${libinternet-apps}
    ${libnetanim}
)

build_lib_example(
  NAME aodv_ex2
  SOURCE_FILES aodv_ex2.cc
  LIBRARIES_TO_LINK
    ${libwifi}
    ${libinternet}
    ${libaodv}
    ${libinternet-apps}
)

build_lib_example(
  NAME aodv_netanim
  SOURCE_FILES aodv_netanim.cc
  LIBRARIES_TO_LINK
    ${libwifi}
    ${libinternet}
    ${libaodv}
    ${libinternet-apps}
	${libmobility}
	${libapplications}
	${libflow-monitor}
	${libnetanim}
)

build_lib_example(
  NAME aodv_p2p
  SOURCE_FILES aodv_p2p.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libinternet}
    ${libnetwork}
    ${libpoint-to-point}
	${libcsma}
	${libmobility}
	${libnetanim}
	${libwifi}
    ${libinternet-apps}
    ${libflow-monitor}
    ${libapplications}
    ${libaodv}
    
)

build_lib_example(
  NAME aodv_random
  SOURCE_FILES aodv_random.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libpoint-to-point}
    ${libcsma}
    ${libwifi}
    ${libinternet}
    ${libapplications}
    ${libaodv}
    ${libinternet-apps}
    ${libflow-monitor}
    ${libnetanim}
)

build_lib_example(
  NAME cp_aodv_p2p
  SOURCE_FILES cp_aodv_p2p.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libinternet}
    ${libnetwork}
    ${libpoint-to-point}
    ${libcsma}
    ${libmobility}
    ${libnetanim}
    ${libwifi}
    ${libinternet-apps}
    ${libflow-monitor}
    ${libapplications}
    ${libaodv}

)

build_lib_example(
  NAME aodv_grid_final
  SOURCE_FILES aodv_grid_final.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libpoint-to-point}
    ${libcsma}
    ${libwifi}
    ${libinternet}
    ${libapplications}
    ${libaodv}
    ${libinternet-apps}
    ${libflow-monitor}
    ${libnetanim}
)

build_lib_example(
  NAME aodv_random_final
  SOURCE_FILES aodv_random_final.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libinternet}
    ${libnetwork}
    ${libpoint-to-point}
    ${libcsma}
    ${libmobility}
    ${libnetanim}
    ${libwifi}
    ${libinternet-apps}
    ${libflow-monitor}
    ${libapplications}
    ${libaodv}

)

