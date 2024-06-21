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

