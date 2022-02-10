# Copyright 1999-2002 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License v2
# $Id: msicp-0.8.10.ebuild,v 1.1.1.1 2006/08/31 22:40:54 hari Exp $

DESCRIPTION="SIP Express Router"

HOMEPAGE="http://www.iptel.org/msicp"
SRC_URI="ftp://ftp.berlios.de/pub/msicp/0.8.10/src/${P}_src.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86"

DEPEND=">=sys-devel/gcc-2.95.3
		>=sys-devel/bison-1.35
		>=sys-devel/flex-2.5.4a
		mysql? ( >=dev-db/mysql-3.23.52 )
		dev-libs/expat"

S="${WORKDIR}/${P}"

src_compile() {
	if [ ! "`use ipv6`" ]; then
		cp Makefile.defs Makefile.defs.orig 
		sed -e "s/-DUSE_IPV6//g" Makefile.defs.orig > Makefile.defs;
	fi
	local exclude="CVS radius_acc radius_auth snmp"
	use mysql || exclude="${exclude} mysql"
	make all CFLAGS="${CFLAGS}" \
		prefix=${D}/ \
		exclude_modules="${exclude}" \
		cfg-prefix=/ \
		cfg-target=/etc/msicp/ || die
}

src_install () {
	local exclude="CVS radius_acc radius_auth snmp"
	use mysql || exclude="${exclude} mysql"
	make install \
		prefix=${D}/ \
		exclude_modules="${exclude}" \
		bin-prefix=${D}/usr/sbin \
		bin-dir="" \
		cfg-prefix=${D}/etc \
		cfg-dir=msicp/ \
		cfg-target=/etc/msicp \
		modules-prefix=${D}/usr/lib/msicp \
		modules-dir=modules \
		modules-target=/usr/lib/msicp/modules/ \
		man-prefix=${D}/usr/share/man \
		man-dir="" \
		doc-prefix=${D}/usr/share/doc \
		doc-dir=${P} || die
	exeinto /etc/init.d
	newexe gentoo/msicp.init msicp
	# fix what the Makefile don't do
	exeinto /usr/sbin
	newexe scripts/harv_ser.sh harv_ser.sh
	rm ${D}/usr/sbin/gen_ha1
	if [ "`use mysql`" ]; then
		exeinto /usr/bin
		newexe utils/gen_ha1/gen_ha1 gen_ha1
	else
		rm ${D}/usr/sbin/ser_mysql.sh
	fi
}

pkg_prerm () {
	/etc/init.d/msicp stop >/dev/null
}
