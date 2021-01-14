DESCRIPTION = "Heartbeat Linux kernel module"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

COMPATIBLE_MACHINE = "qemuarm"

SRC_URI = "file://heartbeat.c"

S = "${WORKDIR}"

do_compile() {
	set CFLAGS -g
	${CC} ${CFLAGS} -lm -pthread heartbeat.c ${LDFLAGS} -o heartbeat
	unset CFLAGS
}

do_install(){
	install -d ${D}${bindir}
	install -m 0755 heartbeat ${D}${bindir}
}
